#include <arduinoFFT.h>
#include <driver/i2s.h>

const i2s_port_t I2S_PORT = I2S_NUM_0;
const int BLOCK_SIZE = 1024;

const double samplingFrequency = 44100;
const int maxMicFreq = 15000;
const int nBins = 32;

const float binSize = samplingFrequency / BLOCK_SIZE;
const int maxBins = maxMicFreq / binSize;
double bins[nBins];

double vReal[BLOCK_SIZE];
double vImag[BLOCK_SIZE];
int32_t samples[BLOCK_SIZE];

arduinoFFT FFT = arduinoFFT(vReal, vImag, BLOCK_SIZE,
                            samplingFrequency); /* Create FFT object */

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
  Serial.begin(1000000);
  setupMic();
}

// Return light triangular shaped dithernoise for 
// 32bit int that needs to be downsampled to 16 bit.
int32_t dithernoise() { 
  int32_t x1 = esp_random();
  int32_t x2 = esp_random();
  int32_t triangleRand = ((x1>>15) + (x2>>15)) / 2;
  return triangleRand;
}

void loop() {
  // Read multiple samples at once and calculate the sound pressure

  // annoying variable needed to read i2s. It not used for anything.
  size_t ps;

  // Read the sound data from i2s port
  int esp_err = i2s_read(I2S_PORT, &samples,
                         BLOCK_SIZE, // the doc says bytes, but its elements.
                         &ps,
                         portMAX_DELAY); // no timeout

  // Prepare the spamles for FFT
  // Discard the least significant bits to make it from 32 bit to 16 bit.
  // Also add some dithering noise to help with downsampling
  for (uint16_t i = 0; i < BLOCK_SIZE; i++) {
    vReal[i] = short((samples[i] + dithernoise()) >> 16);
    vImag[i] = 0.0; // Imaginary part must be zeroed in case of looping to avoid
                    // wrong calculations and overflows
  }

  FFT.DCRemoval();
  FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(FFT_FORWARD);
  FFT.ComplexToMagnitude(); // magintude is half the size

  // init the bins with zeros
  for (int i = 0; i < nBins; i++) {
    bins[i] = 0;
  }

  int index = 0;
  for (int i = 2; i <= maxBins; i++) {
    index = i / (maxBins / nBins);
    bins[index] = max(bins[index], log2(vReal[i]));
  }

  Serial.print(char(27)); // Print "esc"
  Serial.print("[2J");

  // PrintVector(vReal,BLOCK_SIZE,SCL_FREQUENCY);

  for (int i = 0; i < nBins; i++) {
    Serial.println(String(bins[i]));
  }

  Serial.println();

  // PrintVector(vReal, (1024 >> 1), SCL_FREQUENCY);
  double x = FFT.MajorPeak();
  Serial.println(x, 6); // Print out what frequency is the most dominant.
}

// abscissa = ((i * 1.0 * samplingFrequency) / 1024);
