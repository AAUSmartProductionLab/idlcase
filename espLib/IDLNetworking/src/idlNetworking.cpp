#include "IDLNetworking.h"

/*===========================================================================*/
// Constructor. initializing the fota object 
IDLNetworking::IDLNetworking(const char *_deviceType, int _version) 
    : fota(esp32FOTA(String(_deviceType), _version))
{
    
    deviceType = _deviceType;
    version = _version;
  
    // initialize more values. 
    sprintf(deviceId, "%06X", (uint)(ESP.getEfuseMac() >> 24));
    sprintf(versionString, "Version: %d", version);
}

/*===========================================================================*/
// Initialize the IDLNetworking object 
void IDLNetworking::begin() {
    // First Read the filesystem to get saved values.
    readFileSystem();

    wifiPortal();
    writeFileSystem();
    
    tryOTA();
    
    PSClient.setServer(MQTTServer, String(MQTTPort).toInt());
    PSClient.setCallback(
        [this](char *topic, byte *payload, unsigned int length) {
            this->PSCallback(topic, payload, length);
        });
}

/*===========================================================================*/
// Loop function to update the IDLNetworking library
void IDLNetworking::loop(int interval) {
    unsigned long now = millis();
    if (now > lastPublish + interval){
        if (interval != 0) Serial.println("WARNING: pubish interval exceeded. Too much data to push within the given time.");
    }
    while (millis() < lastPublish + interval){
        // do nothing
    }

    lastPublish = millis();

    if (!PSClient.connected()) {
        mqttConnect();
    }
    PSClient.loop();

    if (WiFi.status() != WL_CONNECTED) {
        wifiPortal(1,true);
        delay(1000);
    }
}


/*===========================================================================*/
// Reset clears all stored information on the flash.
// This clears WiFi SSID, firmware servers etc. 
void IDLNetworking::reset() {
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    SPIFFS.format();
    Serial.println("flash and wifiManager is reset and default settings are set. now halting execution 10 sec.");
    while (true) {
        sleep(10000);
    }
}



/*===========================================================================*/
// check the web server if there's a new update. This is done at boot and
// whenever a new firmware is announced over MQTT
void IDLNetworking::tryOTA() {
    fota.checkURL = otaServer;

    bool updateNeeded = fota.execHTTPcheck();
    if (updateNeeded) {
        fota.execOTA();
    }
}

/*===========================================================================*/
// Callback funktion for whenever the MQTT client receives a message. 
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

/*===========================================================================*/
void IDLNetworking::readFileSystem() {
    // read configuration from FS json
    Serial.println("reading FS...");

    if (!SPIFFS.begin()){        
        Serial.println("failed to mount FS. Formatting it and trying again");
        SPIFFS.format();
        SPIFFS.begin();
    } 
    if (!SPIFFS.exists("/config.json")){
        Serial.println("/config.json file does not yet exist. Add some date via the WiFi portal. Until then default values are used.");
        strcpy(usingDefaults , "true");
        return ;        
    }
    // file exists, reading and loading
    File configFile = SPIFFS.open("/config.json", "r");
    if(! configFile) {
        Serial.println("Failed to open /config.json - Using default values");
        strcpy(usingDefaults , "true");
        return ;
    }

    // Allocate a buffer to store contents of the file.
    size_t size = configFile.size();
    std::unique_ptr<char[]> buf(new char[size]);

    // Read the bytes and deserialize them into jsonformat.
    configFile.readBytes(buf.get(), size);
    configFile.close();

    DynamicJsonDocument jsonDoc(2048) ;
    auto error = deserializeJson(jsonDoc,buf.get());
    if (error) {
        Serial.println("failed to serialise json config. Using Default values...");
        strcpy(usingDefaults , "true");
        return;
    }   

    // print to terminal
    serializeJsonPretty(jsonDoc, Serial);

    // copy the json values into variables.
    strcpy(MQTTServer, jsonDoc["MQTTServer"]);
    strcpy(MQTTPort, jsonDoc["MQTTPort"]);
    strcpy(otaServer, jsonDoc["otaServer"]);
    strcpy(otaTopic, jsonDoc["otaTopic"]);


    Serial.print("MQTTServer - ");
    Serial.print(MQTTServer);
    Serial.print(":");
    Serial.println(MQTTPort);
    Serial.print("otaServer - ");
    Serial.print(otaServer);
    Serial.print( " - toppic : ");
    Serial.println(otaTopic);
}

