#include <IRremote.hpp> 
#include "Touch.h"
#include "Plasma.h"
#include "AirSensor.h"
#include "LedPixel.h"
#include "Buzzer.h"
#include "SystemEnv.h"



#if defined(ESP32)
#define IR_PIN          4
#else
#define IR_PIN          7
#endif

#define DEBUG_MODE		1


Buzzer buzzer;
LedPixel led;
Touch touch;
AirSensor aSensor;
Plasma plasma;

int modeNum = MODE_NONE;
int stageNum = PL_DISABLE;
int fanSpeed = -1;
int vocStatus;

bool isTouchBlock;
bool isDeviceEnable;
bool isBreakTime;
bool isMute;

unsigned long runStartTime, runEndTime;
unsigned long breakStartTime, breakEndTime;

int oldStageNum;
bool isTouchEnabled = true;

void startAction(bool _isResetTimer, bool _isLedSetup)
{
	if(stageNum == PL_DISABLE)
	{
		plasma.plasmaDisable();
		plasma.fanDisable();
	}
	else
	{
		isBreakTime = false;
		if (stageNum != PL_STAGE_3)
		{
			runEndTime = TIMER_8H;
		}
		else
		{
			runEndTime = TIMER_3H;
		}
		
		if(_isResetTimer)
		{
			runStartTime = millis();
		}
		
		if(modeNum == MODE_VENTILATION)
		{
			runEndTime = TIMER_8H;
			plasma.plasmaDisable();
			plasma.fanEnable(fanSpeed);
		}
		else
		{
			plasma.setStatus(stageNum, fanSpeed);
		}
	}
		
	if(_isLedSetup)
	{
		led.setStatus(modeNum, stageNum);
	}	
}


void setAutoState()
{
	if(vocStatus == VOC_GOOD || vocStatus == -1)
	{
		stageNum = PL_STAGE_1;
	}
	else
	{
		stageNum = PL_STAGE_2;
	}
	fanSpeed = -1;
}


void changeStage()
{
	modeNum = MODE_NONE;
	fanSpeed = -1;
	stageNum++;
	if(stageNum > PL_STAGE_3)
	{
		stageNum = PL_STAGE_1;
	}
	else if(stageNum == PL_STAGE_3)
	{
		isMute = true;
		buzzer.setWarningState(isMute);
	}
	oldStageNum = stageNum;
	startAction(true, true);
}


void changeMode()
{
	modeNum++;
	
	if(modeNum > MODE_SLEEP)
	{
		modeNum = MODE_NONE;
	}
	
	if(modeNum == MODE_NONE)
	{
		stageNum = PL_STAGE_1;
		fanSpeed = -1;
	}	
	else if (modeNum == MODE_AUTO)
	{
		setAutoState();
	}
	else if (modeNum == MODE_GO_OUT)
	{
		stageNum = PL_STAGE_3;
		fanSpeed = -1;
		buzzer.setWarningState(isMute = true);
	}	
	if(modeNum == MODE_VENTILATION)
	{
		stageNum = PL_VENTILATION;
		fanSpeed = 200;
	}
	else if (modeNum == MODE_SLEEP)
	{
		stageNum = PL_STAGE_1;
		fanSpeed = 100;
	}
	startAction(true, true);	
}


void swapDeviceEnable()
{
	isDeviceEnable = !isDeviceEnable;
	isTouchEnabled = false;
	if(isDeviceEnable)
	{			
		modeNum = MODE_NONE;
		stageNum = PL_STAGE_1;
		oldStageNum = stageNum;
			
		led.startRainbow(1000);
		led.setEffectDelay(1000);
		buzzer.startSound();	
		//startAction(true, false);
	}
	else
	{			
		modeNum = MODE_NONE;
		stageNum = PL_DISABLE;
		
		//plasma.plasmaDisable();
		//plasma.fanDisable();
		led.startRainbow(1000);
		led.setEffectDelay(1000);
		buzzer.stopSound();
		//led.setStatus(modeNum, stageNum);
	}	
}

