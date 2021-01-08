#include <ArduinoJson.h>

#include "version.h"

/***************************************************************************/
// oled screen
#include "SSD1306Wire.h"

/***************************************************************************/
// Industrial Data Logger networking implementation.
#include <IDLNetworking.h>

/***************************************************************************/
// OneWire and DS18B20 libraries
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 26
#define TEMPERATURE_PRECISION 11
#define NUMBER_OF_THERMOMETERS 8

/***************************************************************************/
#include <Wire.h>
//#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

/***************************************************************************/



/*=========================================================================*/
/*=========================================================================*/
// analog pins

int pins[] = {32,33,34,35};
int nPins = 4;
int analogActivatePin = 23; // pull low to activate analog pins.
int hasAnalogPins = false;

/*=========================================================================*/
// Onewire and DS18B20 sensor instance
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);

int deviceCount = 0;

// Variable to hold temporary device addresses
DeviceAddress thermoAdr;

// Variable to hold temporary temperature values 
float tempC; 

/*=========================================================================*/
// lux sensor instance
TwoWire I2CLight = TwoWire(1);
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
bool hasLuxSensor = false ;

/*=========================================================================*/
// IDLNetworking instance

IDLNetworking idl = IDLNetworking("espCustom1", VERSION);

/*=========================================================================*/
// OLED screen instance
SSD1306Wire display(0x3c, 5, 4);

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


/*=========================================================================*/
// printing a thermometer address as string
void printAddress(DeviceAddress deviceAddress)
{ 
  for (uint8_t i = 0; i < 8; i++)
  {
    Serial.print("0x");
    if (deviceAddress[i] < 0x10) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    if (i < 7) Serial.print(", ");
  }
  Serial.println("");
}

/*=========================================================================*/
// Char array printed as string of hex
void array_to_string(byte array[], unsigned int len, char buffer[])
{
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}


/*=========================================================================*/
// struct to hold light data
struct lightData{
    uint16_t full;
    uint16_t ir;
    uint16_t visible;
    float lux;
} myLuxData;

/*=========================================================================*/
// dallas temperature sensors and tls light sensor setup
void sensorsSetup() {
    // temperature sensors
    sensors.begin();

    // locate devices on the bus
    Serial.print("Locating devices...");
    Serial.print("Found ");
    deviceCount = sensors.getDeviceCount();
    Serial.print(deviceCount, DEC);
    Serial.println(" devices.");
    Serial.println("");

    Serial.println("Printing addresses...");
    for (int i = 0; i < deviceCount; i++)    {
        Serial.print("Sensor ");
        Serial.print(i + 1);
        Serial.print(" : ");
        sensors.getAddress(thermoAdr, i);
        printAddress(thermoAdr);

        // set the resolution to x bit per device
        sensors.setResolution(thermoAdr, TEMPERATURE_PRECISION);
    }
    
    // lux sensor
    I2CLight.begin(21,22);
    hasLuxSensor = tsl.begin(&I2CLight);
    
    // You can change the gain on the fly, to adapt to brighter/dimmer light situations
    tsl.setGain(TSL2591_GAIN_LOW);      
 
    // Changing the integration time gives you a longer time over which to sense light
    // longer timelines are slower, but are good in very low light situtations!
    tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);

    // Print some useful sensor details 
    sensor_t sensor;
    tsl.getSensor(&sensor);
    Serial.println(F("------------------------------------"));
    Serial.print  (F("Sensor:       ")); Serial.println(sensor.name);
    Serial.print  (F("Driver Ver:   ")); Serial.println(sensor.version);
    Serial.print  (F("Unique ID:    ")); Serial.println(sensor.sensor_id);
    Serial.print  (F("Max Value:    ")); Serial.print(sensor.max_value); Serial.println(F(" lux"));
    Serial.print  (F("Min Value:    ")); Serial.print(sensor.min_value); Serial.println(F(" lux"));
    Serial.print  (F("Resolution:   ")); Serial.print(sensor.resolution, 4); Serial.println(F(" lux"));  
    Serial.println(F(""));
    /* Display the gain and integration time for reference sake */  
    Serial.print  (F("Gain:         "));
    tsl2591Gain_t gain = tsl.getGain();
    switch(gain)
    {
        case TSL2591_GAIN_LOW:
        Serial.println(F("1x (Low)"));
        break;
        case TSL2591_GAIN_MED:
        Serial.println(F("25x (Medium)"));
        break;
        case TSL2591_GAIN_HIGH:
        Serial.println(F("428x (High)"));
        break;
        case TSL2591_GAIN_MAX:
        Serial.println(F("9876x (Max)"));
        break;
    }
    Serial.print  (F("Timing:       "));
    Serial.print((tsl.getTiming() + 1) * 100, DEC); 
    Serial.println(F(" ms"));
    Serial.println(F("------------------------------------"));
    Serial.println(F(""));


}

