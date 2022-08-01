 /*
IAQ_display 
Author: Nachiket Dalvi


*/


#include <Wire.h>
#include <SHT2x.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <EEPROM.h>
#include <ILI9341_Fast.h>
#include "RREFont.h"
#include "rre_arialb_16.h"
#include <AltSoftSerial.h>



#define SCR_WD   240 //setting the display resolution
#define SCR_HT   320 
#define TFT_CS 10  //pins for display 
#define TFT_DC  5
#define TFT_RST 6


#define STARTING_VALUE 350
#define CO2_O 7


AltSoftSerial debug;
ILI9341 lcd = ILI9341(TFT_DC, TFT_RST, TFT_CS);
RREFont font;


struct value{
  float pm10,pm25;
  unsigned long int co2_result;
}current, previous;



unsigned long int co2_duration;
unsigned long int timer = 0;


// needed for RREFont library initialization, define your fillRect
void customRect(int x, int y, int w, int h, int c) 
{
 return lcd.fillRect(x, y, w, h, c); 
}




 


void setup() 
{
  debug.begin(9600);
  Serial.begin(9600); 
  Wire.begin();
  
  pinMode(CO2_O,INPUT);

  previous.co2_result = STARTING_VALUE;
  lcd.init();
  lcd.fillScreen(BLACK);
  font.init(customRect, SCR_WD, SCR_HT); // custom fillRect function and screen width and height values
  font.setFont(&rre_arialb_16);
  font.setColor(WHITE);
  lcd.setRotation(1);
  delay(2000);

  lcd.fillScreen();
  
  
  font.printStr(4,10,"CO2 and Dust Levels");
}


int dust(float *p25, float *p10)
{
    byte buffer;
    int value;
    int len = 0;
    int pm10_serial = 0;
    int pm25_serial = 0;
    int checksum_is;
    int checksum_ok = 0;
    int error = 1;

    while ((debug.available() > 0) || (debug.available() >= (10-len))) 
    {
        buffer = debug.read();
        value = int(buffer);

        switch (len) 
        {
            case (0): if (value != 170) { len = -1; }; break;
            case (1): if (value != 192) { len = -1; }; break;
            case (2): pm25_serial = value; checksum_is = value; break;
            case (3): pm25_serial += (value << 8); checksum_is += value; break;
            case (4): pm10_serial = value; checksum_is += value; break;
            case (5): pm10_serial += (value << 8); checksum_is += value; break;
            case (6): checksum_is += value; break;
            case (7): checksum_is += value; break;
            case (8): if (value == (checksum_is % 256)) { checksum_ok = 1; } else { len = -1; }; break;
            case (9): if (value != 171) { len = -1; }; break;
        }
        len++;
        if (len == 10 && checksum_ok == 1) 
        {
            *p10 = (float)pm10_serial/10.0;
            *p25 = (float)pm25_serial/10.0;
            len = 0; checksum_ok = 0; pm10_serial = 0.0; pm25_serial = 0.0; checksum_is = 0;
            error = 0;
        }
        yield();
    }
    return error;
}


void co2()
{
    co2_duration = pulseIn(CO2_O,HIGH);
    current.co2_result = 3*((co2_duration/1000)-2);

    if(current.co2_result >= 5000)//upper limit of co2
    {
       current.co2_result = previous.co2_result;
    }
    current.co2_result = round(current.co2_result*10)/10;
    while(current.co2_result == STARTING_VALUE)
    {
        co2();
       
    }
    
}



void display()

{
  char temp[5],hum[5],tvoc[16], co[12], p10[6], p25[6];
  int e;
  String s,c;
  c = String(current.co2_result);
  c.toCharArray(co,12);
  
  dtostrf(current.pm25,4,0,p25);
  dtostrf(current.pm10,4,0,p10);  
  lcd.setRotation(1);
  font.setColor(WHITE);
  font.printStr(0,40,"CO2 (ppm)");
  font.printStr(0,60,co);
  debug.print(F("C02 "));debug.print(co);

///////////////////////////////////////////
  
  font.printStr(0,100,"PM2.5 (ug/m3)");
  font.printStr(0,120,p25);
  debug.print(F("PM25 "));debug.print(p25);

//////////////////////////////////////////
  

  
  font.printStr(0,160,"PM10 (ug/m3)");
  font.printStr(0,180,p10);
  debug.print(F("PM10 "));debug.print(p10);

}



void sensor_poll()
{
  int e;
  e = dust(&current.pm25, &current.pm10);
  co2();
 
}

void loop() 
{
  sensor_poll();
 // calibration();
  display();

  //delay(5000);

  delay(120000);
  customRect(0 ,40,320,320,BLACK);
    //delay(100);
}  





