# RFID scanner

Handheld device to read mifare-classic RFID tags. The device consists of the following elements:
  * ESP32
  * MFRC522 rfid reader
  * Button
  * Indicator LED
  * Oled screen
  * Piezo speaker

### How to use
Hold the device close to an rfid tag until a beep is heard and the indicator led lights up. The rfid UID is briefly displayed on the oled screen. Press the button to display the UID of the last red tag or press and hold the button when scanning a new tag do display the tag for the duration you hold the button. The UID will be send as an event via MQTT.

### Wifi manager
The firmware has a wifimanager that creates a hotspot that when connecting to it you can configure the device via the captive portal. The portal will start automatically if the configured access point is not available.  
In order to force the hotspot and captive portal to start, press and hold the button when turning on the device. The device will beep when the portal opens. 

In order to access the portal connect to the created hotspot named 'CONFIGURE ME - 12AB34'  with the ending being the device ID. When connecting to this hotspot all traffic will be routed to a configuration portal that can be accecced with any browser. Connecting from an Android device this portal pops up by itself via a notification. Is that not the case the portal can be accessed on ip address `192.168.1.4`. 

### MQTT data format
**Examples of topics and the payload**
```
/idl/1C8781/events
[
    {       
        "table": "events",                             // influxdb tabel
        "msg": "handheld rfid scan of id:A4 B5 C6",    // human readable version
        "payload": "A4 B5 C6",                         // UID of rfid tag
    },
]
```