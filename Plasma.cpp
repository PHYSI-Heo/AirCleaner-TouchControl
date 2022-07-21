#include "Plasma.h"

int cleanPlasma1OvfCount;
int cleanPlasma2OvfCount;
bool isCleanPlasma1Error;
bool isCleanPlasma2Error;

void  (*changePlasmaListener)(bool);

Plasma::Plasma()
{
}

void Plasma::init(bool _debug)
{
	isDebugging = _debug;

	pinMode(PLASMA1_PIN, OUTPUT);
	pinMode(PLASMA2_PIN, OUTPUT);
	pinMode(FAN_POWER_PIN, OUTPUT);
	
	#ifdef ESP32_BOARD
	ledcAttachPin(FAN_PWM_PIN, PWM_CHANNEL);
	ledcSetup(PWM_CHANNEL, 5000, 16);      //  ledcSetup(ch, freq, resolution) // 16 bit (0~65535)
	#else
	pinMode(FAN_PWM_PIN, OUTPUT);
	#endif

	digitalWrite(PLASMA1_PIN, LOW);
	digitalWrite(PLASMA2_PIN, LOW);
	digitalWrite(FAN_PWM_PIN, LOW);
	digitalWrite(FAN_POWER_PIN, LOW);
}


void Plasma::setChangePlasmaListener(void (*_changePlasmaListener)(bool))
{
	changePlasmaListener = _changePlasmaListener;
}

void Plasma::plasmaDisable()
{
	digitalWrite(PLASMA1_PIN, LOW);
	delay(10);
	digitalWrite(PLASMA2_PIN, LOW);
	delay(10);
}

void Plasma::plasmaEnable(int pinNum)
{
	digitalWrite(pinNum, HIGH);
	delay(10);
}

void Plasma::fanDisable()
{
	digitalWrite(FAN_POWER_PIN, LOW);
	digitalWrite(FAN_PWM_PIN, LOW);
}

void Plasma::fanEnable(int _speed)
{
	digitalWrite(FAN_POWER_PIN, HIGH);
	#ifdef ESP32_BOARD
	ledcWrite(PWM_CHANNEL, _speed);
	#else
	analogWrite(FAN_PWM_PIN, _speed);
	#endif
	
	if (isDebugging)
	{
		Serial.print(F("[Plasma] Fan Speed : "));
		Serial.println(_speed);
	}
}


/*
		====== Current Calculator ======
*/
void Plasma::measureCurrent()
{
	float amp = ((analogRead(CURRENT_PIN) / 1024.0) * 5000 - 2500) / 185;
	if (isnan(amp))
	{
		return;
	}
	
	totalCurrent += amp;
	measureCurrentCount++;
}

float Plasma::calculateAverageCurrent(int _size) 
{
	totalCurrent = 0;
	measureCurrentCount = 0;
	for (int i = 0; i < _size; i++) 
	{
		measureCurrent();
		delay(50);
	}
	return totalCurrent / measureCurrentCount;
}

void Plasma::bindOffsetCurrent() 
{
	offsetCurrent = calculateAverageCurrent(10);
	if (isnan(offsetCurrent)) 
	{
		offsetCurrent = 0.7;
	}

	if (isDebugging) {
		Serial.print(F("[AMP] Current Offset : "));
		Serial.println(offsetCurrent);
	}
}

float Plasma::calculateAverageCurrentAsync() 
{
	measureCurrent();
	if (measureCurrentCount < CURRENT_AVG_SIZE) 
	{
		return -1.0f;
	}
	float avgAmp = totalCurrent / measureCurrentCount - offsetCurrent;
	totalCurrent = 0;
	measureCurrentCount = 0;
	
	if (isDebugging) {
		Serial.print(F("[AMP] Average Current : "));
		Serial.println(avgAmp);
	}
	
	return avgAmp;
}


