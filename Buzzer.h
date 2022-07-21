// Buzzer.h

#ifndef _BUZZER_h
#define _BUZZER_h


#if defined(ESP32)
#define ESP32_BOARD
#endif

#include "arduino.h"


#ifdef ESP32_BOARD
#define BUZZER_F_PIN    15
#define BUZZER_C_PIN    2
#define BUZZER_CHANNEL	0
#else
#define BUZZER_F_PIN    10
#define BUZZER_C_PIN    2
#endif

#define DEFAULT_FREQ	2000
#define WARN_OUTPUT_TIME  500
#define WARN_SLEEP_TIME   3000

class Buzzer
{
	public:
	Buzzer();

	void init(bool _debug = false);

	void outputSound(long frequency, long ms);
	void setWarningState(bool enable);
	
	void warningSound();
	void startSound();
	void stopSound();
	
	bool isWarnEnable();

	private:
	unsigned long warningTime;
	unsigned long warningInterval;
	bool isWarning = true;
	bool isDebugging = false;
	
	bool isEnable = false;
};

#endif