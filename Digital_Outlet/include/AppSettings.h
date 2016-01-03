/*
 * AppSettings.h
 *
 *  Created on: 13 ��� 2015 �.
 *      Author: Anakod
 */

#include <SmingCore/SmingCore.h>

#ifndef INCLUDE_APPSETTINGS_H_
#define INCLUDE_APPSETTINGS_H_

#define APP_SETTINGS_FILE ".settings.conf" // leading point for security reasons :)

struct ApplicationSettingsStorage
{
	String ssid;
	String password;

	bool dhcp = true;

	IPAddress ip;
	IPAddress netmask;
	IPAddress gateway;
	uint32 maxOnTime;
	time_t switchOffTime;
	time_t switchOnTime;
	uint8 timeZone = 2;


	void load()
	{
		DynamicJsonBuffer jsonBuffer;
		if (exist())
		{
			int size = fileGetSize(APP_SETTINGS_FILE);
			char* jsonString = new char[size + 1];
			fileGetContent(APP_SETTINGS_FILE, jsonString, size + 1);
			JsonObject& root = jsonBuffer.parseObject(jsonString);

			JsonObject& network = root["network"];
			JsonObject& digOutlet = root["digOutlet"];

			ssid = network["ssid"].asString();
			password = network["password"].asString();

			dhcp = network["dhcp"];

			ip = network["ip"].asString();
			netmask = network["netmask"].asString();
			gateway = network["gateway"].asString();

			String stringValue;
			stringValue = digOutlet["maxOnTime"].asString();
			maxOnTime = stringValue.toInt();

			stringValue = digOutlet["switchOnTime"].asString();
			switchOnTime = stringValue.toInt();

			stringValue = digOutlet["switchOffTime"].asString();
			switchOffTime = stringValue.toInt();

			stringValue = digOutlet["timeZone"].asString();
			timeZone = stringValue.toInt();

			delete[] jsonString;
		}
	}

	void save()
	{
		DynamicJsonBuffer jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();

		JsonObject& network = jsonBuffer.createObject();
		JsonObject& digOutlet = jsonBuffer.createObject();
		root["network"] = network;
		root["digOutlet"] = digOutlet;
		network["ssid"] = ssid.c_str();
		network["password"] = password.c_str();

		network["dhcp"] = dhcp;

		// Make copy by value for temporary string objects
		network["ip"] = ip.toString();
		network["netmask"] = netmask.toString();
		network["gateway"] = gateway.toString();

		digOutlet["maxOnTime"] = String(maxOnTime);
		digOutlet["switchOnTime"] = String(switchOnTime);
		digOutlet["switchOffTime"] = String(switchOffTime);
		digOutlet["timeZone"] = String(timeZone);

		//TODO: add direct file stream writing
		String rootString;
		root.printTo(rootString);
		fileSetContent(APP_SETTINGS_FILE, rootString);
	}

	bool exist() { return fileExist(APP_SETTINGS_FILE); }
};

extern ApplicationSettingsStorage AppSettings;

#endif /* INCLUDE_APPSETTINGS_H_ */
