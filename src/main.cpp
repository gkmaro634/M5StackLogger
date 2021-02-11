#include <Arduino.h>
#include <M5Core2.h>
#include <WiFi.h>
#include "Ambient.h"
#include "secret.h"

#define BUFFER_SIZE 11
#define SAMPLING_INTERVAL_MILLISECONDS 1000
#define AMBIENT_POST_INTERVAL_SECONDS 300

// const char* ssid = "****" <= secret.h
// const char* pass = "****" <= secret.h
// int ambientChid = "****" <= secret.h
// const char* writeKey = "****" <= secret.h

// float pressure = 0.0;
// float temp = 0.0;
// float humid = 0.0;
// float battery = 0.0;
// float pressureBuffer[BUFFER_SIZE];
// float tempBuffer[BUFFER_SIZE];
// float humidBuffer[BUFFER_SIZE];
int samplingCounter = 0;
int bufferCounter = 0;
int ppd42_output = 0;

WiFiClient client;
// Ambient ambient;

volatile int timer_counter = 0;
hw_timer_t * timer = NULL;
portMUX_TYPE timer_mux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer0Tick(){
	portENTER_CRITICAL_ISR(&timer_mux);
	timer_counter++;
	portEXIT_CRITICAL_ISR(&timer_mux);
}

void setup()
{
	// put your setup code here, to run once:
	M5.begin(115200);
	M5.Lcd.setTextSize(1);
	M5.Lcd.setBrightness(64);
	M5.Lcd.println("Hello World!");
	// Serial.println("Hello World!");

	pinMode(33, INPUT);
	timer = timerBegin(0, 80, true);// 1us?
	timerAttachInterrupt(timer, &onTimer0Tick, true);
	timerAlarmWrite(timer, 1000000, true);
	timerAlarmEnable(timer);

	WiFi.begin(ssid, pass);
	while( WiFi.status() != WL_CONNECTED){
		delay(500);
		M5.Lcd.print(".");
    	// Serial.println(".");
	}
	M5.Lcd.println("WiFi connected");
	// Serial.println("WiFi connected");

	// ambient.begin(ambientChid, writeKey, &client);
}

void loop()
{
	portENTER_CRITICAL_ISR(&timer_mux);
	if(timer_counter > 1){
		// Serial.printf("timer ticked\r\n");
		M5.Lcd.printf("timer ticked\r\n");
		timer_counter = 0;
	}
	portEXIT_CRITICAL_ISR(&timer_mux);

	if(samplingCounter > AMBIENT_POST_INTERVAL_SECONDS){
		//// calc values to post ambient

		//// set and send values ambient
		// ambient.set(1, avePressure / 100.0);
		// ambient.set(2, aveTemp);
		// ambient.set(3, aveHumid);
		// ambient.set(4, battery);
		// ambient.send();

		//// after process send ambient
		// samplingCounter = 0;
		// M5.Lcd.clear();
		// M5.Lcd.printf("Power: %.1f [V], \r\n", battery);
		// M5.Lcd.printf("%.1f[deg], %.1f[%%], %.1f [hPa]\r\n", aveTemp, aveHumid, avePressure / 100.0);
	}

	// put your main code here, to run repeatedly:
    // // read device output
	// float pressure = bme.readPressure();
	// if (sht30.get() == 0)
	// {
	// 	temp = sht30.cTemp;
	// 	humid = sht30.humidity;
	// }

	// // enqueue buffer
	// pressureBuffer[bufferCounter] = pressure;
	// tempBuffer[bufferCounter] = temp;
	// humidBuffer[bufferCounter] = humid;
	// bufferCounter++;
	// if(bufferCounter >= BUFFER_SIZE){
	// 	bufferCounter = 0;
	// }

	// ppd42_output = digitalRead(33);
	// Serial.printf("ppd42: %d\r\n", ppd42_output);

	delay(SAMPLING_INTERVAL_MILLISECONDS);
	samplingCounter++;
}


