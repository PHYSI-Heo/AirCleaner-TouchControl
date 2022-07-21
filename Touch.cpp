#include "Touch.h"


void (*touchCallback)(int);

Touch::Touch()
{
	
}

void Touch::init(bool _debug /* = false */)
{
	isDebugging = _debug;
	
	pinMode(TOUCH_1_PIN, INPUT_PULLUP);
	pinMode(TOUCH_2_PIN, INPUT_PULLUP);
	pinMode(TOUCH_3_PIN, INPUT_PULLUP);
	
	if(isDebugging)
	{
		Serial.print(F("[Touch] Pins "));
		Serial.print(TOUCH_1_PIN);
		Serial.print(F(", "));
		Serial.print(TOUCH_2_PIN);
		Serial.print(F(", "));
		Serial.println(TOUCH_3_PIN);
	}	
}

void Touch::setCallback(void (*_touchCallback)(int))
{
	touchCallback = _touchCallback;
}

void Touch::listen()
{
	int tCnt = 0;
	int tNumber = -1;
		
	if(digitalRead(TOUCH_1_PIN))
	{
		while(digitalRead(TOUCH_1_PIN))
		{
			tCnt++;
			delay(TOUCH_DELAY);
		}		
		tNumber = TOUCH_POWER;
	}	
	else if(digitalRead(TOUCH_2_PIN))
	{
		while(digitalRead(TOUCH_2_PIN))
		{
			tCnt++;
			delay(TOUCH_DELAY);
		}
		if(tCnt > LONG_TOUCH_SIZE)
		{
			tNumber = TOUCH_MUTE;
		}
		else
		{
			tNumber = TOUCH_STAGE;
		}
	}	
	else if(digitalRead(TOUCH_3_PIN))
	{
		while(digitalRead(TOUCH_3_PIN))
		{
			tCnt++;
			delay(TOUCH_DELAY);
		}
		
		if(tCnt > LONG_TOUCH_SIZE)
		{
			tNumber = TOUCH_LOCK;
		}
		else
		{
			tNumber = TOUCH_MODE;
		}
	}
	
	if(tNumber != -1)
	{
		touchCallback(tNumber);
		if(isDebugging)
		{
			Serial.print(F("[Touch] Number :  "));
			Serial.println(tNumber);
		}
	}

}
