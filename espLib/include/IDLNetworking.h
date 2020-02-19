#ifndef _IDLNETWORKING_H_
#define _IDLNETWORKING_H_

/***************************************************************************/
//  default libraries 
#include "Arduino.h"
#include "WiFi.h"

/*****************************************************************************/
// filesystem management 
#include <SPIFFS.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

/*****************************************************************************/
// include mqtt pubsub
#include <PubSubClient.h>

/*****************************************************************************/
//needed for the wifimanager with captive portal. 
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

/***************************************************************************/
// firmware over the air.
#include <esp32fota.h>
#include "version.h"


class IDLNetworking{
private:

    // create a wifi client to handle the mqtt traffic
    WiFiClient espClient;
    PubSubClient PSClient = PubSubClient(espClient);

    // MQTT connection service function
    void mqttConnect();

    bool wifiPortal(int timeout = 300); /* default 5 minutes*/
    void readFileSystem();
    void writeFileSystem();

    

    void PSCallback(char* toppic, byte* payload, unsigned int length);
    bool shouldSaveConfig;
    void saveConfigCallback() {shouldSaveConfig = true; }

    
    
    char deviceId[24];
    char MQTTServer[40];
    char MQTTPort[6] = "1883";
    char mqtt_out_toppic [64];
    char mqtt_in_toppic [64];

    char fwServer[128];
    char versionString[16];
    
    const char *deviceType;

    
        // where to listen for updates
    const char* otaTopic = "idlota/" + *deviceType;

    // where to check for updates
    const char* otaMeta = "http://10.13.37.1/db/" + *deviceType; 

    // every time a new firmware is released, existing esp devices
    // will check this type and version number to see if they need updating
    esp32FOTA fota = esp32FOTA(String(deviceType), VERSION);

    void tryOTA();


public:
    // constructor destructor
    IDLNetworking(const char *deviceType);
    //~IDLNetworking();


    // loop 
    void loop();

    void reset();

};

#endif /* _IDLNETWORKING_H_ */