/*===========================================================================*/
void IDLNetworking::setDefaults(){
    if(!WiFi.isConnected()) 
        return;

    strcpy(MQTTServer, WiFi.gatewayIP().toString().c_str());
    strcpy(MQTTPort, "1883");
    sprintf(otaServer, "http://%s/db/%s", WiFi.gatewayIP().toString().c_str() ,deviceType);
    sprintf(otaTopic, "idlota/%s", deviceType);
}

/*===========================================================================*/
void IDLNetworking::writeFileSystem() {
    Serial.println("Writing config save file to FS");
    if (!SPIFFS.begin()){        
        Serial.println("failed to mount FS. formatting and trying again.");
        SPIFFS.format();
        SPIFFS.begin();
    } 
    File configFile = SPIFFS.open("/config.json", FILE_WRITE);
    if(! configFile) {
        Serial.println("Failed to open /config.json for writing");
        return ;
    }

    DynamicJsonDocument jsonDoc(2048);

    jsonDoc["MQTTServer"] = MQTTServer;
    jsonDoc["MQTTPort"] = MQTTPort;
    jsonDoc["otaServer"] = otaServer;
    jsonDoc["otaTopic"] = otaTopic;

    serializeJsonPretty(jsonDoc, Serial);
    serializeJson(jsonDoc, configFile);


    configFile.close();
    // end save
}

/*===========================================================================*/
void IDLNetworking::wifiPortal(int timeout, bool autoConnect) {
    WiFiManagerParameter custom_text0("<hr/><p style=\"margin-bottom:0em; margin-top:1em;\"><b>Use defaults</b></p>");
    WiFiManagerParameter customUseingDefaults("useingDefaults", "use defaults below", usingDefaults, 6);
    WiFiManagerParameter custom_text1("<hr/><p style=\"margin-bottom:0em; margin-top:1em;\"><b>MQTT settings</b></p>");
    WiFiManagerParameter customMQTTServer("MQTTServer", "mqtt server", MQTTServer, 40);
    WiFiManagerParameter customMQTTPort("port", "mqtt port", MQTTPort, 5);
    WiFiManagerParameter custom_text2("<hr/><p style=\"margin-bottom:0em; margin-top:1em;\"><b>Firmware settings</b></p>");
    WiFiManagerParameter customOtaServer("otaServer", "Firmware Server", otaServer, 50);
    WiFiManagerParameter customOtaTopic("otaTopic", "Firmware Annonce Topic", otaTopic, 35);
    // WiFiManager
    // Local intialization. Once its business is done, there is no need to keep
    // it around
    WiFiManager wifiManager;

    // set config save notify callback
    wifiManager.setSaveConfigCallback([this] { this->saveConfigCallback(); });

    // add all your parameters here
    wifiManager.addParameter(&custom_text0);
    wifiManager.addParameter(&customUseingDefaults);
    wifiManager.addParameter(&custom_text1);
    wifiManager.addParameter(&customMQTTServer);
    wifiManager.addParameter(&customMQTTPort);
    wifiManager.addParameter(&custom_text2);
    wifiManager.addParameter(&customOtaServer);
    wifiManager.addParameter(&customOtaTopic);

    // set minimum quality of signal so it ignores AP's under that quality
    // defaults to 8%
    wifiManager.setMinimumSignalQuality(8);

    // fetches ssid and pass and tries to connect
    // if it does not connect it starts an access point with the specified name
    // here  "AutoConnectAP"
    // and goes into a blocking loop awaiting configuration

    wifiManager.setTimeout(timeout);
    char buf[30];
    sprintf(buf, "CONFIGURE ME - %s", deviceId);
    delay(200);
    if (autoConnect) {
        wifiManager.autoConnect(buf);
    }else{
        wifiManager.startConfigPortal(buf);
    }

    // read updated parameters. Might not have changed but it should be safe
    // to update them again.
    strcpy(MQTTServer, customMQTTServer.getValue());
    strcpy(MQTTPort, customMQTTPort.getValue());
    strcpy(otaServer, customOtaServer.getValue());
    strcpy(otaTopic, customOtaTopic.getValue());
    strcpy(usingDefaults, customUseingDefaults.getValue());

    if ( strcmp(usingDefaults, "true") == 0){  // evaluate to 0 if there is no difference between the strings
        Serial.println("using default values for MQTT and OTA server");
        setDefaults();
    }    

    Serial.println("local ip");
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.gatewayIP());
    Serial.println(WiFi.subnetMask());

    writeFileSystem();
}

