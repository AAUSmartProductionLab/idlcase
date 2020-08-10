#ifndef _IDLNETWORKING_H_
#define _IDLNETWORKING_H_
#define IDL_JSON_SIZE 2048

/***************************************************************************/
//  default libraries
// #include "Arduino.h"
// #include "WiFi.h"

/*****************************************************************************/
// filesystem management
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson
#include <SPIFFS.h>

/*****************************************************************************/
// include mqtt pubsub
#include <PubSubClient.h>

/*****************************************************************************/
// needed for the wifimanager with captive portal.
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

/***************************************************************************/
// firmware over the air.
#include <esp32fota.h>

class IDLNetworking {
  public:
    // constructor destructor
    IDLNetworking(const char *_deviceType, int _version);
    //~IDLNetworking();

    void loop(int interval = 1000);

    void reset();

    void begin();

    void setDefaults();

    JsonObject pushEvent(char *table, char *message, char *payload);
    JsonObject pushMeasurement(char *table, char *name, char *unit, float value);
    void addTag(JsonObject msgObj, char *name, char *value);
    void addTag(JsonObject msgObj, char *name, int value);

    void sendMeasurements();
    void sendEvents();
    void sendAll();

    char *getDeviceId() { return deviceId; }

    char *getVersionString() { return versionString; }


  private:
    // A constant string depicting the device type this is
    const char *deviceType;
    
    // A Device id which defaults to the last digits of the MAC Address 
    char deviceId[24]; 

    // variable to save last publish time
    unsigned long lastPublish = 0;

    bool usingDefaults = false;

    // wifiManager portal and flash read/write functions.
    void wifiPortal(int timeout = 300); /* default 5 minutes*/
    void readFileSystem();
    void writeFileSystem();

    // WiFi client to handle the MQTT traffic
    WiFiClient wifiClient;

    // MQTT client to send receive messages.
    PubSubClient PSClient = PubSubClient(wifiClient);
    // MQTT connection information
    char MQTTServer[40];
    char MQTTPort[6] = "1883";
  
    // MQTT connection service function
    void mqttConnect();
    // Callback for MQTT service.
    void PSCallback(char *toppic, byte *payload, unsigned int length);
    bool shouldSaveConfig;
    void saveConfigCallback() { shouldSaveConfig = true; }

    // Every time a new firmware is released, existing esp devices
    // will check this type and version number to see if they need updating
    esp32FOTA fota;
    int version;
    // MQTT toppic to listen for updates
    char otaTopic[35];
    // Convinient string form for the version. Intended for future ota upgrades.
    char versionString[16] = "";
    // Where to check for updates
    char otaServer[50];
    // Function to check for updates.
    void tryOTA();

    DynamicJsonDocument *jsonMeasurements;
    DynamicJsonDocument *jsonEvents;

};

#endif /* _IDLNETWORKING_H_ */