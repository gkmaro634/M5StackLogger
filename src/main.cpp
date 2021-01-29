#include <Arduino.h>
#include <M5Core2.h>
#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_BMP280.h>
#include "SHT3X.h"
#include "Ambient.h"
#include "secret.h"

#define BUFFER_SIZE 11
#define SAMPLING_INTERVAL_MILLISECONDS 1000
#define AMBIENT_POST_INTERVAL_SECONDS 300

// const char* ssid = "****" <= secret.h
// const char* pass = "****" <= secret.h
// int ambientChid = "****" <= secret.h
// const char* writeKey = "****" <= secret.h

SHT3X sht30;
Adafruit_BMP280 bme;

float pressure = 0.0;
float temp = 0.0;
float humid = 0.0;
float battery = 0.0;
float pressureBuffer[BUFFER_SIZE];
float tempBuffer[BUFFER_SIZE];
float humidBuffer[BUFFER_SIZE];
int samplingCounter = 0;
int bufferCounter = 0;

WiFiClient client;
Ambient ambient;

void setup()
{
	// put your setup code here, to run once:
	M5.begin();
	Wire.begin();
	M5.Lcd.setTextSize(1);
	M5.Lcd.setBrightness(64);
	M5.Lcd.println("Hello World!");

	while (!bme.begin(0x76))
	{
		M5.Lcd.println("BMP280 init fail");
	}

	WiFi.begin(ssid, pass);
	while( WiFi.status() != WL_CONNECTED){
		delay(500);
		M5.Lcd.print(".");
	}
	M5.Lcd.println("WiFi connected");

	ambient.begin(ambientChid, writeKey, &client);
}

void loop()
{
	if(samplingCounter > AMBIENT_POST_INTERVAL_SECONDS){
		battery = M5.Axp.GetBatVoltage();
		float avePressure = 0;
		float aveTemp = 0;
		float aveHumid = 0;
		for(int i = 0; i < BUFFER_SIZE; i++){
			avePressure += pressureBuffer[i];
			aveTemp += tempBuffer[i];
			aveHumid += humidBuffer[i];
		}
		avePressure /= BUFFER_SIZE;
		aveTemp /= BUFFER_SIZE;
		aveHumid /= BUFFER_SIZE;

		ambient.set(1, avePressure / 100.0);
		ambient.set(2, aveTemp);
		ambient.set(3, aveHumid);
		ambient.set(4, battery);
		ambient.send();

		samplingCounter = 0;
		M5.Lcd.clear();
		M5.Lcd.printf("Power: %.1f [V], \r\n", battery);
		M5.Lcd.printf("%.1f[deg], %.1f[%%], %.1f [hPa]\r\n", aveTemp, aveHumid, avePressure / 100.0);
	}

	// put your main code here, to run repeatedly:
	float pressure = bme.readPressure();
	if (sht30.get() == 0)
	{
		temp = sht30.cTemp;
		humid = sht30.humidity;
	}

	pressureBuffer[bufferCounter] = pressure;
	tempBuffer[bufferCounter] = temp;
	humidBuffer[bufferCounter] = humid;
	bufferCounter++;
	if(bufferCounter >= BUFFER_SIZE){
		bufferCounter = 0;
	}

	delay(SAMPLING_INTERVAL_MILLISECONDS);
	samplingCounter++;
}