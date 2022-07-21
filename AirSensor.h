// AirSensor.h

#ifndef _AIRSENSOR_h
#define _AIRSENSOR_h

#if defined(ESP32)
#define ESP32_BOARD
#endif

#include "arduino.h"
#include "SystemEnv.h"
#include "Adafruit_SGP30.h"
#include "Wire.h"

#define MEASURE_INTERVAL	2000
#define DELAY_INTERVAL		500

#define QUALITY_GOOD_LIMIT		30
#define QUALITY_NORMAL_LIMIT	80

#define MEASURE_DELAY_LIMIT		20

class AirSensor
{
	public:
	AirSensor();
	bool init(bool _debug = false);
	
	void setQualityCallback(void (*_receiveQualityStatus)(int));
	
	void listen();
	
	private:
	bool isDebugging = false;
	bool isEnable = false;
	
	long measureTime;
	long measureInterval = DELAY_INTERVAL;
	int quality;
	
	int delayCount = 0;
	
	void measure();
};

#endif
