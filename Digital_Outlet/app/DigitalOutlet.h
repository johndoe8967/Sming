/*
 * DigitalOutlet.h
 *
 *  Created on: 01.01.2016
 *      Author: johndoe
 */

#ifndef APP_DIGITALOUTLET_H_
#define APP_DIGITALOUTLET_H_

#include <SmingCore/SmingCore.h>
#include <AppSettings.h>


enum DIGOUTLETSTATE {SwitchedOff, SwitchedOn};
enum DIGOUTLETTRANS {none, manualSwitch_On, deviceDetect_On, maxOnTimeout_Off, switchOffTime_Off};


class DigitalOutlet {
public:
	DigitalOutlet();
	DigitalOutlet(unsigned char pin);
	virtual ~DigitalOutlet();
	void outletWorker();
	void changeState(DIGOUTLETTRANS trans);
	DIGOUTLETSTATE getState() {return digOutletState;}

private:
	void initOutlet();
	void calcSwitchOffTimeStamp();

	unsigned char pin;
	DIGOUTLETSTATE digOutletState = SwitchedOff;

	time_t switchOffTimeStamp;
	int timeout=0;

};

#endif /* APP_DIGITALOUTLET_H_ */
