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
const int maxMicFreq = 1000;
const int nBins = 16;

const float binSize = samplingFrequency / BLOCK_SIZE;
const int usefulBins = maxMicFreq / binSize;
const int binsToBin = ceil(usefulBins / nBins);
double bins[nBins];

double vReal[BLOCK_SIZE];
double vImag[BLOCK_SIZE];
int16_t samples[BLOCK_SIZE];

arduinoFFT FFT = arduinoFFT(vReal, vImag, BLOCK_SIZE,
                            samplingFrequency); /* Create FFT object */


/*=========================================================================*/
// imu instance
TwoWire I2CMPU = TwoWire(1);
MPU9250FIFO mpu(I2CMPU,0x68);
size_t fifoSize;
float fifoBuffer[85]; //  512byte/ 6 byte data = 85 data points (accel_x = 2byte, accel_y = 2byte, accel_z = 2byte)
// I2C device found at address 0x0C  !
// I2C device found at address 0x68  !
// I2C device found at address 0x76  !

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
    I2CMPU.begin(21,22,400000);

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
    mpu.enableFifo(true,false,false,false);

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
        
        delay(25);

        mpu.readFifo();
        mpu.getFifoAccelX_mss(&fifoSize, fifoBuffer);
        
        if (fifoSize >= 85){
            Serial.print("fifo buffer overflow"); 
            Serial.print(fifoSize);
        } 

        // transfer the fifo buffer til vReal
        for (size_t i = 0; i < fifoSize && counter < BLOCK_SIZE; i++, counter++)
        {
            vReal[counter] = fifoBuffer[i];
            vImag[counter] = 0;
        }
    }

    // for(int i = 0; i < BLOCK_SIZE; i++){
    //     Serial.print(vReal[i]);
    // } 
    

    // // Prepare the spamles for FFT
    // // Discard the least significant bits to make it from 32 bit to 16 bit.
    // // Also add some dithering noise to help with downsampling artifacts
    // for (uint16_t i = 0; i < BLOCK_SIZE; i++) {
    //     vReal[i] = short((samples[i] + dithernoise()) >> 16);
    //     vImag[i] = 0.0; // Imaginary part must be zeroed in case of looping to
    //                     // avoid wrong calculations and overflows
    // }
    for (int i = 0; i < BLOCK_SIZE/2; i++){
        Serial.print(vReal[i]); Serial.print(", ");
    }

    // Do the FFT
    FFT.DCRemoval();
    FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(FFT_FORWARD);
    FFT.ComplexToMagnitude(); // magintude is half the size
    Serial.print("major peak = "); 
    Serial.println(FFT.MajorPeak());
    
    //Serial.print(" \n");

    // init the bins with zeros
    for (int i = 0; i < nBins; i++) {
        bins[i] = 0;
    }

    // // go over each bin
    // for (int i = 0; i <= nBins; i++) {
    //     // Calculate the offset in the vReal array
    //     int offset = i * binsToBin;
    //     // Go the offset and find the maximum value in vReal array
    //     for (int j = offset; j < offset + binsToBin; j++) {
    //         bins[i] = max(bins[i], log2(vReal[j]));
    //     }
    // }

    // idl.pushMeasurement("microphone","sensor 1","Hz",FFT.MajorPeak());

    // // Store the Bins as key value pairs.
    // // The key is the start frequency of the bin
    // for (int i = 0; i < nBins; i++) {
    //     // calculate what the offset was in the vReal array
    //     int offset = i * binsToBin;
    //     // calculate the frequency for this bin.
    //     char freq[20];
    //     sprintf(freq,"%.0f", ((offset * samplingFrequency) / 1024.0));
    //     //int freq2 = ((offset * samplingFrequency) / 1024.0);
    //     // store the bin in json.
    //     auto msg = idl.pushMeasurement("microphone","sensor 1","dBm",bins[i]);
    //     idl.addTag(msg,"band",freq);
    // }

    // idl.sendMeasurements();

}
