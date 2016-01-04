/*
 * DigitalOutlet.cpp
 *
 *  Created on: 01.01.2016
 *      Author: johndoe
 */

#include "DigitalOutlet.h"

void DigitalOutlet::initOutlet(){
	pinMode(pin, OUTPUT);
	digitalWrite(pin, digOutletState==SwitchedOn);
}
DigitalOutlet::DigitalOutlet():pin(0) {
	calcSwitchOffTimeStamp();
	calcSwitchOnTimeStamp();
	initOutlet();
}

DigitalOutlet::DigitalOutlet(unsigned char pin) {
	this->pin = pin;
	calcSwitchOffTimeStamp();
	calcSwitchOnTimeStamp();
	initOutlet();
}


DigitalOutlet::~DigitalOutlet() {
	pinMode(pin, INPUT);
}

time_t DigitalOutlet::calcNextSwitchTimeStamp(time_t time) {
	DateTime now = SystemClock.now(eTZ_Local);
	if ((time < 86400) && (time > 0)) {

		now.Hour = time/3600;
		time %= 3600;
		now.Minute = time / 60;
		time %= 60;
		now.Second = time;

		if (SystemClock.now(eTZ_Local).toUnixTime() > now.toUnixTime()) {
			Serial.print("SwitchTime in the past, add a day: ");
			Serial.println(now);
			now.Day++;
		}
		Serial.print("SwitchTime: ");
		Serial.print(now);
		Serial.print(" ActTime: ");
		Serial.println(SystemClock.now(eTZ_Local).toUnixTime());

		return now.toUnixTime();
	} else {
		Serial.println("invalid Time");
		return 0x7fffffff;
	}
}


void DigitalOutlet::calcSwitchOffTimeStamp() {
	switchOffTimeStamp = calcNextSwitchTimeStamp(AppSettings.switchOffTime);
}
void DigitalOutlet::calcSwitchOnTimeStamp() {
	switchOnTimeStamp = calcNextSwitchTimeStamp(AppSettings.switchOnTime);
}

void DigitalOutlet::changeState(DIGOUTLETTRANS trans) {
	switch (trans) {
	case manualSwitch_On:
	case mqttSwitch_On:
	case switchOnTime_On:
		Serial.print("Get maxOnTime: ");
		Serial.println(AppSettings.maxOnTime);
		timeout = AppSettings.maxOnTime;

		calcSwitchOffTimeStamp();
		digOutletState = SwitchedOn;
		break;


	case deviceDetect_On:
		digOutletState = SwitchedOn;
		break;

	case maxOnTimeout_Off:
	case switchOffTime_Off:
	case manualSwitch_Off:
	case mqttSwitch_Off:
		calcSwitchOnTimeStamp();
		digOutletState = SwitchedOff;
		break;
	default:
		break;
	}
}

void DigitalOutlet::outletWorker() {

/*	Serial.print("SwitchOnTime: ");
	Serial.print(switchOnTimeStamp);
	Serial.print(" SwitchOffTime: ");
	Serial.print(switchOffTimeStamp);
	Serial.print(" ActTime: ");
	Serial.println(SystemClock.now(eTZ_Local).toUnixTime());*/



	switch (digOutletState) {
	case SwitchedOff:
		if (SystemClock.now(eTZ_Local).toUnixTime() > switchOnTimeStamp) {
			Serial.print("SwitchOnTime: ");
			Serial.println(switchOnTimeStamp);
			changeState(switchOnTime_On);
		}

		break;
	case SwitchedOn:
		if (--timeout == 0) {
			changeState(maxOnTimeout_Off);
		}

		if (SystemClock.now(eTZ_Local).toUnixTime() > switchOffTimeStamp) {
			Serial.print("SwitchOffTime: ");
			Serial.println(switchOffTimeStamp);
			changeState(switchOffTime_Off);
		}

		break;
	default:
		break;
	}

	digitalWrite(pin, digOutletState==SwitchedOn);

}

