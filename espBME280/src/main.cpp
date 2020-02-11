#include "Arduino.h"
#include "WiFi.h"

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BME280.h>

#include "SSD1306Wire.h"        

#include <PubSubClient.h>
#include <esp32fota.h>

#include "version.h"

// where to listen for updates
const char* otaTopic = "idlota/espBME280";

// where to check for updates
const char* otaMeta = "http://10.13.37.1/db/espBME280";

const char* firmwareType = "espBME280";

// every time a new firmware is released, existing esp devices
// will check this type and version number to see if they need updating
esp32FOTA fota(firmwareType, VERSION);

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

Adafruit_BME280 bme; // I2C
void setupSensor() {

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

void scan() {
  byte error, address;
  int nDevices;
 
  Serial.println("Scanning...");
 
  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
 
    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");
 
      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
 
  delay(500);           // wait 5 seconds for next scan

}

SSD1306Wire display(0x3c, 5, 4);
char deviceId[24];
char versionString[24];
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
      display.drawString(0, 36, versionString);
    }

    display.display();

}

void tryOTA() {
  fota.checkURL = otaMeta;

  bool updatedNeeded = fota.execHTTPcheck();
  if (updatedNeeded) {

    display.clear();
    display.setFont(ArialMT_Plain_24);
    display.drawString(0, 20, "Updating");
    display.display();

    fota.execOTA();
  }
}


WiFiClient espClient;
PubSubClient client(espClient);
char mqtt_topic_temperature[32];
char mqtt_topic_humidity[32];
char mqtt_topic_pressure[32];

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
  // We will just assume any received message is a request for OTA updates
  // FIXME
  tryOTA();
}


void setup() {
    Serial.begin(115200);
    delay(100);

    sprintf(deviceId, "%06X", (int)(ESP.getEfuseMac() >> 24));
    sprintf(mqtt_topic_temperature, "idl/%s/temperature", deviceId);
    sprintf(mqtt_topic_humidity, "idl/%s/humidity", deviceId);
    sprintf(mqtt_topic_pressure, "idl/%s/pressure", deviceId);
    sprintf(versionString, "%s: %d", firmwareType, VERSION);

    // Initialising the UI will init the display too.
    display.init();

    // display.flipScreenVertically();

    displayLoop(); 

    scan();
    
    setupSensor();

    connectToWiFi();

    tryOTA();

    client.setServer("10.13.37.1", 1883);
    client.setCallback(callback);

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
  client.subscribe(otaTopic);
}

char msgBuf[50];
void loop() {
    if (!client.connected()) {
      reconnect();
    }
    
    client.loop();
    
    sprintf(msgBuf, "{\"value\": %f, \"unit\":\"*C\"}", bme.readTemperature());
    client.publish(mqtt_topic_temperature, msgBuf);

    sprintf(msgBuf, "{\"value\": %f, \"unit\":\"hPa\"}", bme.readPressure() / 100.0F);
    client.publish(mqtt_topic_pressure, msgBuf);

    sprintf(msgBuf, "{\"value\": %f, \"unit\":\"%%RH\"}", bme.readHumidity());
    client.publish(mqtt_topic_humidity, msgBuf);

    displayLoop();

    delay(250);
}