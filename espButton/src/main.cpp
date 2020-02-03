#include "Arduino.h"
#include "WiFi.h"

#include <Wire.h>
#include <SPI.h>

#include "SSD1306Wire.h"        

#include <PubSubClient.h>

#include <EasyButton.h>

void connectToWiFi() {
 
  WiFi.mode(WIFI_STA);
  WiFi.begin("idlcase", "Aalborg9000Robotlab");
  Serial.print("Connecting to "); Serial.println("idlcase");
 
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
 
    if ((++i % 16) == 0) {
      Serial.println(F(" still trying to connect"));
    }
  }
 
  Serial.print(F("Connected. My IP address is: "));
  Serial.println(WiFi.localIP());
}

SSD1306Wire display(0x3c, 5, 4);
char deviceId[24];
void displayLoop() {
    display.clear();
    if (WiFi.status() != WL_CONNECTED) {
      display.setFont(ArialMT_Plain_24);
      display.drawString(0, 20, "Connecting...");
    } else {
      display.setFont(ArialMT_Plain_24);
      display.drawString(0, 0, deviceId);
      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 25, WiFi.localIP().toString());
    }

    display.display();

}

WiFiClient espClient;
PubSubClient client(espClient);
char mqtt_topic[32];

EasyButton button(26);
char msgBuf[50];

void onPressed() {
  client.publish(mqtt_topic, "{\"type\": \"btnGreenShort\", \"msg\": \"Green button single pressed\"}");
}

void onPressedForDuration() {
  client.publish(mqtt_topic, "{\"type\": \"btnGreenLong\", \"msg\": \"Green button long pressed\"}");
}

void setup() {
    Serial.begin(115200);
    delay(100);
    sprintf(deviceId, "%06X", (uint)(ESP.getEfuseMac() >> 24));
    sprintf(mqtt_topic, "idl/%s/event", deviceId);

    // Initialising the UI will init the display too.
    display.init();

    // display.flipScreenVertically();

    displayLoop(); 

    connectToWiFi();
    
    client.setServer("10.13.37.1", 1883);

    button.begin();
    button.onPressedFor(1000, onPressedForDuration);
    button.onPressed(onPressed);

    Serial.println("Setup done");
}

void reconnect() {
  if(WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect(deviceId)) {
      Serial.println("MQTT connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  button.read();

  displayLoop();

}