#include <arduinoFFT.h>
#include <driver/i2s.h>

#include <ArduinoJson.h>

/***************************************************************************/
// oled screen
#include "SSD1306Wire.h"

/***************************************************************************/
// Industrial Data Logger networking implementation.
#include <IDLNetworking.h>

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

const i2s_port_t I2S_PORT = I2S_NUM_0;
const int BLOCK_SIZE = 1024;

const int samplingFrequency = 44100;
const int maxMicFreq = 15000;
const int nBins = 32;

const float binSize = samplingFrequency / BLOCK_SIZE;
const int usefulBins = maxMicFreq / binSize;
const int binsToBin = ceil(usefulBins / nBins);
double bins[nBins];

double vReal[BLOCK_SIZE];
double vImag[BLOCK_SIZE];
int32_t samples[BLOCK_SIZE];

arduinoFFT FFT = arduinoFFT(vReal, vImag, BLOCK_SIZE,
                            samplingFrequency); /* Create FFT object */

IDLNetworking idl = IDLNetworking("espFFT");

/*=========================================================================*/
// OLED screen instance
SSD1306Wire display(0x3c, 5, 4);

/**************************************************************************/
// display loop
void displayLoop() {
    display.clear();
    if (WiFi.status() != WL_CONNECTED) {
        display.setFont(ArialMT_Plain_24);
        display.drawString(0, 20, "Connecting...");
    } else {
        display.setFont(ArialMT_Plain_24);
        display.drawString(0, 0, idl.getDeviceId());
        display.setFont(ArialMT_Plain_10);
        display.drawString(0, 25, WiFi.localIP().toString());
    }

    display.display();
}

void setupMic() {
    Serial.println("Configuring I2S...");
    esp_err_t err;

    // The I2S config as per the example

    const i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // Receive, not transfer
        .sample_rate = samplingFrequency,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_24BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Interrupt level 1
        .dma_buf_count = 4,                       // number of buffers
        .dma_buf_len = BLOCK_SIZE,                // samples per buffer
    };

    // The pin config as per the setup
    i2s_pin_config_t pin_config = {
        .bck_io_num = 14,   // IIS_SCLK
        .ws_io_num = 15,    // IIS_LCLK
        .data_out_num = -1, // IIS_DSIN
        .data_in_num = 32   // IIS_DOUT
    };

    // Configuring the I2S driver and pins.
    // This function must be called before any I2S driver read/write operations.
    err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("Failed installing driver: %d\n", err);
        while (true)
            ;
    }
    err = i2s_set_pin(I2S_PORT, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("Failed setting pin: %d\n", err);
        while (true)
            ;
    }
    Serial.println("I2S driver installed.");
}

void setup() {
    Serial.begin(921600);
    idl.begin();
    setupMic();

    display.init();
    displayLoop();
}

// Return light triangular shaped dithernoise for
// 32bit int that needs to be downsampled to 16 bit.
int32_t dithernoise() {
    int32_t x1 = esp_random();
    int32_t x2 = esp_random();
    int32_t triangleRand = ((x1 >> 15) + (x2 >> 15)) / 2;
    return triangleRand;
}

void loop() {
    delay(250);
    idl.loop();
    displayLoop();

    // Annoying variable needed to read i2s. It not used for anything.
    size_t ps;

    // Read the sound data from i2s port
    int esp_err =
        i2s_read(I2S_PORT, &samples,
                 BLOCK_SIZE, // The doc says bytes, but its elements.
                 &ps,
                 portMAX_DELAY); // No timeout. wait til buffer is full

    // Prepare the spamles for FFT
    // Discard the least significant bits to make it from 32 bit to 16 bit.
    // Also add some dithering noise to help with downsampling artifacts
    for (uint16_t i = 0; i < BLOCK_SIZE; i++) {
        vReal[i] = short((samples[i] + dithernoise()) >> 16);
        vImag[i] = 0.0; // Imaginary part must be zeroed in case of looping to
                        // avoid wrong calculations and overflows
    }

    // Do the FFT
    FFT.DCRemoval();
    FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(FFT_FORWARD);
    FFT.ComplexToMagnitude(); // magintude is half the size

    // init the bins with zeros
    for (int i = 0; i < nBins; i++) {
        bins[i] = 0;
    }

    // go over each bin
    for (int i = 0; i <= nBins; i++) {
        // Calculate the offset in the vReal array
        int offset = i * binsToBin;
        // Go the offset and find the maximum value in vReal array
        for (int j = offset; j < offset + binsToBin; j++) {
            bins[i] = max(bins[i], log2(vReal[j]));
        }
    }

    // Clear the serial
    Serial.print(char(27)); // Print "esc"
    Serial.print("[2J");
    Serial.println();

    // prepare a json buffer.
    DynamicJsonBuffer jsonBuffer;

    // create a root object
    JsonObject &j_root = jsonBuffer.createObject();
    // Set message type to measurement
    j_root["type"] = "measurement";
    // Create an object to store values
    JsonObject &j_values = j_root.createNestedObject("values");
    // Create two objects. One for DBM valuse and another for the Frequency
    JsonObject &j_dbm = j_values.createNestedObject("dbm");
    JsonObject &j_frequency = j_values.createNestedObject("frequency");
    // Store the peak frequency
    j_frequency["value"] = FFT.MajorPeak();

    // Store the Bins as key value pairs.
    // The key is the start frequency of the bin
    for (int i = 0; i < nBins; i++) {
        // calculate what the offset was in the vReal array
        int offset = i * binsToBin;
        // calculate the frequency for this bin.
        float freq = ((offset * samplingFrequency) / 1024);
        // store the bin in json.
        j_dbm[String(freq)] = bins[i];
    }

    // Print to serial
    j_dbm.prettyPrintTo(Serial);

    double x = FFT.MajorPeak();
    Serial.println(x, 6); // Print out what frequency is the most dominant.

    // Print to mqtt // This is really really slow for some reason.
    idl.sendRaw("microphone", j_root);
}
