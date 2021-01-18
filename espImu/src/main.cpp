#include <arduinoFFT.h>
#include "version.h"
#include "MPU9250.h"


/***************************************************************************/
// oled screen
#include "SSD1306Wire.h"

/***************************************************************************/
// Industrial Data Logger networking implementation.

#include <IDLNetworking.h>
#define IDL_JSON_SIZE 8192
/***************************************************************************/


/***************************************************************************/
/***************************************************************************/

const int BLOCK_SIZE = 1024;

const int samplingFrequency = 1000;
const int nBins = 32;

const float binSize = samplingFrequency / (BLOCK_SIZE * 1.0);
const int binsToBin = ceil(BLOCK_SIZE / nBins);
double bins[nBins];

double vReal[BLOCK_SIZE];
double vImag[BLOCK_SIZE];
int16_t samples[BLOCK_SIZE];

arduinoFFT FFT = arduinoFFT(vReal, vImag, BLOCK_SIZE,
                            samplingFrequency); /* Create FFT object */


/*=========================================================================*/
// imu instance
//TwoWire I2CMPU = TwoWire(1);
MPU9250 mpu(Wire, 0x68);
unsigned long lastRead = micros();

// I2C device found at address 0x0C  ! bme
// I2C device found at address 0x68  ! mpu9250
// I2C device found at address 0x76  ! mpu

/*=========================================================================*/
// idl instance
IDLNetworking idl = IDLNetworking("espImu", VERSION);

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



void setup() {
    Serial.begin(115200);
    idl.begin();


    Wire.begin();
    display.init();
    displayLoop();
    

    // setup i2c for the mpu
    //I2CMPU.begin(21,22,400000);

    // start communication with IMU 
    int status = mpu.begin();
    if (status < 0) {
        Serial.println("IMU initialization unsuccessful");
        Serial.println("Check IMU wiring or try cycling power");
        Serial.print("Status: ");
        Serial.println(status);
        while(1) {}
    }
    // setting the accelerometer full scale range to +/-8G
    mpu.setAccelRange(MPU9250::ACCEL_RANGE_16G);
    // setting the gyroscope full scale range to +/-500 deg/s
    mpu.setGyroRange(MPU9250::GYRO_RANGE_500DPS);
    // setting DLPF bandwidth to 184 Hz
    mpu.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_184HZ);
    // setting SRD to 0 for a 1000 Hz update rate
    mpu.setSrd(0);

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
    idl.loop(0);
    displayLoop();

    int counter = 0;
    while (counter < BLOCK_SIZE){
        
        while(micros() <= lastRead + 990){/*do nothing */}
        lastRead=micros();

        mpu.readSensor();

        // transfer the fifo buffer til vReal
        vReal[counter] = mpu.getAccelX_mss();
        vImag[counter] = 0;
        counter++ ;
    }

    // Do the FFT
    FFT.DCRemoval();
    FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(FFT_FORWARD);
    FFT.ComplexToMagnitude(); // magintude is half the size

    Serial.print("major peak = "); 
    Serial.println(FFT.MajorPeak());
    

    // init the bins with zeros
    for (int i = 0; i < nBins; i++) {
        bins[i] = 0;
    }

    // walk over half of the data (half is just mirror)
    for (int i = 0; i < BLOCK_SIZE/2; i++){
        int binIndex = floor(i / (BLOCK_SIZE/2/nBins));

        bins[binIndex] = max(bins[binIndex], vReal[i]);
    }

    // for (int i = 0; i < nBins; i++){
    //     Serial.print(bins[i]); Serial.print(", ");
    // }

    // for (int i = 0; i < BLOCK_SIZE/2; i++){
    //     Serial.print(vReal[i]); Serial.print(", ");
    // }

    idl.pushMeasurement("vibrationMajorPeak","sensor 1","Hz",FFT.MajorPeak());

    
    idl.pushMeasurement("temperature","sensor 1","celsius",mpu.getTemperature_C());

    Serial.println(mpu.getTemperature_C());
    
    // Store the Bins as key value pairs.
    // The key is the start frequency of the bin
    // Walk over the bins
    for (int i = 0; i < nBins; i++) {
        // calculate what the offset was in the vReal array
        int offset = i * (BLOCK_SIZE/2/nBins);
        // calculate the frequency for this bin.
        char freq[20];
        sprintf(freq,"%.0f", offset * binSize);
        // store the bin in json.
        auto msg = idl.pushMeasurement("vibration","sensor 1","m/s2/Hz",bins[i]);
        idl.addTag(msg,"band",freq);
    }

     idl.sendMeasurements();

}