/*=========================================================================*/
// Read IR and Full Spectrum at once and convert to lux
void readLuxSensor()
{
  // More advanced data read example. Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
  // That way you can do whatever math and comparisons you want!
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  myLuxData.full = full;
  myLuxData.ir = ir;
  myLuxData.visible = full-ir;
  myLuxData.lux = tsl.calculateLux(full,ir);

//   Serial.print(F("[ ")); Serial.print(millis()); Serial.print(F(" ms ] "));
//   Serial.print(F("IR: ")); Serial.print(ir);  Serial.print(F("  "));
//   Serial.print(F("Full: ")); Serial.print(full); Serial.print(F("  "));
//   Serial.print(F("Visible: ")); Serial.print(full - ir); Serial.print(F("  "));
//   Serial.print(F("Lux: ")); Serial.println(tsl.calculateLux(full, ir), 6);

}

/*=========================================================================*/
// esp setup loop
void setup() {
    Serial.begin(115200);
    delay(100);
    
    // read analog activation pin 
    pinMode(analogActivatePin, INPUT_PULLUP);
    delay(500);
    hasAnalogPins = (0 == digitalRead(analogActivatePin)); // true if activation pin is pulled low
    Serial.print("has analog pins = ");
    Serial.println(hasAnalogPins);

    sensorsSetup();

    idl.begin();

    // Initialising the UI will init the display too.
    display.init();
    displayLoop(); 

    Serial.println("Setup done");
}


/*=========================================================================*/
// esp infinite loop
void loop() {
    idl.loop(2000);
    
    // displayLoop();

    // Send command to all the sensors for temperature conversion
    sensors.requestTemperatures();
    
    // Store temperature from each sensor
    for (int i = 0; i < deviceCount; i++){
        tempC = sensors.getTempCByIndex(i);
        sensors.getAddress(thermoAdr, i);

        char hex_string[20] ;
        array_to_string(thermoAdr, 8, hex_string);

        idl.pushMeasurement("temperature",hex_string,"celcius",tempC);
         
    }

    if(hasLuxSensor ){
        // read light sensor 
        readLuxSensor();
        idl.pushMeasurement("light","sensor 1", "raw_full",myLuxData.full);
        idl.pushMeasurement("light","sensor 1", "raw_ir", myLuxData.ir);
        idl.pushMeasurement("light","sensor 1", "lux", myLuxData.lux);
    }

    if(hasAnalogPins){
        char pin_string[8] ;

        for (int i = 0; i < nPins; i++){
            int adcValue = analogRead(pins[i]);
            float flow = ((adcValue * (20-2) ) / (4095)) + 2;  // NewValue = (((OldValue - OldMin) * (NewMax - NewMin)) / (OldMax - OldMin)) + NewMin.

            sprintf(pin_string, "pin:%i", pins[i]);
            idl.pushMeasurement("flow",pin_string, "l/min", flow);
        }
    }

    idl.sendMeasurements();

}