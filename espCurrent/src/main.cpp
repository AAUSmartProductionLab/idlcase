#include <ArduinoJson.h>

#include "version.h"

/***************************************************************************/
// oled screen
#include "SSD1306Wire.h"
SSD1306Wire display(0x3c, 5, 4);

/***************************************************************************/
// Industrial Data Logger networking implementation.
#include <IDLNetworking.h>
IDLNetworking idl = IDLNetworking("espCurrent", VERSION);

/***************************************************************************/
// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3
#include "EmonLib.h" // Include Emon Library
EnergyMonitor emon1; // Create an instance

/*=========================================================================*/

// oled display loop
void displayLoop(){
  // Displays device information on the oled dispaly.
  display.clear();
  if (WiFi.status() != WL_CONNECTED){
    display.setFont(ArialMT_Plain_24);
    display.drawString(0, 0, idl.getDeviceId());
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 25, "Connecting...");
    display.drawString(0, 36, idl.getVersionString());
  }
  else{
    display.setFont(ArialMT_Plain_24);
    display.drawString(0, 0, idl.getDeviceId());
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 25, WiFi.localIP().toString());
    display.drawString(0, 36, idl.getVersionString());
  }

  display.display();
}

void setup(){
  Serial.begin(115200);

  idl.begin();

  emon1.current(32, 6.06); // Current: input pin, calibration. Amps at 1v ac input

  // Initialising the UI will init the display too.
  display.init();
  displayLoop();

  Serial.println("Setup done");
}

void loop(){
  idl.loop(1000);

  double Irms = emon1.calcIrms(5000); // Calculate Irms only

  Serial.print(Irms * 230.0); // Apparent power
  Serial.print(" ");
  Serial.println(Irms); // Irms

  idl.pushMeasurement("current", "sensor1", "A", Irms);
  idl.pushMeasurement("power", "sensor1", "W", Irms * 230.0);
  idl.sendMeasurements();

}