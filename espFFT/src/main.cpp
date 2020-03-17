#include <WiFi.h>
#include <Ticker.h>
#include <arduinoFFT.h>
#include <driver/i2s.h>

const i2s_port_t I2S_PORT = I2S_NUM_0;
const int BLOCK_SIZE = 1024;

const double samplingFrequency = 44100;
const uint8_t amplitude = 150;

double vReal[BLOCK_SIZE];
double vImag[BLOCK_SIZE];
int32_t samples[BLOCK_SIZE];


arduinoFFT FFT = arduinoFFT(); /* Create FFT object */

// Connecting to the Internet
const char * ssid = "idlcase";
const char * password = "Aalborg9000Robotlab";

int bands[BLOCK_SIZE/16];

void setupMic() {
  Serial.println("Configuring I2S...");
  esp_err_t err;

  // The I2S config as per the example
  const i2s_config_t i2s_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // Receive, not transfer
      .sample_rate = samplingFrequency,                        
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // could only get it to work with 32bits
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // although the SEL config should be left, it seems to transmit on right
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     // Interrupt level 1
      .dma_buf_count = 4,                           // number of buffers
      .dma_buf_len = BLOCK_SIZE                     // samples per buffer
  };

  // The pin config as per the setup
    i2s_pin_config_t pin_config = {
        .bck_io_num = 26,  // IIS_SCLK
        .ws_io_num = 32,   // IIS_LCLK
        .data_out_num = -1,// IIS_DSIN
        .data_in_num = 33  // IIS_DOUT
    };

  // Configuring the I2S driver and pins.
  // This function must be called before any I2S driver read/write operations.
  err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("Failed installing driver: %d\n", err);
    while (true);
  }
  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("Failed setting pin: %d\n", err);
    while (true);
  }
  Serial.println("I2S driver installed.");
}

void setup() {

  Serial.begin(115200);
  // put your setup code here, to run once:
  /*WiFi.begin(ssid, password);
  while(WiFi.status()!=WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
*/
  
  delay(1000);
  Serial.println("Setting up mic");
  setupMic();
  Serial.println("Mic setup completed");

  delay(1000);

}
/*
void getData() {

  String json = "[";
  for (int i = 0; i < 8; i++) {
    if (i > 0) {
      json +=", ";
    }
    json += "{\"bin\":";
    json += "\"" + labels[i] + "\"";
    json += ", \"value\":";
    json += String(bands[i]);
    json += "}"; 
  }
  json += "]";
}
*/
#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

void PrintVector(double *vData, uint32_t bufferSize, uint8_t scaleType)
{
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa;
    /* Print abscissa value */
    switch (scaleType)
    {
      case SCL_INDEX:
        abscissa = (i * 1.0);
	break;
      case SCL_TIME:
        abscissa = ((i * 1.0) / samplingFrequency);
	break;
      case SCL_FREQUENCY:
        abscissa = ((i * 1.0 * samplingFrequency) / 1024);
	break;
    }
    Serial.print(abscissa, 6);
    if(scaleType==SCL_FREQUENCY)
      Serial.print("Hz");
    Serial.print(" ");
    Serial.println(vData[i], 4);
  }
  Serial.println();
}
void loop() {
  // Read multiple samples at once and calculate the sound pressure

  int num_bytes_read = i2s_read_bytes(I2S_PORT, 
                                      (char *)samples, 
                                      BLOCK_SIZE,     // the doc says bytes, but its elements.
                                      portMAX_DELAY); // no timeout

  for (uint16_t i = 0; i < BLOCK_SIZE; i++) {
    vReal[i] = samples[i] << 8;
    vImag[i] = 0.0; //Imaginary part must be zeroed in case of looping to avoid wrong calculations and overflows
  }

  FFT.Windowing(vReal, BLOCK_SIZE, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, BLOCK_SIZE, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, BLOCK_SIZE);

 for (int i = 0; i < (BLOCK_SIZE/16); i++) {
    bands[i] = 0;
  }

  int index = 0;

  for (int i = 2; i < BLOCK_SIZE; i++){ 
    index = (int) floor(i * 16);
    bands[i] = max(bands[index], (int)(vReal[i]/amplitude)); // 125Hz
  }

  for (int i = 0; i < (BLOCK_SIZE/16); i++) {
    Serial.print(String(bands[i]));
    Serial.print(",");
  }

  Serial.println();

  // PrintVector(vReal, (1024 >> 1), SCL_FREQUENCY);
  double x = FFT.MajorPeak(vReal, 1024, 44100);
  Serial.println(x, 6); //Print out what frequency is the most dominant.
}

