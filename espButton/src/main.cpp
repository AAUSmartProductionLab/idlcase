#include "Arduino.h"

#include <ArduinoJson.h>

#include <EasyButton.h>

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

IDLNetworking idl = IDLNetworking("espButton", VERSION);

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


EasyButton button(26); // pin 26

void onPressed() {
  // prepare a json buffer.
    DynamicJsonBuffer jsonBuffer;

    // create a root object
    JsonObject &j_root = jsonBuffer.createObject();
    // Set message type to event
    j_root["type"] = "event";
    j_root["msg"] = "button pressed shortly";
    j_root["payload"] = "shortButtonPress";
    idl.sendRaw("buttonpress", j_root);
    //j_root.prettyPrintTo(Serial);
}
void onPressedForDuration() {
  // prepare a json buffer.
    DynamicJsonBuffer jsonBuffer;

    // create a root object
    JsonObject &j_root = jsonBuffer.createObject();
    // Set message type to event
    j_root["type"] = "event";
    j_root["msg"] = "button pressed more than a second";
    j_root["payload"] = "longButtonPress";
    idl.sendRaw("buttonpress", j_root);
    //j_root.prettyPrintTo(Serial);
}



void setup() {
    Serial.begin(115200);
    delay(100);

    idl.begin();

    // Initialising the UI will init the display too.
    display.init();
    displayLoop();

    button.begin();
    button.onPressedFor(1000, onPressedForDuration);
    button.onPressed(onPressed);

    Serial.println("Setup done");
}

void loop() {
    idl.loop();

    button.read();

    displayLoop();
}


