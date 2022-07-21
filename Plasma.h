// Plasma.h

#ifndef _PLASMA_h
#define _PLASMA_h

#if defined(ESP32)
#define ESP32_BOARD
#endif

#include "arduino.h"
#include "SystemEnv.h"

#ifdef ESP32_BOARD
#define ANALOG_RANGE	4095
#define DC_VOLTAGE		3300
#define ACS_OFFSET		1520
#define PLASMA1_PIN     26
#define PLASMA2_PIN     27
#define FAN_PWM_PIN     23
#define FAN_POWER_PIN   25
#else
#define ANALOG_RANGE	1024
#define DC_VOLTAGE		5000
#define ACS_OFFSET		2500
#define PLASMA1_PIN     3
#define PLASMA2_PIN     4
#define FAN_PWM_PIN     5
#define FAN_POWER_PIN   6
#endif

#define PWM_CHANNEL		1
#define CURRENT_PIN             3
#define CURRENT_AVG_SIZE        10
#define CURRENT_OVERFLOW_LIMIT  3

#define PLASMA_MIN_CURRENT   0.1
#define PLASMA_MAX_CURRENT   0.5

#define FAN_SPEED_MODE1   150
#define FAN_SPEED_MODE2   160
#define FAN_SPEED_MODE3   235

#define MODE1_PLASMA_TIME  5000
#define MODE1_SLEEP_TIME   30000
#define MODE2_PLASMA_TIME  5000
#define MODE2_SLEEP_TIME   15000
#define MODE3_PLASMA_TIME  1500000
#define MODE3_SLEEP_TIME   300000


class Plasma
{
	public:
	Plasma();

	void init(bool _debug = false);

	void bindOffsetCurrent();
	void plasmaDisable();
	void fanDisable();
	void fanEnable(int _speed);
	void setStatus(int _stage, int _fanSpeed);
	
	void run();

	void setChangePlasmaListener(void (*_changePlasmaListener)(bool));

	private:
	bool isDebugging = false;

	float totalCurrent;
	float minCurrentValue;
	float maxCurrentValue;
	float offsetCurrent;
	int measureCurrentCount;
	int overflowCurrentCount;
	bool _isCurrentError = false;
	
	int currentErrorCount;
	int totalCurrentErrorCount;
	
	bool _isRunning = false;
	int plasmaPinNum;
	int stageNum;

	int initCleanModeStep;
	int initCleanModeScanCount;

	unsigned long runningTime;
	unsigned long runningInterval;
	unsigned long enableTime;
	unsigned long sleepTime;

	void plasmaEnable(int pinNum);
	void measureCurrent();
	
	float calculateAverageCurrent(int _size);
	float calculateAverageCurrentAsync();
	
	void checkCurrentOverflow();

	void scanCurrentForNoarmalMode();
	void setNormalPlasmaEnable();

	int initCheckStage = 0;
	int initCheckCount = 0;
	
	void startInitCleanPlasma();
	void scanCurrentForCleanMode();
	void setCleanPlasmaEnable();

};

#endif