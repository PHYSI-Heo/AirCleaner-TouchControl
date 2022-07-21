// Touch.h

#ifndef _TOUCH_h
#define _TOUCH_h

#if defined(ESP32)
#define ESP32_BOARD
#endif

#include "arduino.h"
#include "SystemEnv.h"

#if defined(ESP32)
#define ESP32_BOARD
#endif

#define LONG_TOUCH_SIZE		40

#ifdef ESP32_BOARD
#define TOUCH_1_PIN		5
#define TOUCH_2_PIN		18
#define TOUCH_3_PIN		19
#else
#define TOUCH_1_PIN		A0
#define TOUCH_2_PIN		A1
#define TOUCH_3_PIN		A2
#endif

#define TOUCH_DELAY		50

class Touch
{
	public:
	Touch();
	void init(bool _debug = false);
	
	void listen();
	void setCallback(void (*_touchCallback)(int));

	private:
	bool isDebugging = false;	
	byte tNumber = 0;
};

#endif
