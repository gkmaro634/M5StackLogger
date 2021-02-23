#include <Arduino.h>
#include <M5Core2.h>
#include <WiFi.h>
#include <math.h>
#include "Ambient.h"
#include "secret.h"

#define AMBIENT_POST_INTERVAL_SECONDS 300

WiFiClient client;
Ambient ambient;

hw_timer_t * timer0 = NULL;
portMUX_TYPE timer0Mutex = portMUX_INITIALIZER_UNLOCKED;
volatile bool isTimer0Ticked = false;
const uint16_t timer0Prescaler = 80;// 80MHz / 80 = 1MHz
const int esp32SystemClock = 80 * 1e6;
const int timer0TickIntervalMilliSeconds = 10 * 1000;
uint64_t timer0InterruptTick =  (double)esp32SystemClock / (double)timer0Prescaler * (double)timer0TickIntervalMilliSeconds / 1000.0;

const int mhz19cPin = 33; 
unsigned int mhz19cOutputHighDuration = 0;
float cppm = 0.0;

void IRAM_ATTR onTimer0Ticked(){
	portENTER_CRITICAL_ISR(&timer0Mutex);
	isTimer0Ticked = true;
	portEXIT_CRITICAL_ISR(&timer0Mutex);
}

void setup()
{
	// put your setup code here, to run once:
	M5.begin(115200);
	M5.Lcd.setTextSize(3);
	M5.Lcd.setBrightness(64);
	disableCore0WDT();
	disableCore1WDT();
	// M5.Lcd.sleep();
	// M5.Lcd.setBrightness(0);
	// Serial.println("Hello World!");

	pinMode(mhz19cPin, INPUT);
	timer0 = timerBegin(0, timer0Prescaler, true);// 1us?
	timerAttachInterrupt(timer0, &onTimer0Ticked, true);
	timerAlarmWrite(timer0, timer0InterruptTick, true);

	WiFi.begin(ssid, pass);
	while( WiFi.status() != WL_CONNECTED){
		delay(500);
		M5.Lcd.print(".");
    	// Serial.println(".");
	}
	M5.Lcd.println("WiFi connected");
	// Serial.println("WiFi connected");

	ambient.begin(ambientChid, writeKey, &client);
	delay(1000);
	M5.Lcd.clear();

	timerAlarmEnable(timer0);
}

bool isSetValue = false;

void loop()
{
	// TODO: ダストセンサの平均かとAmbientのポストの周期を独立させる
	portENTER_CRITICAL_ISR(&timer0Mutex);
	if(isTimer0Ticked == true){
		isSetValue = true;
		isTimer0Ticked = false;
	}
	portEXIT_CRITICAL_ISR(&timer0Mutex);

	if(isSetValue == true){
		// post ambient
		ambient.set(1, cppm);
		ambient.send();

		isSetValue = false;
	}

	while(true){
		if(digitalRead(mhz19cPin) != LOW){
			break;
		}
	}
	mhz19cOutputHighDuration = pulseIn(mhz19cPin, HIGH, 3000 * 1000);// TODO: other thread
	if(mhz19cOutputHighDuration != 0){
		float th = (float)mhz19cOutputHighDuration / 1000.0;
		cppm = 2.0 * (th - 2.0);
		M5.Lcd.setCursor(0, 0);
		M5.Lcd.printf("CO2\r\n");
		M5.Lcd.printf("%5.0f [ppm]\r\n", cppm);
	}

	delay(1);
}


