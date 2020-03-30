#include "Arduino.h"

#include <Adafruit_BME280.h>

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

/*=========================================================================*/
// IDLNetworking instance

IDLNetworking idl = IDLNetworking("espBME280");

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
    }

    display.display();
}

/*=========================================================================*/
// BME sensor instance
Adafruit_BME280 bme; // I2C

// bme setup loop
void readSensor() {

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

    readSensor();

    Serial.println("Setup done");
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
    j_root.prettyPrintTo(Serial);
    j_values.remove("Celsius");

    JsonObject &j_h = j_values.createNestedObject("RH");
    j_h["value"] = bme.readHumidity();
    idl.sendRaw("humidity", j_root);
    j_root.prettyPrintTo(Serial);
    j_values.remove("RH");

    JsonObject &j_p = j_values.createNestedObject("hPa");
    j_p["value"] = bme.readPressure();
    idl.sendRaw("pressure", j_root);
    j_root.prettyPrintTo(Serial);

    displayLoop();

    delay(250);
}