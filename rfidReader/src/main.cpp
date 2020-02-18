/**************************************************************************
*
 *          :::::::::  :::::::::: ::::::::::: ::::::::: 
 *         :+:    :+: :+:            :+:     :+:    :+: 
 *        +:+    +:+ +:+            +:+     +:+    +:+  
 *       +#++:++#:  :#::+::#       +#+     +#+    +:+   
 *      +#+    +#+ +#+            +#+     +#+    +#+    
 *     #+#    #+# #+#            #+#     #+#    #+#     
 *    ###    ### ###        ########### #########       
*
****************************************************************************/
const char DeviceType[] = "rfidReader";

#define LED_PIN 2    
#define PIEZO_PIN 13 
#define IRQ_PIN 22   
#define BTN_PIN 26    
#define RST_PIN 17    
#define SCLK_PIN 18   
#define MISO_PIN 19 
#define MOSI_PIN 23  
#define CS_PIN 0 

/***************************************************************************/
//  default libraries 
#include "Arduino.h"
#include "WiFi.h"
#include <SPI.h>
#include <Wire.h>

/***************************************************************************/
// RFID reader MFRC522
#include <MFRC522.h>

/***************************************************************************/
// oled screen
#include "SSD1306Wire.h"        

/***************************************************************************/
// include mqtt pubsub
#include <PubSubClient.h>

/***************************************************************************/
// firmware over the air.
#include <esp32fota.h>
#include "version.h"

/***************************************************************************/
//needed for the wifimanager with captive portal. 
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

/***************************************************************************/
// filesystem management 
#include <SPIFFS.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/


/*=========================================================================*/
// Firmware over the air 

// where to listen for updates
const char* otaTopic = "idlota/" + *DeviceType;

// where to check for updates
const char* otaMeta = "http://10.13.37.1/db/" + *DeviceType; 

// every time a new firmware is released, existing esp devices
// will check this type and version number to see if they need updating
esp32FOTA fota(DeviceType, VERSION);

char fwServer[128];
char versionString[16];
char deviceId[24];

/*=========================================================================*/
// RFID reader instance
MFRC522 mfrc522(CS_PIN,RST_PIN);

/*=========================================================================*/
// create a wifi client to handle the mqtt traffic
WiFiClient espClient;
PubSubClient PSClient(espClient);
char MQTTServer[40];
char MQTTPort[6] = "1883";
char mqtt_out_toppic [64];
char mqtt_in_toppic [64];

/*=========================================================================*/
// OLED screen instance 
SSD1306Wire display(0x3c, 5, 4);


/***************************************************************************/
//callback notifying us of the need to save the wifi manager config
bool shouldSaveConfig = false;
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

/**************************************************************************/
// display loop

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

/**************************************************************************/
// check the web server if there's a new update. This is done at boot and 
// whenever a new firmware is announced over MQTT
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

/***************************************************************************/
// MQTT function that tries to reconnect to the server.
void reconnect() {
    if(WiFi.status() != WL_CONNECTED) {
        //TODO connectToWiFi();
    }
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (PSClient.connect(deviceId)) {
      Serial.println("MQTT connected");
      PSClient.subscribe(otaTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(PSClient.state());
      Serial.println(" try again in the next loop");
    }
}

/***************************************************************************/
void readFileSystem(){
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


void saveFileSystem(){

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


void wifiPortal(bool boot){
  WiFiManagerParameter custom_text1("<hr/><p style=\"margin-bottom:0em; margin-top:1em;\"><b>MQTT settings</b></p>");
  WiFiManagerParameter customMQTTServer("server", "mqtt server", MQTTServer, 40);
  WiFiManagerParameter customMQTTPort("port", "mqtt port", MQTTPort, 5);
  WiFiManagerParameter custom_text2("<hr/><p style=\"margin-bottom:0em; margin-top:1em;\"><b>Firmware settings</b></p>");
  WiFiManagerParameter customFwServer("server", "Firmware Server", "0.0.0.0", 128);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
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
  if (boot){
    wifiManager.autoConnect("CONFIGURE ME - " + *deviceId);
  }
  else{
    wifiManager.setTimeout(30);
    wifiManager.startConfigPortal("CONFIGURE ME - SMART RFID");
  }
  // if here we are now connected 
  //read updated parameters
  strcpy(MQTTServer, customMQTTServer.getValue());
  strcpy(MQTTPort, customMQTTPort.getValue());
  strcpy(fwServer, customFwServer.getValue());

}

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

void setup(){
    Serial.begin(19200);
    sprintf(deviceId, "%06X", (uint)(ESP.getEfuseMac() >> 24));
    sprintf(mqtt_out_toppic, "idl/%s/event", deviceId);
    sprintf(versionString, "Version: %d", VERSION);

    sprintf(deviceId, "%06X", (uint)(ESP.getEfuseMac() >> 24));
    // create outtoppic using the names from the saved parameters. 
    sprintf(mqtt_out_toppic,"idl/%s/event", deviceId);  

    // announce the toppic on serial for debug
    char anouncement[128];
    sprintf(anouncement,"Publishing on MQTT toppic: %s:%s/%s\0", MQTTServer, MQTTPort, mqtt_out_toppic);
    Serial.println(anouncement);

    SPI.begin();
    mfrc522.PCD_Init();


    // pin setups
    pinMode(LED_PIN,OUTPUT);
    pinMode(PIEZO_PIN,OUTPUT);
    pinMode(BTN_PIN,INPUT);
    pinMode(IRQ_PIN,INPUT);

    ledcSetup(0,5000,8);
    ledcAttachPin(PIEZO_PIN,0);

    readFileSystem();

    wifiPortal(true);

    saveFileSystem();

    Serial.println("local ip");
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.gatewayIP());
    Serial.println(WiFi.subnetMask());


    // Initialising the UI will init the display too.
    display.init();

    displayLoop(); 

    PSClient.setServer(MQTTServer, String(MQTTPort).toInt());
    PSClient.setCallback(callback);


}


void loop() {
  if (!PSClient.connected()) {
    reconnect();
  }

  PSClient.loop();

  displayLoop();

    delay(500);
}
