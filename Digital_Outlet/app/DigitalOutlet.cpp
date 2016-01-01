/*
 * DigitalOutlet.cpp
 *
 *  Created on: 01.01.2016
 *      Author: johndoe
 */

#include "DigitalOutlet.h"
#define OUT_PIN 2 // GPIO2



void DigitalOutlet::initOutlet(){
	pinMode(OUT_PIN, OUTPUT);
	digitalWrite(OUT_PIN, digOutletState==SwitchedOn);
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
	case switchOffTime_Off:
	case manualSwitch_Off:
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

	if(digOutletState==SwitchedOn) {
		debugf("set output");
		digitalWrite(OUT_PIN, 1);
	} else {
		debugf("clear output");
		digitalWrite(OUT_PIN, 0);
	}

	//digitalWrite(pin, digOutletState==SwitchedOn);
}