/*===========================================================================*/
// MQTT function that tries to reconnect to the server.
void IDLNetworking::mqttConnect() {
    if (WiFi.status() != WL_CONNECTED) {
        // TODO connectToWiFi();
    }
    char announcement[128];
    sprintf(announcement,"Attempting MQTT connection to server: %s...", MQTTServer);
    Serial.print(announcement);

    // Attempt to connect
    if (PSClient.connect(deviceId)) {
        Serial.println("MQTT connected");
        PSClient.subscribe(otaTopic);

        // announce the subscribe toppic on serial for debug
        sprintf(announcement, "Subscribing on MQTT topic: %s \nPublishing on MQTT topic: idl/%s/<messageType>", otaTopic, deviceId);
        Serial.println(announcement);

    } else {
        Serial.print("failed, rc=");
        Serial.print(PSClient.state());
        Serial.println("Sleep a second and try again in the next loop");
        delay(1000);
    }
}


/*===========================================================================*/
JsonObject IDLNetworking::pushEvent(char *table, char *msg, char *payload){
    if(! jsonEvents){
        jsonEvents = new DynamicJsonDocument(IDL_JSON_SIZE);
    }

    JsonObject obj = jsonEvents->createNestedObject();
    obj["table"] = table;
    obj["msg"] = msg;
    obj["payload"] = payload;

    return obj;

}

/*===========================================================================*/
JsonObject IDLNetworking::pushMeasurement(char *table, char *name, char *unit, float value){
    if(! jsonMeasurements){
        jsonMeasurements = new DynamicJsonDocument(IDL_JSON_SIZE);
    }

    JsonObject obj = jsonMeasurements->createNestedObject();
    obj["table"] = table;
    obj["name"] = name;
    obj["unit"] = unit;
    obj["value"] = value;

    return obj;
}

/*===========================================================================*/
void IDLNetworking::addTag(JsonObject msgObj, char *name, char *value){
    if(!msgObj.containsKey("tags")){
        msgObj.createNestedObject("tags");
    }
    msgObj["tags"][name] = value;
}
void IDLNetworking::addTag(JsonObject msgObj, char *name, int value){
    if(!msgObj.containsKey("tags")){
        msgObj.createNestedObject("tags");
    }
    msgObj["tags"][name] = value;
}

/*===========================================================================*/
void IDLNetworking::sendMeasurements(){
    if(!jsonMeasurements){return;}

    // debug print to servial. This is expensive prosessing wise. 
    //serializeJsonPretty(*jsonMeasurements, Serial);

    // send measurements
    char buff[43];
    sprintf(buff,"idl/%s/measurements",deviceId);

    PSClient.beginPublish(buff,measureJson(*jsonMeasurements),false);
    serializeJson(*jsonMeasurements,PSClient);
    PSClient.endPublish();
    
    // delete and forget
    delete jsonMeasurements;
    jsonMeasurements = nullptr;
}

/*===========================================================================*/
void IDLNetworking::sendEvents(){
    if(!jsonEvents){return;}

    // debug print to servial. This is expensive prosessing wise. 
    //serializeJsonPretty(*jsonEvents, Serial);
    
    // send events
    char buff[25];
    sprintf(buff,"idl/%s/events",deviceId);
    PSClient.beginPublish(buff,measureJson(*jsonEvents),false);
    serializeJson(*jsonEvents,PSClient);
    PSClient.endPublish();
    
    // delete and forget
    delete jsonEvents;
    jsonEvents = nullptr;
}
/*===========================================================================*/
void IDLNetworking::sendAll() {
    sendMeasurements();
    sendEvents();
}




/*===========================================================================*/
// void IDLNetworking::sendRaw(char *kind, JsonObject &json) {
//     sprintf(mqtt_out_toppic, "idlcase/%s/%s", kind, deviceId);
//     size_t length = json.measureLength();
//     PSClient.beginPublish(mqtt_out_toppic, length, false);
//     json.printTo(PSClient);
//     PSClient.endPublish();
// }

/*===========================================================================*/



