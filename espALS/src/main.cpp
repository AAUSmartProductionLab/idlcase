#include "Arduino.h"

#include <ArduinoJson.h>

#include "version.h"

/***************************************************************************/
// oled screen
#include "SSD1306Wire.h"

/***************************************************************************/
// Industrial Data Logger networking implementation.
#include <IDLNetworking.h>

/***************************************************************************/
#define CS 15 // Assignment of the CS pin

#include <SPI.h> // call library



/*=========================================================================*/
// IDLNetworking instance

IDLNetworking idl = IDLNetworking("espALS", VERSION);

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
// esp setup loop
void setup() {
    Serial.begin(115200);
    delay(100);

    idl.begin();

    // Initialising the UI will init the display too.
    display.init();
    displayLoop(); 


    SPI.begin(14,12,13,15);                // initialization of SPI port
    SPI.setDataMode(SPI_MODE3); // configuration of SPI communication in mode 0
    SPI.setClockDivider(SPI_CLOCK_DIV8); // configuration of clock at 1MHz
    pinMode(CS, OUTPUT);

    Serial.println("Setup done");
}

uint16_t readLuminance(){
    digitalWrite(CS, LOW); // activation of CS line
    uint16_t data = SPI.transfer16(0); 
    digitalWrite(CS, HIGH); // deactivation of CS line
    data = data >> 4; // reduce to 12 bit's of data TODO there might actually only be 11 bits 
    return data;
}

void loop() {
    idl.loop();
    
    idl.pushMeasurement("luminance", "sensor 1", "value", readLuminance());
    idl.sendMeasurements();

    displayLoop();

}