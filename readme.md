## Overview

This repository contains everything related to the Indrustrial data logger suitcase, including Raspberry Pi configuration in `ansible/`, firmware for actural devices (`espBME280/`, `espButton` ... ) and `dash/` which glues all the stuff togeather .

Concept drawing
#### Insert drawing

Data flows from sensors, though MQTT where `dash` listens for events which it then passes on to the GUI of the Raspberry Pi and stores it in the database.
 
Users access a Grafana frontend where they will be able to browse time series data directly from the database.
All the above software runs locally on the Raspberry Pi

## BOM
Raspberry Pi
Raspberry Pi display
4G GSM dongle
esp32 devices

