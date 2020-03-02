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

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

IDLNetworking idlNetworking = IDLNetworking("rfidReader");
/*=========================================================================*/
// RFID reader instance
MFRC522 mfrc522(CS_PIN,RST_PIN);


/*=========================================================================*/
// OLED screen instance 
SSD1306Wire display(0x3c, 5, 4);


/**************************************************************************/
// display loop

void displayLoop() {
    display.clear();
    if (WiFi.status() != WL_CONNECTED) {
      display.setFont(ArialMT_Plain_24);
      display.drawString(0, 20, "Connecting...");
    } else {
      display.setFont(ArialMT_Plain_24);
      display.drawString(0, 0, "deviceId");
      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 25, WiFi.localIP().toString());
    }

    display.display();
}


void setup(){
    Serial.begin(115200);
 
    //idlNetworking.reset();

    SPI.begin();
    mfrc522.PCD_Init();


    // pin setups
    pinMode(LED_PIN,OUTPUT);
    pinMode(PIEZO_PIN,OUTPUT);
    pinMode(BTN_PIN,INPUT);
    pinMode(IRQ_PIN,INPUT);

    ledcSetup(0,5000,8);
    ledcAttachPin(PIEZO_PIN,0);

    // Initialising the UI will init the display too.
    display.init();

    displayLoop(); 

    idlNetworking.begin();


}

void array_to_string(byte array[], unsigned int len, char buffer[])
{
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

void loop() {

  idlNetworking.loop();

  displayLoop();

  if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
    delay(50);
    return;
  }

  // does not work on esp32  :(
  //tone(PIEZO_PIN,880,100);

  // Dump UID
  char buff[30];
  array_to_string(mfrc522.uid.uidByte, mfrc522.uid.size, buff);

  idlNetworking.sendEvent("rfidRead","hand held scan", buff);

  Serial.print(F("Card UID:"));
  Serial.print(buff);

  delay(500);

  
}
