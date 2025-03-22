         /////////////////////////////////////////////  
        //    Bluetooth-Enabled Snowman Weather    //
       //         and Air Quality Gift Card       //
      //             ---------------             //
     //             (Arduino Nano)              //           
    //             by Kutluhan Aktar           // 
   //                                         //
  /////////////////////////////////////////////

//
// Via the gift card's Android application, adjust its RGB eye color and display weather and air quality information on the ST7789.
//
// For more information:
// https://www.theamplituhedron.com/projects/Bluetooth-enabled-Snowman-Weather-and-Air-Quality-Gift-Card/
//
//
// Connections
// Arduino Nano :           
//                                HC-06 Bluetooth Module
// D7  --------------------------- TX
// D8  --------------------------- RX
// 5V  --------------------------- 5V
// GND --------------------------- GND
//                                ST7789 240x240 IPS
// GND --------------------------- GND
// 3.3V -------------------------- VCC
// D13 --------------------------- SCL
// D11 --------------------------- SDA
// D9  --------------------------- RES
// D10 --------------------------- DC
//                                BMP180 Barometric Pressure/Temperature/Altitude Sensor
// A4  --------------------------- SDA
// A5  --------------------------- SCL
//                                DHT11 Humidity and Temperature Sensor
// D2  --------------------------- S
//                                MQ-135 Air Quality Sensor
// A3  --------------------------- S
//                                RGB LED
// D3  --------------------------- R
// D5  --------------------------- G
// D6  --------------------------- B

/*
 ST7789 240x240 IPS (without CS pin) connections (only 6 wires required):

 #01 GND -> GND
 #02 VCC -> VCC (3.3V only!)
 #03 SCL -> D13/SCK
 #04 SDA -> D11/MOSI
 #05 RES -> D8 or any digital
 #06 DC  -> D7 or any digital
 #07 BLK -> NC
*/

// Include the required libraries.
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Arduino_ST7789_Fast.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "DHT.h"

// Initiate the bluetooth module. Connect the defined RX pin (7) to the TX pin on the bluetooth module.
SoftwareSerial Gift_Card(7, 8); // RX, TX

// Define the ST7789 240x240 IPS display settings.
#define TFT_DC    10
#define TFT_RST   9
#define SCR_WD   240
#define SCR_HT   240

// Include the converted images:
#include "temp.c"
#include "humd.c"
#include "pre.c"
#include "airq.c"

// Initiate the ST7789 240x240 IPS display.
Arduino_ST7789 tft = Arduino_ST7789(TFT_DC, TFT_RST); 

// Define the BMP180 sensor settings.
Adafruit_BMP085 bmp;
double temperature, _altitude;
int pressure, pressure_sea;

// Define the DHT11 object.
DHT dht;
float humidity, temperature_DHT;

// Define the MQ-135 pin.
#define mq135 A3
int air_quality;

// Define RGB pins.
#define redPin 3
#define greenPin 5
#define bluePin 6

// Define interface options:
volatile boolean home, tem, hum, pres, air, ani;
 
void setup() {
  Serial.begin(9600);

  // Activate the bluetooth module.
  Gift_Card.begin(9600);

  // You can change the default settings of the HC-06 Module by uncommenting the function below - Name: Gift Card, Password: 1234, Baud Rate: 9600.
  // changeBluetoothSettings();

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);  

  // Open and clear the ST7789 240x240 IPS display.
  tft.init(SCR_WD, SCR_HT);
  tft.fillScreen(BLACK);
  
  // Check whether the BMP180 module is working.
  if (!bmp.begin()){ Serial.println("Could not find a valid BMP085 sensor, check wiring!"); while (1) {} }

  // Initiate the DHT11 Module.
  dht.setup(2);

  // Homescreen
  tft.fillScreen(RGBto565(248, 178, 41));
  tft.setCursor(0, 40);
  tft.setTextColor(RGBto565(22, 21, 118));
  tft.setTextSize(6);
  tft.println("Happy");
  tft.println("New");
  tft.println("Year:)");

}

