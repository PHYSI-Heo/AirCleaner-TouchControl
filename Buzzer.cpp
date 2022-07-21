#include "Buzzer.h"


Buzzer::Buzzer() {
}

void Buzzer::init(bool _debug) {
	isDebugging = _debug;
	pinMode(BUZZER_C_PIN, OUTPUT);
	digitalWrite(BUZZER_C_PIN, LOW);
			
	if(isDebugging)
	{
		Serial.print(F("[Buzzer] PWM pin :"));
		Serial.print(BUZZER_F_PIN);
		Serial.print(F(", Charge pin :"));
		Serial.println(BUZZER_C_PIN);
	}
	
	#ifdef ESP32_BOARD
	ledcAttachPin(BUZZER_F_PIN, BUZZER_CHANNEL);
	#endif
}

void Buzzer::outputSound(long frequency, long ms) {	
	#ifdef ESP32_BOARD
	ledcWriteTone(BUZZER_CHANNEL, frequency);
	#else
	tone(BUZZER_F_PIN, frequency);
	#endif
	digitalWrite(BUZZER_C_PIN, HIGH);
	delay(ms);
	digitalWrite(BUZZER_C_PIN, LOW);
		
	if(isDebugging)
	{
		Serial.println(F("[Buzzer] Output Sound.."));
	}
}

void Buzzer::setWarningState(bool enable) {
	//noTone(BUZZER_F_PIN);
	isWarning = enable;
	warningTime = millis();
	warningInterval = WARN_SLEEP_TIME;
	isEnable = false;
	
	if (isDebugging) {
		Serial.print(F("[Buzzer] Warning Enable : "));
		Serial.println(isWarning);
	}
}

void Buzzer::warningSound() {
	if (isWarning && millis() - warningTime > warningInterval) {
		isEnable = !isEnable;
		if(isEnable)
		{	
			warningInterval = WARN_OUTPUT_TIME;
			#ifdef ESP32_BOARD
			ledcWriteTone(BUZZER_CHANNEL, DEFAULT_FREQ);
			#else
			tone(BUZZER_F_PIN, DEFAULT_FREQ);
			#endif
			digitalWrite(BUZZER_C_PIN, HIGH);
		}
		else
		{
			warningInterval = WARN_SLEEP_TIME;
			digitalWrite(BUZZER_C_PIN, LOW);
		}
		warningTime = millis();
	}
}


void Buzzer::startSound()
{
	outputSound(DEFAULT_FREQ, 100);
}

void Buzzer::stopSound()
{
	outputSound(DEFAULT_FREQ, 100);
}

bool Buzzer::isWarnEnable()
{
	return isEnable;
}