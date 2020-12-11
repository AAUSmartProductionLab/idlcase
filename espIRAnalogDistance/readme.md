# Analog Infrared Distance Sensor

Handheld device to measure distance to an object. The device consists of the following elements:
  * ESP32
  * Sharp IR sensor  10-80 cm
  * Button
  * Indicator LED
  * Oled screen
  * Piezo speaker

### How to use
Press and hold the button to activate reading mode. When the device is active the indicator led will be on. The device will make a measurement every 250ms and publish a message over MQTT

### Wifi manager
The firmware has a wifimanager that creates a hotspot that when connecting to it you can configure the device via the captive portal. The portal will start automatically if the configured access point is not available.  
In order to force the hotspot and captive portal to start, press and hold the button when turning on the device. The device will beep when the portal opens. 

In order to access the portal connect to the created hotspot named 'CONFIGURE ME - 12AB34'  with the ending being the device ID. When connecting to this hotspot all traffic will be routed to a configuration portal that can be accecced with any browser. Connecting from an Android device this portal pops up by itself via a notification. Is that not the case the portal can be accessed on ip address `192.168.1.4`. 

### MQTT data format
Example of topic and the payload
```
/idl/C4B3CC/measurements
[
    {       
        "table": "distance",        // influxdb table
        "name": "ir_analog",        // influxdb tag
        "unit": "cm",               // influxdb tag
        "value": 23,                // influxdb value
    },
]
```