/*
		Run Timer
*/
void appendRunTime()
{
	runEndTime += IR_SIGNAL_1H;
	if (runEndTime > TIMER_8H)
	{
		runEndTime = TIMER_8H;
	}

	if(DEBUG_MODE)
	{
		Serial.print("(Append) End TIme >> ");
		Serial.println(runEndTime);
	}
}

void startBreakTimer()
{
	isBreakTime = true;
	plasma.plasmaDisable();
	breakStartTime = millis();
	breakEndTime = TIMER_1H;
	
	if(DEBUG_MODE)
	{
		Serial.println(F("(Break) Start Break time.."));
	}
}

void breakTimer() {
	if (millis() - breakStartTime > breakEndTime)
	{
		if(modeNum == MODE_NONE && stageNum == PL_STAGE_3)
		{
			swapDeviceEnable();
		}
		else
		{
			if (modeNum == MODE_GO_OUT)
			{
				// Go Out -> Auto
				modeNum = MODE_AUTO;
				setAutoState();
			}
			startAction(true, true);
		}
	}
}

void runTimer()
{
	if (millis() - runStartTime > runEndTime) 
	{
		if(modeNum == MODE_VENTILATION)
		{
			stageNum = oldStageNum;
			modeNum = MODE_NONE;
			startAction(true, true);
		}
		else
		{
			startBreakTimer();
		}
	}
}

/*
		Callback Methods
*/
void rainbowEffectCallback(void)
{	
	isTouchEnabled = true;
	startAction(true, true);
	if(!isDeviceEnable)
	{	
		led.showBarSingleColor(0, 0, 0);
	}	
}

void receiveAirQualityStatus(int state)
{	
	vocStatus = state;
	led.setVoc(state);
	if (modeNum == MODE_AUTO)
	{
		setAutoState();
		startAction(false, true);
	}
}

void touchCallback(int tNum)
{
	if(tNum == TOUCH_LOCK)
	{
		if(!isDeviceEnable)
		{
			return;
		}
		isTouchBlock = !isTouchBlock;
		led.setLock(isTouchBlock);
		buzzer.outputSound(DEFAULT_FREQ, 75);
	}
	else if(tNum == TOUCH_MUTE)
	{
		if(stageNum != PL_STAGE_3)
		{
			return;
		}
		isMute = !isMute;
		led.setEffectDelay(1000);
		buzzer.setWarningState(isMute);
		buzzer.outputSound(DEFAULT_FREQ, 75);
	}
	else if(!isTouchBlock)
	{
		if(tNum == TOUCH_POWER)
		{
			swapDeviceEnable();
		}
		else if(isDeviceEnable && tNum == TOUCH_STAGE)
		{
			changeStage();
			led.setEffectDelay(1000);
			buzzer.outputSound(DEFAULT_FREQ, 75);
		}		
		else if(isDeviceEnable && tNum == TOUCH_MODE)
		{
			changeMode();
			led.setEffectDelay(1000);
			buzzer.outputSound(DEFAULT_FREQ, 75);
		}		
	}
}


