#include "IDLNetworking.h"


IDLNetworking::IDLNetworking(const char *deviceType){
    deviceType = deviceType;
    sprintf(deviceId, "%06X", (uint)(ESP.getEfuseMac() >> 24));
    sprintf(mqtt_out_toppic, "idl/%s/event", deviceId);
    sprintf(versionString, "Version: %d", VERSION);

    readFileSystem();
    wifiPortal();
    writeFileSystem();

}


void IDLNetworking::readFileSystem(){
    //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(MQTTServer,       json["MQTTServer"]);
          strcpy(MQTTPort,         json["MQTTPort"]);
        
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
 

  Serial.print("MQTTServer - ");
  Serial.print(MQTTServer);
  Serial.print(":");
  Serial.println(MQTTPort);
}


void IDLNetworking::writeFileSystem(){

    //save the custom parameters to FS
    if (shouldSaveConfig) {
        Serial.println("saving config");
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.createObject();
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
        //end save
    }
}


bool IDLNetworking::wifiPortal(int timeout){
    WiFiManagerParameter custom_text1("<hr/><p style=\"margin-bottom:0em; margin-top:1em;\"><b>MQTT settings</b></p>");
    WiFiManagerParameter customMQTTServer("server", "mqtt server", MQTTServer, 40);
    WiFiManagerParameter customMQTTPort("port", "mqtt port", MQTTPort, 5);
    WiFiManagerParameter custom_text2("<hr/><p style=\"margin-bottom:0em; margin-top:1em;\"><b>Firmware settings</b></p>");
    WiFiManagerParameter customFwServer("server", "Firmware Server", fwServer, 128);

    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    
    // init save flag to false 
    shouldSaveConfig = false;
    //set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);


    //add all your parameters here
    wifiManager.addParameter(&custom_text1);
    wifiManager.addParameter(&customMQTTServer);
    wifiManager.addParameter(&customMQTTPort);
    wifiManager.addParameter(&custom_text2);
    wifiManager.addParameter(&customFwServer);

    /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
    //reset settings - for testing
    //  wifiManager.resetSettings();
    /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

    //set minimum quality of signal so it ignores AP's under that quality
    //defaults to 8%
    wifiManager.setMinimumSignalQuality(8);

    //fetches ssid and pass and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    
    wifiManager.setTimeout(timeout);
    wifiManager.startConfigPortal("CONFIGURE ME - " + *deviceId);

    if (shouldSaveConfig){
        //read updated parameters. Might not have changed but it should be safe to update them again. 
        strcpy(MQTTServer, customMQTTServer.getValue());
        strcpy(MQTTPort, customMQTTPort.getValue());
        strcpy(fwServer, customFwServer.getValue());

        Serial.println("local ip");
        Serial.println(WiFi.localIP());
        Serial.println(WiFi.gatewayIP());
        Serial.println(WiFi.subnetMask());
    }


}

/***************************************************************************/
// MQTT function that tries to reconnect to the server.
void IDLNetworking::mqttConnect() {
    if(WiFi.status() != WL_CONNECTED) {
        //TODO connectToWiFi();
    }
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (PSClient.connect(deviceId)) {
        Serial.println("MQTT connected");
        PSClient.subscribe(otaTopic);

        // announce the toppic on serial for debug
        char announcement[128];
        sprintf(announcement,"Publishing on MQTT toppic: %s:%s/%s\0", MQTTServer, MQTTPort, mqtt_out_toppic);
        Serial.println(announcement);
        sprintf(announcement, "Subscribing on MQTT toppic: %s", otaTopic);
        Serial.println(announcement);

    } else {
        Serial.print("failed, rc=");
        Serial.print(PSClient.state());
        Serial.println(" try again in the next loop");
    }
}

void IDLNetworking::loop(){

}