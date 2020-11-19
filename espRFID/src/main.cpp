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
#define LED_PIN 2    
#define PIEZO_PIN 13 
#define IRQ_PIN 22   
#define BTN_PIN 26    
#define RST_PIN 17    
#define SCLK_PIN 18   
#define MISO_PIN 19 
#define MOSI_PIN 23  
#define CS_PIN 0 
#define PIEZO_FREQ 2000

/***************************************************************************/
//  default libraries 
#include <SPI.h>
#include <Wire.h>

/***************************************************************************/
// RFID reader MFRC522
#include <MFRC522.h>

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
MFRC522 mfrc522(CS_PIN,RST_PIN);
char myTag[30] = "NO TAG";


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

void displayTag(int showTime = 1000){
  // Dislay the rfid tag on the screen for some time
  display.clear();
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 0, myTag);
  display.display();
  delay(showTime);

}

void array_to_string(byte array[], unsigned int len, char buffer[]){
  // Takes an array of bytes and prints them out as hex in a string
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*3+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*3+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
        buffer[i*3+2] = ' ';
    }
    buffer[len*2] = '\0';
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
    pinMode(IRQ_PIN,INPUT); // not in use but may be needed by the mfrc522 lib

    ledcSetup(0,PIEZO_FREQ,8);
    ledcAttachPin(PIEZO_PIN,0);

    // Initialising the UI will init the display too.
    display.init();

    // update the display with device id and status info.
    displayLoop(); 

    // Begin the idl instance. This connects to wifi and takes care of 
    // mqtt and messages and basically all networking.
    idl.begin();

    // Begin the rfid instance. 
    SPI.begin();
    mfrc522.PCD_Init();

    // Begin button instance. 
    // Set function callback when the button is pressed. I made use of a
    // lambda function to pass in the time it should display the tag
    // information on the screen. 
    button.begin();
    button.onPressed( []{displayTag(1500);});
    
    // if the button is pressed upon boot it opens the wifi portal
    // regardless of connection status. 
    button.read();
    if (button.isPressed()){
      openWifiPortal();
    }
}

void loop() {

  idl.loop(0);

  displayLoop();

  button.read();

  if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
    delay(50);
    return;
  }

  // only here if card is pressent. 


  // Dump UID
  array_to_string(mfrc522.uid.uidByte, mfrc522.uid.size, myTag);


  char buff2[50];
  sprintf(buff2,"handheld rfid scan of id:%s",myTag);

  idl.pushEvent("events",buff2, myTag);
  idl.sendEvents();
  Serial.print(F("Card UID:"));
  Serial.print(myTag);

  // piezo buzzer on
  ledcWrite(0,50);
  // led on
  digitalWrite(LED_PIN,HIGH);
  // display on screen
  displayTag(0); 
  // led and piezo time
  delay(300);
  // piezo buzzer off
  ledcWrite(0,0);

  while(button.isPressed()){
    // keey the display on
    delay(10);
    button.read();
  }

  // led off
  digitalWrite(LED_PIN,LOW);
  
}
