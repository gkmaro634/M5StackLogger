#include <Arduino.h>
#include <M5Core2.h>
#include <Wire.h>
#include <WiFi.h>

void setup() {
  // put your setup code here, to run once:
  M5.begin();
  M5.Lcd.println("Setupped");
}

void loop() {
  // put your main code here, to run repeatedly:
  M5.Lcd.println("Hello World!");
  delay(1000);
}