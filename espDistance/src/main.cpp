/**************************************************************************
*   
 *         ::::::::::: :::::::::              :::     ::::    :::     :::     :::        ::::::::   ::::::::    
 *            :+:     :+:    :+:           :+: :+:   :+:+:   :+:   :+: :+:   :+:       :+:    :+: :+:    :+:    
 *           +:+     +:+    +:+          +:+   +:+  :+:+:+  +:+  +:+   +:+  +:+       +:+    +:+ +:+            
 *          +#+     +#++:++#:          +#++:++#++: +#+ +:+ +#+ +#++:++#++: +#+       +#+    +:+ :#:             
 *         +#+     +#+    +#+         +#+     +#+ +#+  +#+#+# +#+     +#+ +#+       +#+    +#+ +#+   +#+#       
 *        #+#     #+#    #+#         #+#     #+# #+#   #+#+# #+#     #+# #+#       #+#    #+# #+#    #+#        
 *   ########### ###    ###         ###     ### ###    #### ###     ### ########## ########   ########          
 *         ::::::::: ::::::::::: :::::::: ::::::::::: :::     ::::    :::  ::::::::  ::::::::::                 
 *        :+:    :+:    :+:    :+:    :+:    :+:   :+: :+:   :+:+:   :+: :+:    :+: :+:                         
 *       +:+    +:+    +:+    +:+           +:+  +:+   +:+  :+:+:+  +:+ +:+        +:+                          
 *      +#+    +:+    +#+    +#++:++#++    +#+ +#++:++#++: +#+ +:+ +#+ +#+        +#++:++#                      
 *     +#+    +#+    +#+           +#+    +#+ +#+     +#+ +#+  +#+#+# +#+        +#+                            
 *    #+#    #+#    #+#    #+#    #+#    #+# #+#     #+# #+#   #+#+# #+#    #+# #+#                             
 *   ######### ########### ########     ### ###     ### ###    ####  ########  ##########        
*
****************************************************************************/
#define LED_PIN 2    
#define PIEZO_PIN 13  
#define BTN_PIN 26    
#define IR_ANALOG_PIN 34
#define PIEZO_FREQ 2000

/***************************************************************************/
//  default libraries 
#include <SPI.h>
#include <Wire.h>

/***************************************************************************/
// Distance sensor 
#include <ESP32SharpIR.h>

/***************************************************************************/
// oled screen
#include "SSD1306Wire.h"        

/***************************************************************************/
// Industrial Data Logger networking implementation.
#include <IDLNetworking.h>
#include "version.h"

/***************************************************************************/
#include <EasyButton.h>

/***************************************************************************/
/***************************************************************************/

IDLNetworking idl = IDLNetworking("espRFID", VERSION);
/*=========================================================================*/
// RFID reader instance
ESP32SharpIR sharp(ESP32SharpIR::GP2Y0A21YK0F, IR_ANALOG_PIN);

/*=========================================================================*/
// easy button instance
EasyButton button(BTN_PIN,40,false,false);


/*=========================================================================*/
// OLED screen instance 
SSD1306Wire display(0x3c, 5, 4);


/**************************************************************************/
// display loop
void displayLoop() {
  // Displays device information on the oled dispaly. 
    display.clear();
    if (WiFi.status() != WL_CONNECTED) {
        display.setFont(ArialMT_Plain_24);
        display.drawString(0, 0, idl.getDeviceId());
        display.setFont(ArialMT_Plain_10);
        display.drawString(0, 25, "Connecting...");
        display.drawString(0, 36, idl.getVersionString());
    } else {
        display.setFont(ArialMT_Plain_24);
        display.drawString(0, 0, idl.getDeviceId());
        display.setFont(ArialMT_Plain_10);
        display.drawString(0, 25, WiFi.localIP().toString());
        display.drawString(0, 36, idl.getVersionString());
    }

    display.display();
}

void displayDist(int dist){
  // Dislay the distance on the screen
  display.clear();
  display.setFont(ArialMT_Plain_24);
  char buff[10] ;
  sprintf(buff,"%d cm", dist);
  display.drawString(0, 0, String(buff));
  display.display();
}

void openWifiPortal() {
  // Small helper function that buzzez the pieco and opens the wifi portal
    // piezo buzzer set frequenzy -> on -> off
    ledcSetup(0,3500,8);
    ledcWrite(0,50);
    delay(300);
    ledcWrite(0,0);

    idl.openWifiPortal();
    
    // reset piezo buzzer frequency 
    ledcSetup(0,PIEZO_FREQ,8);

}

void setup(){
    Serial.begin(115200);

    // pin setups
    pinMode(LED_PIN,OUTPUT);
    pinMode(PIEZO_PIN,OUTPUT);

    ledcSetup(0,PIEZO_FREQ,8);
    ledcAttachPin(PIEZO_PIN,0);

    // Initialising the UI will init the display too.
    display.init();

    // update the display with device id and status info.
    displayLoop(); 

    // Begin the idl instance. This connects to wifi and takes care of 
    // mqtt and messages and basically all networking.
    idl.begin();

    // Begin button instance. 
    // Set function callback when the button is pressed. I made use of a
    // lambda function to pass in the time it should display the tag
    // information on the screen. 
    button.begin();
    
    // if the button is pressed upon boot it opens the wifi portal
    // regardless of connection status. 
    button.read();
    if (button.isPressed()){
      openWifiPortal();
    }
}

void loop() {

  idl.loop(0);

  button.read();
  if (! button.isPressed()) {
    // display device info
    displayLoop();
    // led off
    digitalWrite(LED_PIN,LOW);
    delay(30);
    return;
  }

  // only here if button is pressed. 
  
  // Led on
  digitalWrite(LED_PIN,HIGH);

  int dist = sharp.getDistance();

  idl.pushMeasurement("distance","ir_analog","cm", dist);
  idl.sendMeasurements();
  
  Serial.print(F("Distance: "));
  Serial.println(dist);

  // display on screen
  displayDist(dist);
  delay(250); 

}
