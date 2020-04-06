#include "Arduino.h"

#include <Adafruit_BME280.h>

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
int recu[2]; // Storage of module data
int lumiere;


/***************************************************************************/
/***************************************************************************/

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
// BME sensor instance
Adafruit_BME280 bme; // I2C

// bme setup loop
void printBME() {

  if (!bme.begin(0x76)) {
    Serial.println(F("Could not find a valid BME280 sensor, check wiring!"));
  }
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");

  Serial.print("Pressure = ");

  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

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

    printBME();

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
    
    // prepare a json buffer.
    DynamicJsonBuffer jsonBuffer;

    // create a root object
    JsonObject &j_root = jsonBuffer.createObject();
    // Set message type to measurement
    j_root["type"] = "measurement";
    // Create an object to store values
    JsonObject &j_values = j_root.createNestedObject("values");
    // Create a value object with the name of the unit.  
    JsonObject &j_c = j_values.createNestedObject("Celsius");
    j_c["value"] = bme.readTemperature();
    idl.sendRaw("temperature", j_root);
    //j_root.prettyPrintTo(Serial);
    j_values.remove("Celsius");

    JsonObject &j_h = j_values.createNestedObject("RH");
    j_h["value"] = bme.readHumidity();
    idl.sendRaw("humidity", j_root);
    //j_root.prettyPrintTo(Serial);
    j_values.remove("RH");

    JsonObject &j_p = j_values.createNestedObject("hPa");
    j_p["value"] = bme.readPressure();
    idl.sendRaw("pressure", j_root);
    //j_root.prettyPrintTo(Serial);
    j_values.remove("hPa");

    JsonObject &j_l = j_values.createNestedObject("luminance");
    j_l["value"] = readLuminance();
    idl.sendRaw("luminance", j_root);
    //j_root.prettyPrintTo(Serial);
    j_values.remove("luminance");


    displayLoop();

    delay(250);
}