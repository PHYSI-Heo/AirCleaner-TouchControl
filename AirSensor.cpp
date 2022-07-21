#include "AirSensor.h"



Adafruit_SGP30 sgp;

void (*receiveQualityStatus)(int);


AirSensor::AirSensor()
{
	
}


bool AirSensor::init(bool _debug /* = false */)
{
	isDebugging = _debug;
	
	isEnable = sgp.begin();
	if (isDebugging)
	{
		Serial.print(F("[VOC] Sensor Enable : "));
		Serial.println(isEnable);		
	}
	return isEnable;
}


void AirSensor::setQualityCallback(void (*_receiveQualityStatus)(int))
{
	receiveQualityStatus = _receiveQualityStatus;
}

void AirSensor::measure()
{
	if(millis() - measureTime > MEASURE_INTERVAL)
	{
		if (!sgp.IAQmeasure())
		{
			if(isDebugging)
			{
				Serial.println(F("[VOC] Measurement failed.."));
			}
			return;
		}
		
		/*		
		if(isDebugging)
		{
			Serial.print("[Air] TVOC ");
			Serial.print(sgp.TVOC);
			Serial.print(" ppb\t");
			Serial.print("eCO2 ");
			Serial.print(sgp.eCO2);
			Serial.println(" ppm");
		}		
		*/
				
		int mQuality;
		if(sgp.TVOC < QUALITY_GOOD_LIMIT)
		{
			mQuality = VOC_GOOD;				
		}
		else if(sgp.TVOC < QUALITY_NORMAL_LIMIT)
		{			
			mQuality = VOC_NORMAL;
		}
		else
		{
			mQuality = VOC_BAD;
		}
		
		if(mQuality != quality)
		{
			if(isDebugging)
			{
				Serial.print(F("[VOC] Change Quality : "));
				Serial.println(mQuality);
			}
			quality = mQuality;
			receiveQualityStatus(quality);	
		}		
		measureTime = millis();
	}
}


void AirSensor::listen()
{
	if(millis() - measureTime > measureInterval)
	{
		if(delayCount < MEASURE_DELAY_LIMIT)
		{
			receiveQualityStatus(-1);
			delayCount++;
			if(delayCount == MEASURE_DELAY_LIMIT)
			{
				measureInterval = MEASURE_INTERVAL;
			}
		}
		else if(isEnable)
		{
			measure();
		}
		measureTime = millis();
	}
	
}