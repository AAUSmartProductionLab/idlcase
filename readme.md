## Overview

This repository contains everything related to the Industrial data logger suitcase, including Raspberry Pi configuration in `ansible/`, firmware for actual devices (`espBME280/`, `espButton` ... ) and `dash/` which glues a lot of stuff together.

## Concept drawing
![picture](docs/conceptdiagram.png)

Data flows from sensors, though MQTT where `dash` listens for events which it then passes on to the GUI of the Raspberry Pi and stores it in the database.
 
Users access a Grafana frontend where they will be able to browse time series data directly from the database.
All the above software runs locally on the Raspberry Pi

## Connectivity
Raspberry Pi(s) continuously tries to join a VPN with our cloud server - the cloud server acts as a reverse proxy and passes HTTP requests for configured domains directly to the Raspberry Pis inside the VPN. Traffic not inside the VPN and also not directly connected to the Pi's wifi network will be rejected.

## Firmware over-the-air updates
The OTA scheme implemented tries to solve two scenarios
* A developer sits in platformio and wants to update all BME280 sensor devices
* A case is pulled out - some BME280 sensors are updated, some are not - those that are not should be able to update themselves without anyone touching the IDE

To solve these scenarios, the `dash/` projects host simple JSON files describing the latest version of a given firmware type - When a device cold boots, it will try to compare its release to the one fetched from `dash`.

When a developer wants to upload new firmware, `idlversion` and `idluploader` helps figuring out what version number to use next, when running "Upload" it will upload the firmware to the IDL case and publish a message on MQTT which devices listen to. Devices then to the regular OTA update just as if hey where cold booted.

## WifiManager
Our Library IDLNetworking makes use of [tzapu's wifimanager](https://github.com/tzapu/WiFiManager/tree/development). If the ESP device cannot connect to a known wifi it will start the wifimanager. The device will create a hotspot named "CONFIGURE ME - 12AB34" with the ending being the device ID. When connecting to this hotspot all traffic will be routed to a configuration portal that can be accecced with any browser. Connecting from an Android device this portal pops up by itself via a notification. Is that not the case the portal can be accessed on ip address `192.168.1.4`. 
From the portal it is possible to select any nearby access point and type in the password for it. As well as selecting access point it is also possible to set the MQTT server and port to publish on. Finally you can set the server where the ESP-device will check for firmware updates.
Writing `true` into the field named "use defaults below" will set MQTT and firmware server to the ip of gateway.
The protal is kept open for 5 minutes before the device tries to connect to known wifi again.  
Known bugs: wifimanager opens the configuration portal on every other reset. See issue [#1067](https://github.com/tzapu/WiFiManager/issues/1067) for updates. 


## BOM
Raspberry Pi
Raspberry Pi display
4G GSM dongle
esp32 devices and sensors

## MQTT namespaces and Datatypes
Two high-level namespaces exist at the moment, `idl` and `idlota`

### /idl/<deviceID>/<messageType>
Used for incoming events and metrics, `dash` assumes data is metric by default. `<deviceID>`, is used to tag the data and `<messageType` is dictating the format of the json payload. Two message-type formats exists, `events` and `measurements`. Common for both formats is the massages are in an array with an object for each measurement and event. Be aware that the formats does not mix. One array must contain only one format type chosen by the MQTT toppic

**Examples of Metric topics and their payloads:**
```
/idl/C4B3CC/measurements
[
    {       
        "table": "temperature",     // influxdb tabel
        "name": "sensor 1",         // influxdb tag
        "unit": "celcius",          // influxdb tag
        "value": 23.34,             // influxdb value
        "tags":{                    // optional extra tags
            "placement" : "kitchen"
        }
    },
]
```
**Event message**
```
/idl/1C8781/events
[
    {       
        "table": "events",                      // influxdb tabel
        "msg": "someone pushed the red button", // human readable 
        "payload": "{blob}",                    // whatever one sees fit
        "tags": {                               // optional extra tags
            "color":"red",
            "priority": "1",
        }
    },
]
```
**Multiple measurements**
```
/idl/1C8781/measurements
[
    {       
        "table": "temperature",
        "name": "BME280",
        "unit": "celsius",
        "value": 20.5,
    },
    {       
        "table": "microphone",
        "name": "left channel",
        "unit": "dBm",
        "value": 42.47,
        "tags": { 
            "band":"100hz",
        }
    },
    {       
        "table": "microphone",
        "name": "left channel",
        "unit": "dBm",
        "value": 51.25,
        "tags": { 
            "band":"200hz",
        }
    },
    {       
        "table": "microphone",
        "name": "right channel",
        "unit": "dBm",
        "value": 42.87,
        "tags": { 
            "band":"100hz",
        }
    },
    {       
        "table": "microphone",
        "name": "right channel",
        "unit": "dBm",
        "value": 50.15,
        "tags": { 
            "band":"200hz",
        }
    },
]
```


### /idlota topics
These topics are used to announce firmware updates to running devices. There is no payload and publishes to this topic makes sure interested devices check the OTA service for updates.

## Legal
Copyright 2020 Aalborg University
MIT License