void Plasma::checkCurrentOverflow()
{
	float amp = calculateAverageCurrentAsync();
	if (amp == -1 || isnan(amp))
	{
		return;
	}

	if (amp < minCurrentValue || amp > maxCurrentValue)
	{
		overflowCurrentCount++;
		if (isDebugging)
		{
			Serial.print(F("[AMP] Current Overflow : "));
			Serial.print(overflowCurrentCount);
			Serial.print(F(" ( "));
			Serial.print(amp);
			Serial.println(F(" )"));
		}
	}
	else
	{
		overflowCurrentCount = 0;
	}
	initCheckCount++;
}


/*
      ====== Normal Plasma Functions ======
*/
void Plasma::scanCurrentForNoarmalMode()
{
	if (!_isRunning || _isCurrentError) {
		return;
	}

	if (overflowCurrentCount < CURRENT_OVERFLOW_LIMIT)
	{
		checkCurrentOverflow();
	}
	else
	{
		_isCurrentError = true;
		digitalWrite(plasmaPinNum, LOW);

		currentErrorCount++;
		totalCurrentErrorCount++;

		if (isDebugging)
		{
			Serial.print(F("[AMP] Current Error Count : "));
			Serial.print(currentErrorCount);
			Serial.print(F(", Total Count "));
			Serial.println(totalCurrentErrorCount);
		}
	}
}

void Plasma::setNormalPlasmaEnable()
{
	_isRunning = !_isRunning;

	if (_isRunning)
	{
		if (_isCurrentError)
		{
			plasmaPinNum = plasmaPinNum == PLASMA1_PIN ? PLASMA2_PIN : PLASMA1_PIN;
			_isCurrentError = false;
			if (isDebugging)
			{
				Serial.print("([Plasma] Swap Pin Number : ");
				Serial.println(plasmaPinNum);
			}
		}
		plasmaEnable(plasmaPinNum);
	}
	else
	{
		if (!_isCurrentError)
		{
			currentErrorCount = 0;
		}
		plasmaDisable();
	}
	
	overflowCurrentCount = 0;
	
	if (isDebugging)
	{
		Serial.print(F("[Plasma] Normal Mode Runnable : "));
		Serial.println(_isRunning);
	}
}



/*
      ====== Clean Functions ======
*/
void Plasma::startInitCleanPlasma()
{
	if (initCheckStage == 0)
	{
		minCurrentValue = PLASMA_MIN_CURRENT;
		maxCurrentValue = PLASMA_MAX_CURRENT;
		plasmaDisable();
		plasmaEnable(PLASMA1_PIN);
		overflowCurrentCount = 0;
		initCheckStage++;
	}
	else if (initCheckStage == 1)
	{
		checkCurrentOverflow();
		if (initCheckCount >= 3)
		{
			cleanPlasma1OvfCount = overflowCurrentCount;
			initCheckStage++;
		}
	}
	else if (initCheckStage == 2)
	{
		plasmaDisable();
		plasmaEnable(PLASMA2_PIN);
		overflowCurrentCount = 0;
		initCheckCount = 0;
		initCheckStage++;
	}
	else if (initCheckStage == 3)
	{
		checkCurrentOverflow();
		if (initCheckCount >= 3)
		{
			cleanPlasma2OvfCount = overflowCurrentCount;
			initCheckStage++;
		}
	}
	else if (initCheckStage == 4)
	{
		plasmaDisable();
		minCurrentValue = 0;
		maxCurrentValue = 0;
		overflowCurrentCount = 0;

		isCleanPlasma1Error = cleanPlasma1OvfCount >= CURRENT_OVERFLOW_LIMIT;
		isCleanPlasma2Error = cleanPlasma2OvfCount >= CURRENT_OVERFLOW_LIMIT;

		if (!isCleanPlasma1Error)
		{
			minCurrentValue += PLASMA_MIN_CURRENT;
			maxCurrentValue += PLASMA_MAX_CURRENT;
			plasmaEnable(PLASMA1_PIN);
		}
		if (!isCleanPlasma2Error)
		{
			minCurrentValue += PLASMA_MIN_CURRENT;
			maxCurrentValue += PLASMA_MAX_CURRENT;
			plasmaEnable(PLASMA2_PIN);
		}

		if (isCleanPlasma1Error || isCleanPlasma1Error)
		{
			currentErrorCount++;
			totalCurrentErrorCount++;
			
			if (isDebugging)
			{
				Serial.print(F("[AMP] Current Error Count : "));
				Serial.print(currentErrorCount);
				Serial.print(F(", Total Count "));
				Serial.println(totalCurrentErrorCount);
			}
		}
		
		_isCurrentError = isCleanPlasma1Error && isCleanPlasma2Error;
	}
}


