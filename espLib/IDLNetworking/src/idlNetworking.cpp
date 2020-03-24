#include "IDLNetworking.h"

IDLNetworking::IDLNetworking(const char *deviceType) {
  deviceType = deviceType;
}

void IDLNetworking::begin() {

  sprintf(deviceId, "%06X", (uint)(ESP.getEfuseMac() >> 24));
  sprintf(mqtt_out_toppic, "idl/%s/event", deviceId);
  sprintf(versionString, "Version: %d", VERSION);

  readFileSystem();
  wifiPortal();
  writeFileSystem();

  PSClient.setServer(MQTTServer, String(MQTTPort).toInt());
  PSClient.setCallback([this](char *topic, byte *payload, unsigned int length) {
    this->PSCallback(topic, payload, length);
  });
}

/**************************************************************************/
// check the web server if there's a new update. This is done at boot and
// whenever a new firmware is announced over MQTT
void IDLNetworking::tryOTA() {
  fota.checkURL = otaMeta;

  bool updatedNeeded = fota.execHTTPcheck();
  if (updatedNeeded) {
    fota.execOTA();
  }
}

void IDLNetworking::PSCallback(char *topic, byte *payload,
                               unsigned int length) {
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

void IDLNetworking::readFileSystem() {
  // read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    if (SPIFFS.exists("/config.json")) {
      // file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(MQTTServer, json["MQTTServer"]);
          strcpy(MQTTPort, json["MQTTPort"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  // end read

  Serial.print("MQTTServer - ");
  Serial.print(MQTTServer);
  Serial.print(":");
  Serial.println(MQTTPort);
}

void IDLNetworking::writeFileSystem() {

  // save the custom parameters to FS
  Serial.println("saving config");
  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();
  json["MQTTServer"] = MQTTServer;
  json["MQTTPort"] = MQTTPort;

  json["ip"] = WiFi.localIP().toString();
  json["gateway"] = WiFi.gatewayIP().toString();
  json["subnet"] = WiFi.subnetMask().toString();

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("failed to open config file for writing");
  }

  json.prettyPrintTo(Serial);
  json.printTo(configFile);
  configFile.close();
  // end save
}

bool IDLNetworking::wifiPortal(int timeout) {
  WiFiManagerParameter custom_text1(
      "<hr/><p style=\"margin-bottom:0em; margin-top:1em;\"><b>MQTT "
      "settings</b></p>");
  WiFiManagerParameter customMQTTServer("server", "mqtt server", MQTTServer,
                                        40);
  WiFiManagerParameter customMQTTPort("port", "mqtt port", MQTTPort, 5);
  WiFiManagerParameter custom_text2(
      "<hr/><p style=\"margin-bottom:0em; margin-top:1em;\"><b>Firmware "
      "settings</b></p>");
  WiFiManagerParameter customFwServer("server", "Firmware Server", fwServer,
                                      128);

  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it
  // around
  WiFiManager wifiManager;

  // set config save notify callback
  wifiManager.setSaveConfigCallback([this] { this->saveConfigCallback(); });

  // add all your parameters here
  wifiManager.addParameter(&custom_text1);
  wifiManager.addParameter(&customMQTTServer);
  wifiManager.addParameter(&customMQTTPort);
  wifiManager.addParameter(&custom_text2);
  wifiManager.addParameter(&customFwServer);

  /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
  // reset settings - for testing
  //  wifiManager.resetSettings();
  /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

  // set minimum quality of signal so it ignores AP's under that quality
  // defaults to 8%
  wifiManager.setMinimumSignalQuality(8);

  // fetches ssid and pass and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration

  wifiManager.setTimeout(timeout);
  char buf[41];
  sprintf(buf, "CONFIGURE ME - %s", deviceId);
  wifiManager.autoConnect(buf);

  if (shouldSaveConfig) {
    // read updated parameters. Might not have changed but it should be safe to
    // update them again.
    strcpy(MQTTServer, customMQTTServer.getValue());
    strcpy(MQTTPort, customMQTTPort.getValue());
    strcpy(fwServer, customFwServer.getValue());

    Serial.println("local ip");
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.gatewayIP());
    Serial.println(WiFi.subnetMask());

    writeFileSystem();
  }
}

/***************************************************************************/
// MQTT function that tries to reconnect to the server.
void IDLNetworking::mqttConnect() {
  if (WiFi.status() != WL_CONNECTED) {
    // TODO connectToWiFi();
  }
  Serial.print("Attempting MQTT connection...");

  // Attempt to connect
  if (PSClient.connect(deviceId)) {
    Serial.println("MQTT connected");
    PSClient.subscribe(otaTopic);

    // announce the toppic on serial for debug
    char announcement[128];
    sprintf(announcement, "Publishing on MQTT toppic: %s:%s/%s", MQTTServer,
            MQTTPort, mqtt_out_toppic);
    Serial.println(announcement);
    sprintf(announcement, "Subscribing on MQTT toppic: %s", otaTopic);
    Serial.println(announcement);

  } else {
    Serial.print("failed, rc=");
    Serial.print(PSClient.state());
    Serial.println(" try again in the next loop");
  }
}

void IDLNetworking::loop() {
  if (!PSClient.connected()) {
    mqttConnect();
  }
  PSClient.loop();

  if (WiFi.status() != WL_CONNECTED) {
    wifiPortal();
  }
}

void IDLNetworking::reset() {
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  SPIFFS.format();
  Serial.println("flash and wifiManager is reset. halting execution");
  while (true) {
    sleep(10000);
  }
}

// send mqtt
void IDLNetworking::sendMeasurement(float value, char *unit, int precision) {
  char buff[128];
  sprintf(buff, "{\"value\": %f,\"unit\":\"%s\",\"precision\":\"%d\" }", value,
          unit, precision);
  PSClient.publish(mqtt_out_toppic, buff);
}

void IDLNetworking::sendEvent(char *type, char *msg, char *payload) {
  char buff[128];
  sprintf(buff, "{\"type\":\"%s\",\"msg\":\"%s\",\"payload\":\"%s\" }", type,
          msg, payload);
  PSClient.publish(mqtt_out_toppic, buff);
}

void IDLNetworking::sendMultiMeasurement(char *values, ) {
  char buff[128];
  sprintf(buff, "{\"type\":\"%s\",\"msg\":\"%s\",\"payload\":\"%s\" }", type,
          msg, payload);
  PSClient.publish(mqtt_out_toppic, buff);
}