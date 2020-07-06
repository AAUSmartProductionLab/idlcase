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
/***************************************************************************/
/***************************************************************************/

/*=========================================================================*/
// IDLNetworking instance

IDLNetworking idl = IDLNetworking("espBME280", VERSION);

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
    
    
    float temp = bme.readTemperature();
    idl.pushMeasurement("temperature","sensor1","celcius", temp);
    float hum = bme.readHumidity();
    idl.pushMeasurement("humidity","sensor1","%",hum);
    float pres = bme.readPressure();
    idl.pushMeasurement("pressure","sensor1","Pa",pres);
    
    idl.sendAll();

    displayLoop();

}