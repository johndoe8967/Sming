/*
 * sendData.cpp
 *
 *  Created on: 15.05.2016
 *      Author: johndoe
 */
#include "../include/sendData.h"
#include <SmingCore/Debug.h>

#define doseRatio 175.43

//#define useRadmon
String RadmonUserName = "-------";
String RadmonPassWord = "-------";
String RadmonHost = "http://radmon.org";     // no need to change this

#define useThingSpeak
String writeAPI = "EA78KMUUOAUOYQAK";
String ThingSpeakHost = "http://api.thingspeak.com";  // no need to change this
HttpClient thingSpeak;
HttpClient radmon;

void onDataSent(HttpClient& client, bool successful)
{
	if (successful)
		Debug.printf("Success sent\r\n");
	else
		Debug.printf("Failed\r\n");

	String response = client.getResponseString();
	Debug.printf("Server response: '%s'\r\n",response.c_str());
	if (response.length() > 0)
	{
		int intVal = response.toInt();

		if (intVal == 0)
			Debug.printf("Sensor value wasn't accepted. May be we need to wait a little?\r\n");
	}
}

#define teststable
	float dose = 0;

void sendData(uint32 events, uint32 intervall) {
	if (thingSpeak.isProcessing()) return; // We need to wait while request processing was completed

	float cpm = float(events)/ (float(intervall)/60000000.0);

#define teststable
#ifdef teststable
	dose++;
#else
	dose = cpm / doseRatio;
#endif

	Debug.printf ("CPM: %f Dose: %f Time: %s\r\n", cpm, dose, SystemClock.now(eTZ_UTC).toISO8601().c_str());

	String url;
#ifdef useRadmon
	url = RadmonHost;
	url += "/radmon.php?function=submit&user=";
	url += RadmonUserName;
	url += "&password=";
	url += RadmonPassWord;
	url += "&value=";
	url += cpm;
	url += "&unit=CPM";
	radmon.downloadString(url, onDataSent);
#endif
#ifdef useThingSpeak
	url = ThingSpeakHost;
	url += "/update?key=";
	url += writeAPI;
	url += "&field1=";
	url += cpm;
	url += "&field2=";
	url += dose;
	url += "&field3=";
	url += WifiStation.getRssi();
//	url += "&created_at=";
//	url += SystemClock.now(eTZ_UTC).toISO8601();
	thingSpeak.downloadString(url, onDataSent);
#endif
}

