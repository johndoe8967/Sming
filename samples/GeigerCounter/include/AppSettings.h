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
	unsigned int duty = 0;
	unsigned int measureTime = 60;

	void load()
	{
		debugf("Appsettings load\r\n");
		DynamicJsonBuffer jsonBuffer;
		if (exist())
		{
			debugf("Appsettings exist\r\n");
			int size = fileGetSize(APP_SETTINGS_FILE);
			char* jsonString = new char[size + 1];
			fileGetContent(APP_SETTINGS_FILE, jsonString, size + 1);
			JsonObject& root = jsonBuffer.parseObject(jsonString);

			JsonObject& geiger = root["geiger"];
			measureTime = geiger["measureTime"];
			duty = geiger["duty"];

			debugf("Appsettings time: %d\r\n",measureTime);
			debugf("Appsettings duty: %d\r\n",duty);

			delete[] jsonString;
		}
	}

	void save()
	{
		DynamicJsonBuffer jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();

		JsonObject& geiger = jsonBuffer.createObject();
		root["geiger"] = geiger;
		geiger["measureTime"] = measureTime;
		geiger["duty"] = duty;

		debugf("Appsettings time: %d\r\n",measureTime);
		debugf("Appsettings duty: %d\r\n",duty);

		String rootString;
		root.printTo(rootString);
		fileSetContent(APP_SETTINGS_FILE, rootString);
		debugf("file: %s",rootString.c_str());
	}

	bool exist() { return fileExist(APP_SETTINGS_FILE); }
};

static ApplicationSettingsStorage AppSettings;

#endif /* INCLUDE_APPSETTINGS_H_ */