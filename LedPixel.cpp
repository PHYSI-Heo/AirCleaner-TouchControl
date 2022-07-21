#include "LedPixel.h"


Adafruit_NeoPixel barPixels(LED_BAR_SIZE, LED_BAR_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel iconPixels(LED_ICON_SIZE, LED_ICON_PIN, NEO_GRB + NEO_KHZ800);

void (*RainbowEffectCallback)(void);

LedPixel::LedPixel()
{
	
}

void LedPixel::init(bool _debug)
{
	isDebugging = _debug;
	
	if(isDebugging)
	{
		Serial.print(F("[LED] Bar pin : "));
		Serial.print(LED_BAR_PIN);
		Serial.print(F(", Icon pin : "));
		Serial.println(LED_ICON_PIN);
	}
	
	barPixels.begin();
	iconPixels.begin();

	//barPixels.setBrightness(BAR_BRIGTHNESS);
	barPixels.clear();
	barPixels.show();

	//iconPixels.setBrightness(ICON_BRIGTHNESS);
	iconPixels.clear();
	iconPixels.show();
	
	stageColor = iconPixels.Color(255, 255, 255);
}

void LedPixel::setRainbowEffectCallback(void (*_RainbowEffectCallback)(void))
{
	RainbowEffectCallback = _RainbowEffectCallback;
}


void LedPixel::setBacklight(bool _enable)
{
	isBacklight = _enable;
	if (isDebugging)
	{
		Serial.print(F("[LED] Backlight Enable : "));
		Serial.println(digitalRead(isBacklight));
	}
	showIconEffect();
}


uint32_t LedPixel::getRGBColor(byte pos)
{
	pos = 255 - pos;
	if (pos < 85)
	{
		return barPixels.Color(255 - pos * 3, 0, pos * 3);
	}
	if (pos < 170)
	{
		pos -= 85;
		return barPixels.Color(0, pos * 3, 255 - pos * 3);
	}
	pos -= 170;
	return barPixels.Color(pos * 3, 255 - pos * 3, 0);
}


void LedPixel::showBarSingleColor(int red, int green, int blue)
{
	for (uint16_t i = 0; i < barPixels.numPixels(); i++)
	{
		barPixels.setPixelColor(i, barPixels.Color(red, green, blue));
	}
	barPixels.show();
}

void LedPixel::startRainbow(long ms)
{
	rainbowRotateCnt = 0;
	rainbowRotateSize = ms / rainbowInterval;
	isRainbowEffect = true;
	
	showRainbow();
}

void LedPixel::showRainbow()
{
	if(millis() - rainbowTime > rainbowInterval)
	{
		if(rainbowRotateCnt == rainbowRotateSize)
		{
			isRainbowEffect = false;
			RainbowEffectCallback();
			//barPixels.clear();
			//barPixels.show();
		}
		else
		{
			for (int i = 0; i < barPixels.numPixels(); i++)
			{
				barPixels.setPixelColor(i, getRGBColor(((i * 256 / barPixels.numPixels()) + (rainbowRotateCnt)) & 255));
			}
			barPixels.show();
			rainbowRotateCnt++;
		}
		rainbowTime = millis();
	}
}

void LedPixel::showStatusBarEffect()
{
	if(modeNum == MODE_SLEEP || stageNum == PL_DISABLE)
	{
		return;
	}
	
	if(millis() - effectTime > EFFECT_INTERVAL)
	{
		barPixelPos++;
		for (int i = 0; i < barPixels.numPixels() / 2; i++)
		{
			int colorPos = barColorPos + (i * barColorStep);
			int pixel1 = (i + barPixelPos) % LED_BAR_SIZE;
			int pixel2 = (barPixels.numPixels() - i - 1 + barPixelPos) % LED_BAR_SIZE;
			barPixels.setPixelColor(pixel1, getRGBColor(colorPos));
			barPixels.setPixelColor(pixel2, getRGBColor(colorPos));
		}
		barPixels.show();

		if (barPixelPos >= LED_BAR_SIZE)
		{
			barPixelPos = 0;
		}
		effectTime = millis();
	}
}

void LedPixel::show()
{
	if(isEffectDelay)
	{
		if(millis() - effectDelayTime > effectDelayLimit)
		{
			isEffectDelay = false;
		}
	}
	else
	{
		if(isRainbowEffect)
		{
			showRainbow();
		}
		else
		{
			showStatusBarEffect();
		}
	}	
}

void LedPixel::clear()
{
	barPixels.clear();
	barPixels.show();
	iconPixels.clear();
	iconPixels.show();	
}

void LedPixel::setEffectDelay(long _ms)
{
	isEffectDelay = true;
	effectDelayTime = millis();
	effectDelayLimit = _ms;
}

void LedPixel::setStatus(int _mode, int _stage)
{
	if(modeNum == _mode && _stage == stageNum)
	{
		return;
	}
	
	modeNum = _mode;
	if(modeNum == MODE_NONE)
	{
		modePixelPos = -1;
	}
	else if(modeNum == MODE_AUTO)
	{
		modePixelPos = MODE_5_POS;
	}
	else if(modeNum == MODE_GO_OUT)
	{
		modePixelPos = MODE_4_POS;
	}
	else if(modeNum == MODE_VENTILATION)
	{
		modePixelPos = MODE_3_POS;
	}
	else if(modeNum == MODE_SLEEP)
	{
		modePixelPos = MODE_2_POS;
		barPixels.clear();
		barPixels.show();
	}
	
	stageNum = _stage;
	if(stageNum == PL_DISABLE)
	{
		stagePixelPos = -1;
		barPixels.clear();
		barPixels.show();
	}
	else if(stageNum == PL_STAGE_1)
	{
		barColorPos = GREEN_MIN_POS;
		stagePixelPos = STAGE_1_POS;
		barColorStep = 8;
		//stageColor = getRGBColor(barColorPos);
	}
	else if(stageNum == PL_STAGE_2)
	{
		barColorPos = BLUE_MIN_POS;
		stagePixelPos = STAGE_2_POS;
		barColorStep = 8;
		//stageColor = getRGBColor(barColorPos);
	}
	else if(stageNum == PL_STAGE_3)
	{
		barColorPos = RED_MIN_POS;
		stagePixelPos = STAGE_3_POS;
		barColorStep = 8;
		//stageColor = getRGBColor(barColorPos);
	}
	else if(stageNum == PL_VENTILATION)
	{
		stagePixelPos = -1;
		barColorPos = BLUE_MIN_POS;
		barColorStep = 12;
	}
	
	showIconEffect();
	showStatusBarEffect();
	
	if (isDebugging)
	{
		Serial.print(F("[LED] Set Mode : "));
		Serial.print(modeNum);
		Serial.print(F("\t Stage : "));
		Serial.println(stageNum);
	}
}

void LedPixel::setVoc(int _state)
{
	if(_state == VOC_GOOD)
	{
		iconPixels.setPixelColor(8, iconPixels.Color(0, 128, 0));
	}
	else if(_state == VOC_NORMAL)
	{
		iconPixels.setPixelColor(8, iconPixels.Color(128, 128, 0));
	}
	else if(_state == VOC_BAD)
	{
		iconPixels.setPixelColor(8, iconPixels.Color(128, 0, 0));
	}
	else
	{
		int colorNum = vocDelayCnt++ % 4;
		
		if(colorNum == 0)
		{
			iconPixels.setPixelColor(8, iconPixels.Color(0, 128, 0));
		}
		else if(colorNum == 1)
		{
			iconPixels.setPixelColor(8, iconPixels.Color(128, 128, 0));
		}
		else if(colorNum == 2)
		{
			iconPixels.setPixelColor(8, iconPixels.Color(128, 64, 0));
		}
		else if(colorNum == 3)
		{
			iconPixels.setPixelColor(8, iconPixels.Color(128, 0, 0));
		}
	}
	iconPixels.show();
}

void LedPixel::setLock(int _enable)
{
	isLock = _enable;
	if(_enable)
	{
		iconPixels.setPixelColor(0, stageColor);
	}
	else
	{
		iconPixels.setPixelColor(0, iconPixels.Color(DEFAULT_BRIGTHNESS, DEFAULT_BRIGTHNESS, DEFAULT_BRIGTHNESS));
	}

	iconPixels.show();
}

void LedPixel::showIconEffect()
{
	for (int i = 0; i < iconPixels.numPixels() - 1; i++)
	{
		if(isBacklight)
		{
			iconPixels.setPixelColor(i, iconPixels.Color(DEFAULT_BRIGTHNESS, DEFAULT_BRIGTHNESS, DEFAULT_BRIGTHNESS));
		}
		else
		{
			iconPixels.setPixelColor(i, iconPixels.Color(0, 0, 0));
		}
	}
	
	if(modePixelPos != -1)
	{
		if(isBacklight)
		{
			iconPixels.setPixelColor(modePixelPos, stageColor);
		}
		else
		{
			iconPixels.setPixelColor(modePixelPos, iconPixels.Color(255, 255, 255));
		}
	}
	
	if(stagePixelPos != -1)
	{
		iconPixels.setPixelColor(stagePixelPos, stageColor);
	}
	
	if(isLock)
	{
		iconPixels.setPixelColor(0, iconPixels.Color(255, 255, 255));
	}
	
	iconPixels.show();
}

void LedPixel::showIconSingleColor(int red, int green, int blue)
{
	for (uint16_t i = 0; i < iconPixels.numPixels() - 1; i++)
	{
		iconPixels.setPixelColor(i, iconPixels.Color(red, green, blue));
	}
	iconPixels.show();
}