void irReceiver() {
	if (IrReceiver.decode())
	{		
		/*
		if (DEBUG_MODE)
		{
			Serial.print(F("[IR] Protocal : "));
			Serial.print(IrReceiver.decodedIRData.protocol);
			Serial.print(F(", Bit : "));
			Serial.print(IrReceiver.decodedIRData.numberOfBits);
			Serial.print(F(", Raw Data : "));
			Serial.println(IrReceiver.decodedIRData.decodedRawData);
		}
		*/
		
		if (IrReceiver.decodedIRData.protocol == 8 && IrReceiver.decodedIRData.numberOfBits == 32)
		{
			if (DEBUG_MODE)
			{
				Serial.print(F("[IR] Protocal : "));
				Serial.print(IrReceiver.decodedIRData.protocol);
				Serial.print(F(", Bit : "));
				Serial.print(IrReceiver.decodedIRData.numberOfBits);
				Serial.print(F(", Raw Data : "));
				Serial.println(IrReceiver.decodedIRData.decodedRawData);
			}
			
			if (IrReceiver.decodedIRData.decodedRawData == IR_SIGNAL_POWER)
			{
				swapDeviceEnable();
			}
			else if (isDeviceEnable)
			{
				if (IrReceiver.decodedIRData.decodedRawData == IR_SIGNAL_MODE1)
				{
					modeNum = MODE_NONE;
					stageNum = PL_STAGE_1;
					startAction(true, true);
					led.setEffectDelay(1000);
					buzzer.outputSound(DEFAULT_FREQ, 75);
				}
				else if (IrReceiver.decodedIRData.decodedRawData == IR_SIGNAL_MODE2)
				{
					modeNum = MODE_NONE;
					stageNum = PL_STAGE_2;
					startAction(true, true);
					led.setEffectDelay(1000);
					buzzer.outputSound(DEFAULT_FREQ, 75);
				}
				else if (IrReceiver.decodedIRData.decodedRawData == IR_SIGNAL_MODE3)
				{					
					modeNum = MODE_NONE;
					stageNum = PL_STAGE_3;
					startAction(true, true);
					led.setEffectDelay(1000);
					buzzer.outputSound(DEFAULT_FREQ, 75);
					buzzer.setWarningState(isMute = true);
				}
				else if(stageNum == PL_STAGE_3)
				{
					if (IrReceiver.decodedIRData.decodedRawData == IR_SIGNAL_1H)
					{
						appendRunTime();
						led.setEffectDelay(1000);
						buzzer.outputSound(DEFAULT_FREQ, 75);
					}
					else if (IrReceiver.decodedIRData.decodedRawData == IR_SIGNAL_2H)
					{
						appendRunTime();
						appendRunTime();
						led.setEffectDelay(1000);
						buzzer.outputSound(DEFAULT_FREQ, 75);
					}
					else if (IrReceiver.decodedIRData.decodedRawData == IR_SIGNAL_MUTE)
					{
						buzzer.setWarningState(isMute = !isMute);
						led.setEffectDelay(1000);
						buzzer.outputSound(DEFAULT_FREQ, 75);
					}
				}				
			}
		}
		IrReceiver.resume();
	}
}

/*
		Main Method
*/
void setup()
{
	Serial.begin(115200);
	Serial.setTimeout(10);	
	if(Serial){
		delay(500);
	}
	IrReceiver.begin(IR_PIN);
	
	buzzer.init(DEBUG_MODE);	
	led.init(DEBUG_MODE);
	touch.init(DEBUG_MODE);
	aSensor.init(DEBUG_MODE);
	plasma.init(DEBUG_MODE);
	
	aSensor.setQualityCallback(receiveAirQualityStatus);
	touch.setCallback(touchCallback);
	led.setRainbowEffectCallback(rainbowEffectCallback);
	
	led.showIconSingleColor(DEFAULT_BRIGTHNESS, DEFAULT_BRIGTHNESS, DEFAULT_BRIGTHNESS);
	led.showBarSingleColor(0, 0, 0);	
	buzzer.outputSound(DEFAULT_FREQ, 75);
	
	if(DEBUG_MODE)
	{
		Serial.println(F("KLAEN_Touch_Ver1.2.0"));
	}	
}

void loop()
{	
	if(isTouchEnabled)
	{
		touch.listen();
	}
	
	irReceiver();	
 	aSensor.listen();	
	 	 
	if(isDeviceEnable)
	{			
		if(stageNum == PL_STAGE_3)
		{			
			buzzer.warningSound();
			if(buzzer.isWarnEnable())
			{
				led.setEffectDelay(1000);
			}
		}
		
		if(isBreakTime)
		{
			breakTimer();
		}
		else
		{
			plasma.run();
			runTimer();
		}		
	}
	
	led.show();
}