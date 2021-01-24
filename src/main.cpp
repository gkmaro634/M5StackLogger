#include <Arduino.h>
#include <M5Core2.h>
#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_BMP280.h>
#include "SHT3X.h"

SHT3X sht30;
Adafruit_BMP280 bme;

float pressure = 0.0;
float temp = 0.0;
float humid = 0.0;

void setup() {
  // put your setup code here, to run once:
  M5.begin();
  Wire.begin();

  M5.Lcd.setTextSize(1);
  M5.Lcd.println("Hello World!");

  while(!bme.begin(0x76)){
    M5.Lcd.println("BMP280 init fail");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  float pressure = bme.readPressure();
  if(sht30.get() == 0){
    temp = sht30.cTemp;
    humid = sht30.humidity;
  }
  M5.Lcd.printf("%.1f[deg], %.1f[%%], %.1f [Pa]\r\n",temp, humid, pressure);

  delay(1000);
}