void loop() {
  // Detect commands transferred by the Android Application (Gift Card).
  Application_Commands();
  
  // Execute the requested command:
  if(home == true){
    tft.fillScreen(RGBto565(248, 178, 41));
    while(home == true){
      Application_Commands();
      // Homescreen
      tft.setCursor(0, 40);
      tft.setTextColor(RGBto565(22, 21, 118));
      tft.setTextSize(6);
      tft.println("Happy");
      tft.println("New");
      tft.println("Year:)");
    }
  }

  if(tem == true){
    tft.fillScreen(BLACK);
    while(tem == true){
      Application_Commands();
      // Collect data from the sensors - BMP180, DHT11, and MQ-135.
      collect_Data();
      // Print:
      tft.setCursor(54, 0);
      tft.setTextColor(RGBto565(255,69,0), BLACK);
      tft.setTextSize(2);
      tft.print(F("Temperature"));
      tft.fillCircle(120, 80, 40, WHITE);
      tft.drawImageF(104, 64, 32, 32, temp);
      tft.setCursor(40, 145);
      tft.setTextSize(4);
      tft.print((String)temperature + "*C");
      tft.setCursor(40, 185);
      tft.print((String)temperature_DHT + "*F");
    }
  }

  if(hum == true){
    tft.fillScreen(BLACK);
    while(hum == true){
      Application_Commands();
      // Collect data from the sensors - BMP180, DHT11, and MQ-135.
      collect_Data();
      // Print:
      tft.setCursor(70, 0);
      tft.setTextColor(BLUE, BLACK);
      tft.setTextSize(2);
      tft.print(F("Humidity"));
      tft.fillCircle(120, 80, 40, WHITE);
      tft.drawImageF(104, 64, 32, 32, humd);
      tft.setCursor(50, 160);
      tft.setTextSize(4);
      tft.println((String)humidity + "%");
    }
  }
  
  if(pres == true){
    tft.fillScreen(BLACK);
    while(pres == true){
      Application_Commands();
      // Collect data from the sensors - BMP180, DHT11, and MQ-135.
      collect_Data();
      // Print:
      tft.setCursor(0, 0);
      tft.setTextColor(RGBto565(154,205,50), BLACK);
      tft.setTextSize(2);
      tft.print(F("Pressure / Altitude"));
      tft.fillCircle(120, 80, 40, WHITE);
      tft.drawImageF(104, 64, 32, 32, pre);
      tft.setCursor(35, 145);
      tft.setTextSize(4);
      tft.println((String)-pressure + " Pa");
      tft.setCursor(35, 185);
      tft.println((String)_altitude + " m");
    }
  }

  if(air == true){
    tft.fillScreen(BLACK);
    while(air == true){
      Application_Commands();
      // Collect data from the sensors - BMP180, DHT11, and MQ-135.
      collect_Data();
      // Print:
      tft.setCursor(54, 0);
      tft.setTextColor(RGBto565(243,208,296), BLACK);
      tft.setTextSize(2);
      tft.print(F("Air Quality"));
      tft.fillCircle(120, 80, 40, WHITE);
      tft.drawImageF(104, 64, 32, 32, airq);
      tft.setCursor(25, 160);
      tft.setTextSize(4);
      String air_q;
      if(air_quality < 10){air_q = "0" + String(air_quality);}else{air_q = String(air_quality);}
      tft.println(air_q + " of 50");
    }
  }
  
  if(ani == true){
    tft.fillScreen(BLACK);
    while(ani == true){
      // Initiate animation design:
      Animation(10, RGBto565(238,119,98));
      Animation(15, RGBto565(243,208,296));
      Animation(20, RGBto565(174,255,205));
      Animation(25, WHITE);
    }
  }
  
}

void Application_Commands(){
  // If the HC-06 Bluetooth module is receiving commands from the Android application:
  if(Gift_Card.available()){
    char c = Gift_Card.read();
    // Execute the requested command:
    switch(c){
      case 'h':
        // Home Screen
        home = true;
        tem = false;
        hum = false;
        pres = false;
        air = false;
        ani = false; 
      break;
      case '1':
        // Temperature
        home = false;
        tem = true;
        hum = false;
        pres = false;
        air = false;
        ani = false;         
      break;
      case '2':
        // Humidity
        home = false;
        tem = false;
        hum = true;
        pres = false;
        air = false;
        ani = false; 
      break;
      case '3':
        // Pressure and Altitude
        home = false;
        tem = false;
        hum = false;
        pres = true;
        air = false;
        ani = false;       
      break;  
      case '4':
        // Air Quality
        home = false;
        tem = false;
        hum = false;
        pres = false;
        air = true;
        ani = false; 
      break;
      case '5':
        // Animation
        home = false;
        tem = false;
        hum = false;
        pres = false;
        air = false;
        ani = true; 
      break;  
      case 'r':
        adjustColor(255, 0, 0);
      break; 
      case 'g':
        adjustColor(0, 255, 0);
      break;
      case 'b':
        adjustColor(0, 0, 255);
      break; 
      case 'y':
        adjustColor(255, 255, 0);
      break; 
      case 'p':
        adjustColor(255, 0, 255);
      break; 
      case 'c':
        adjustColor(0, 255, 255);
      break; 
      case 'w':
        adjustColor(255, 255, 255);
      break;  
      case 'o':
        adjustColor(0, 0, 0);
      break;                          
    }
  }
}

void collect_Data(){
  // Get variables generated by the BMP180 module.
  temperature = bmp.readTemperature();
  pressure = bmp.readPressure();
  pressure_sea = bmp.readSealevelPressure();
  // Calculate altitude assuming 'standard' barometric
  // pressure of 1013.25 millibar = 101325 Pascal
  _altitude = bmp.readAltitude();

  // Get variables generated by the DHT11 module.
  delay(dht.getMinimumSamplingPeriod());
  humidity = dht.getHumidity();
  temperature_DHT = dht.toFahrenheit(dht.getTemperature());
  
  // Get variables generated by the MQ-135 sensor.
  air_quality = map(analogRead(mq135), 0, 850, 0, 50);
}

void adjustColor(int r, int g, int b){
  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);
}

void Animation(uint8_t radius, uint16_t color) {
  int x, y, w = tft.width(), h = tft.height(), r2 = radius * 2;
  tft.fillScreen(BLACK);
  for(x=radius; x<w; x+=r2) {
    for(y=radius; y<h; y+=r2) {
      tft.fillCircle(x, y, radius, color);
      delay(100);
      // Halt animation if requested.
      Application_Commands();
    }
    delay(100);
  }
}

void changeBluetoothSettings(){
  // Define the new settings.
  String Name = "Gift Card";
  String Password = "1234";
  String Uart = "9600,0,0";

  Gift_Card.print("AT+NAME"); // Change the name.
  Gift_Card.println(Name); 
  Serial.print("Name is changed: ");
  Serial.println(Name);
  delay(2000);
  Gift_Card.print("AT+PIN"); // Change the password.
  Gift_Card.println(Password);
  Serial.print("Password is changed: ");
  Serial.println(Password);
  delay(2000);
  Gift_Card.print("AT+UART"); // Change the baud rate. If the bluetooth module is a HC-05, the default value of baud rate is 38400.
  Gift_Card.println(Uart);
  Serial.print("Baud rate is set: ");
  Serial.println(Uart);
  delay(2000);
  Serial.println("Task Completed!"); // You can see in the terminal whether the task is completed correctly or not.
  

}
     