void Plasma::scanCurrentForCleanMode()
{
	if (!_isRunning || _isCurrentError)
	{
		return;
	}

	if (overflowCurrentCount < CURRENT_OVERFLOW_LIMIT)
	{
		checkCurrentOverflow();
	}
	else
	{
		startInitCleanPlasma();
	}
}


void Plasma::setCleanPlasmaEnable() {
	_isRunning = !_isRunning;

	if (_isRunning)
	{
		startInitCleanPlasma();
	}
	else
	{
		if (!_isCurrentError)
		{
			currentErrorCount = 0;
		}
		plasmaDisable();
	}
	overflowCurrentCount = 0;
	runningInterval = _isRunning ? enableTime : sleepTime;
	runningTime = millis();
	if (isDebugging)
	{
		Serial.print(F("[Plasma] Clean Mode Runnable : "));
		Serial.println(_isRunning);
	}
}


void Plasma::setStatus(int _stage, int _fanSpeed) {
	if (isDebugging)
	{
		Serial.print(F("[Plasma] Stage Number : "));
		Serial.println(_stage);
	}
	
	plasmaDisable();	
	totalCurrent = 0;
	measureCurrentCount = 0;
	overflowCurrentCount = 0;
	currentErrorCount = 0;
	_isCurrentError = false;
	
	stageNum = _stage;

	if (stageNum == PL_STAGE_1)
	{
		plasmaPinNum = PLASMA1_PIN;
		runningInterval = enableTime = MODE1_PLASMA_TIME;
		sleepTime = MODE1_SLEEP_TIME;
		minCurrentValue = PLASMA_MIN_CURRENT;
		maxCurrentValue = PLASMA_MAX_CURRENT;
		_isRunning = true;
		
		fanEnable(_fanSpeed == -1 ? FAN_SPEED_MODE1 : _fanSpeed);
		plasmaEnable(plasmaPinNum);
	}
	else if (stageNum == PL_STAGE_2)
	{
		plasmaPinNum = PLASMA2_PIN;
		runningInterval = enableTime = MODE2_PLASMA_TIME;
		sleepTime = MODE2_SLEEP_TIME;
		minCurrentValue = PLASMA_MIN_CURRENT;
		maxCurrentValue = PLASMA_MAX_CURRENT;
		_isRunning = true;
		
		fanEnable(_fanSpeed == -1 ? FAN_SPEED_MODE2 : _fanSpeed);
		plasmaEnable(plasmaPinNum);
	}
	else if (stageNum == PL_STAGE_3)
	{
		runningInterval = enableTime = MODE3_PLASMA_TIME;
		sleepTime = MODE3_SLEEP_TIME;		
		_isRunning = true;
		
		fanEnable(_fanSpeed == -1 ? FAN_SPEED_MODE3 : _fanSpeed);
		startInitCleanPlasma();
	}
	else
	{
		_isRunning = false;
		fanDisable();
	}
	
	//changePlasmaListener(_isRunning);
	runningTime = millis();
}


void Plasma::run()
{
	if (millis() - runningTime > runningInterval)
	{
		if (stageNum != PL_STAGE_3)
		{
			setNormalPlasmaEnable();
			scanCurrentForNoarmalMode();
		}
		else
		{
			setCleanPlasmaEnable();
			scanCurrentForCleanMode();
		}		
		//changePlasmaListener(_isRunning);
		runningInterval = _isRunning ? enableTime : sleepTime;
		runningTime = millis();
	}
}