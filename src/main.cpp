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
const int ppd42OutputCalcIntervalSeconds = 30;
const int timer0TickIntervalMilliSeconds = ppd42OutputCalcIntervalSeconds * 1000;
uint64_t timer0InterruptTick =  (double)esp32SystemClock / (double)timer0Prescaler * (double)timer0TickIntervalMilliSeconds / 1000.0;

const int ppd42Pin = 33; 
unsigned int ppd42OutputLowDuration = 0;
double ppd42OutputLowRatio = 0.0;
double pm25Amount = 0.0;

void IRAM_ATTR onTimer0Ticked(){
	portENTER_CRITICAL_ISR(&timer0Mutex);
	isTimer0Ticked = true;
	portEXIT_CRITICAL_ISR(&timer0Mutex);
}

// 単位をμg/m^3に変換
double ratio2ugm3 (double ratio)
{
	double concent = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520.0 * ratio + 0.62;	
	
	// 全粒子密度(1.65E12μg/ m3)
	double density = 1.65 * 1E12;
	// PM2.5粒子の半径(2.5μm)
	double r25 = 2.5 * 1E-6;
	double vol25 = (4.0 / 3.0) * PI * pow (r25, 3);
	double mass25 = density * vol25; // μg
	double K = 3531.5; // per m^3 
	// μg/m^3に変換して返す
	return concent * K * mass25;
}


void setup()
{
	// put your setup code here, to run once:
	M5.begin(115200);
	M5.Lcd.setTextSize(3);
	M5.Lcd.setBrightness(64);
	// M5.Lcd.sleep();
	// M5.Lcd.setBrightness(0);
	// Serial.println("Hello World!");

	pinMode(ppd42Pin, INPUT);
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

	ppd42OutputLowDuration += pulseIn(ppd42Pin, LOW, 100 * 1000);// TODO: 別スレッドにする
	if(isSetValue == true){
		ppd42OutputLowRatio = (double)ppd42OutputLowDuration / (double)(ppd42OutputCalcIntervalSeconds * 1e6);
		ppd42OutputLowDuration = 0;

		double concent = 1.1 * pow(ppd42OutputLowRatio, 3) - 3.8 * pow(ppd42OutputLowRatio, 2) + 520.0 * ppd42OutputLowRatio + 0.62;	
		// pm25Amount = ratio2ugm3(ppd42OutputLowRatio * 100);
		
		// display
		M5.Lcd.setCursor(0, 0);
		M5.Lcd.printf("dust: %.0lf \r\n", concent);

		// post ambient
		ambient.set(1, concent);
		ambient.send();

		isSetValue = false;
	}
}


