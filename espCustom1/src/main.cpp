#include <ArduinoJson.h>

#include "version.h"

/***************************************************************************/
// oled screen
#include "SSD1306Wire.h"

/***************************************************************************/
// Industrial Data Logger networking implementation.
#include <IDLNetworking.h>

/***************************************************************************/
// OneWire and DS18B20 libraries
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 26
#define TEMPERATURE_PRECISION 11
#define NUMBER_OF_THERMOMETERS 8

/***************************************************************************/
#define CS 15 // Assignment of the CS pin

#include <SPI.h> // call library

/***************************************************************************/
/***************************************************************************/



/*=========================================================================*/
/*=========================================================================*/
// Onewire and DS18B20 sensor instance
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);

int deviceCount = 0;

// Variable to hold temporary device addresses
DeviceAddress thermometer;

// Variable to hold temporary temperature values 
float tempC; 

/*=========================================================================*/
// IDLNetworking instance

IDLNetworking idl = IDLNetworking("espCustom1", VERSION);

/*=========================================================================*/
// OLED screen instance
SSD1306Wire display(0x3c, 5, 4);

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
        display.drawString(0, 36, idl.getVersionString());
    }

    display.display();
}

/*=========================================================================*/
// read the light sensor. 
uint16_t readLuminance(){
    digitalWrite(CS, LOW); // activation of CS line
    uint16_t data = SPI.transfer16(0); 
    digitalWrite(CS, HIGH); // deactivation of CS line
    data = data >> 4; // reduce to 12 bit's of data TODO there might actually only be 11 bits 
    return data;
}
/*=========================================================================*/
// printing a thermometer address
void printAddress(DeviceAddress deviceAddress)
{ 
  for (uint8_t i = 0; i < 8; i++)
  {
    Serial.print("0x");
    if (deviceAddress[i] < 0x10) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    if (i < 7) Serial.print(", ");
  }
  Serial.println("");
}

/*=========================================================================*/
void array_to_string(byte array[], unsigned int len, char buffer[])
{
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}
/*=========================================================================*/

// dallas temperature sensors setup
void sensorsSetup() {
    sensors.begin();

    // locate devices on the bus
    Serial.print("Locating devices...");
    Serial.print("Found ");
    deviceCount = 3;//sensors.getDeviceCount();
    Serial.print(deviceCount, DEC);
    Serial.println(" devices.");
    Serial.println("");

    Serial.println("Printing addresses...");
    for (int i = 0; i < deviceCount; i++)    {
        Serial.print("Sensor ");
        Serial.print(i + 1);
        Serial.print(" : ");
        sensors.getAddress(thermometer, i);
        printAddress(thermometer);

        // set the resolution to x bit per device
        sensors.setResolution(thermometer, TEMPERATURE_PRECISION);
    }

    // setup SPI for the Ambient Light Sensor.    
    delay(500);
    SPI.begin(14,12,13,15);                // initialization of SPI port
    SPI.setDataMode(SPI_MODE3); // configuration of SPI communication in mode 0
    SPI.setClockDivider(SPI_CLOCK_DIV8); // configuration of clock at 1MHz
    pinMode(CS, OUTPUT);



}

/*=========================================================================*/
// esp setup loop
void setup() {
    Serial.begin(115200);
    delay(100);
    
    sensorsSetup();

    idl.begin();

    // Initialising the UI will init the display too.
    display.init();
    displayLoop(); 

    Serial.println("Setup done");
}


void loop() {
    idl.loop();
    
    displayLoop();

    idl.pushMeasurement("light","sensor 1","value",readLuminance());

    // Send command to all the sensors for temperature conversion
    sensors.requestTemperatures();
    
    // Store temperature from each sensor
    for (int i = 0; i < deviceCount; i++){
        tempC = sensors.getTempCByIndex(i);
        sensors.getAddress(thermometer, i);

        char hex_string[20] ;
        array_to_string(thermometer, 8, hex_string);

        idl.pushMeasurement("temperature",hex_string,"celcius",tempC);
         
    }    

    idl.sendMeasurements();

}