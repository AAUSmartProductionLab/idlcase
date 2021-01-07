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
/***************************************************************************/
/***************************************************************************/

/*=========================================================================*/
// IDLNetworking instance

IDLNetworking idl = IDLNetworking("espAnalog", VERSION);

/*=========================================================================*/
// analog pins
int pins[] = {32,33,34,35};
int nPins = 4;

/*=========================================================================*/
// OLED screen instance
SSD1306Wire display(0x3c, 5, 4);

// display loop
void displayLoop() {
  // Displays device information on the oled dispaly. 
    display.clear();
    if (WiFi.status() != WL_CONNECTED) {
        display.setFont(ArialMT_Plain_24);
        display.drawString(0, 0, idl.getDeviceId());
        display.setFont(ArialMT_Plain_10);
        display.drawString(0, 25, "Connecting...");
        display.drawString(0, 36, idl.getVersionString());
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

void setup() {
    Serial.begin(115200);
    delay(100);

    idl.begin();

    // Initialising the UI will init the display too.
    display.init();
    displayLoop();

    Serial.println("Setup done");
}

void loop() {
    idl.loop(4000);

    char pin_string[20] ;

    for (int i = 0; i < nPins; i++){
        int ADC_VALUE = analogRead(pins[i]);
        float voltage = (ADC_VALUE * 3.3 ) / (4095);  // NewValue = (((OldValue - OldMin) * (NewMax - NewMin)) / (OldMax - OldMin)) + NewMin.

        sprintf(pin_string, "pin:%i", pins[i]);
        idl.pushMeasurement("voltage",pin_string, "V", voltage);
    }
    idl.sendMeasurements();

    displayLoop();
}


