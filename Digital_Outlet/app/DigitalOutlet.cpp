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
}

DigitalOutlet::DigitalOutlet(unsigned char pin) {
	this->pin = pin;
	calcSwitchOffTimeStamp();
}


DigitalOutlet::~DigitalOutlet() {
	// TODO Auto-generated destructor stub
}

void DigitalOutlet::calcSwitchOffTimeStamp() {
	DateTime now = SystemClock.now(eTZ_UTC);
	time_t switchOffTime = AppSettings.switchOffTime;

	now.Hour = switchOffTime/3600;
	switchOffTime %= 3600;
	now.Minute = switchOffTime / 60;
	switchOffTime %= 60;
	now.Second = switchOffTime;

	if (SystemClock.now(eTZ_UTC)>now) {
		now.Day++;
	}

	switchOffTimeStamp = now.toUnixTime();
}

void DigitalOutlet::changeState(DIGOUTLETTRANS trans) {
	switch (trans) {
	case manualSwitch_On:
		timeout = AppSettings.maxOnTime;
		calcSwitchOffTimeStamp();
		digOutletState = SwitchedOn;
		break;


	case deviceDetect_On:
		digOutletState = SwitchedOn;
		break;
	case maxOnTimeout_Off:
		digOutletState = SwitchedOff;
		break;
	case switchOffTime_Off:
		digOutletState = SwitchedOff;
		break;
	default:
		break;
	}
}

void DigitalOutlet::outletWorker() {


	switch (digOutletState) {
	case SwitchedOff:
		break;
	case SwitchedOn:
		if (--timeout == 0) {
			changeState(maxOnTimeout_Off);
		}

		if (SystemClock.now(eTZ_UTC).toUnixTime() > switchOffTimeStamp) {
			changeState(switchOffTime_Off);
		}

		break;
	default:
		break;
	}

	digitalWrite(pin, digOutletState==SwitchedOn);
}



