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

## BOM
Raspberry Pi
Raspberry Pi display
4G GSM dongle
esp32 devices and sensors

## MQTT namespaces and Datatypes
Two high-level namespaces exist at the moment, `idl` and `idlota`

### /idl topics
Used for incoming events and metrics, `dash` assumes data is metric by default, but if the last part of the topic is `/event`, a special `Event` type is used which is stored a bit different compared to regular time-series data.

Examples of Metric topics and their payloads:
```
idl/1C8781/temperature      {"value": 49.299999, "unit":"*C"}
idl/1C8781/pressure         {"value": 991.413147, "unit":"hPa"}
idl/1C8781/humidity         {"value": 5.511719, "unit":"%RH"}
idl/30E980/event            {"type": "btnGreenShort", "msg": "Green button single pressed"}
idl/C4B3CC/temperature      {"value": 24.450001, "unit":"*C"}
idl/C4B3CC/pressure         {"value": 992.548035, "unit":"hPa"}
idl/C4B3CC/humidity         {"value": 31.458008, "unit":"%RH"}
```

### /idlota topics
These topics are used to announce firmware updates to running devices. There is no payload and publishes to this topic makes sure interested devices check the OTA service for updates.

## Legal
Copyright 2020 Aalborg University
MIT License
