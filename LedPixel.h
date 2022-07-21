// LedPixel.h

#ifndef _LEDPIXEL_h
#define _LEDPIXEL_h

#if defined(ESP32)
#define ESP32_BOARD
#endif

#include "arduino.h"
#include "Adafruit_NeoPixel.h"
#include "SystemEnv.h"

#define LED_BAR_SIZE		12
#define LED_ICON_SIZE		9
#define BAR_BRIGTHNESS		150
#define ICON_BRIGTHNESS		225

#ifdef ESP32_BOARD
#define LED_BAR_PIN		16
#define LED_ICON_PIN	17
#else
#define LED_BAR_PIN		8
#define LED_ICON_PIN	9
#endif


#define GREEN_MIN_POS   50
#define GREEN_MAX_POS   100
#define BLUE_MIN_POS    115
#define BLUE_MAX_POS    165
#define RED_MIN_POS     0
#define RED_MAX_POS     40

#define EFFECT_INTERVAL   250

#define STAGE_1_POS		5
#define STAGE_2_POS		6
#define STAGE_3_POS		7

#define MODE_1_POS		0
#define MODE_2_POS		1
#define MODE_3_POS		2
#define MODE_4_POS		3
#define MODE_5_POS		4

#define DEFAULT_BRIGTHNESS	25

class LedPixel
{
	public:
		LedPixel();

		void init(bool _debug = false);

		void setStatus(int _mode, int _stage);
		void setVoc(int _stage);
		void setLock(int _enable);
		void setBacklight(bool _enable);

		void showIconSingleColor(int red, int green, int blue);
		void showBarSingleColor(int red, int green, int blue);
	
		void startRainbow(long ms);
		void showRainbow();

		void showStatus();
		void showStatusBarEffect();
	
		void show();
		void clear();
		void setRainbowEffectCallback(void (*_RainbowEffectCallback)(void));
		
		void setEffectDelay(long ms);
	
	private:
		bool isDebugging = false;
		bool isBacklight = true;
		bool isLock = false;
		
		int vocDelayCnt = 0;
		int modeNum = MODE_NONE;
		int modePixelPos = -1;
		int stageNum = PL_DISABLE;
		int stagePixelPos = -1;
		int vocStage;

		int barPixelPos;
		int barColorPos;
		int barColorStep;
		
		long effectTime;

		uint32_t stageColor;
		uint32_t getRGBColor(byte pos);
		
		void showIconEffect();
		
		int rainbowRotateCnt;
		int rainbowRotateSize;
		bool isRainbowEffect;
		long rainbowTime;
		long rainbowInterval = 10;
		
		bool isEffectDelay = false;
		long effectDelayTime;
		long effectDelayLimit;
};

#endif