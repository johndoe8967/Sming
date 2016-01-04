#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <AppSettings.h>
#include "DigitalOutlet.h"
#include "NtpClientDelegateDemo.h"
#include "mqtt.h"

#define OUT_PIN 2 // GPIO2

HttpServer server;
FTPServer ftp;

BssList networks;
String network, password;
Timer connectionTimer;

bool deviceAvailable = false;

Timer wifiStatusTimer;
TimerDelegate wifiStatusWorker;

DigitalOutlet *digOutlet;
Timer outletTimer;
TimerDelegate workOutletTimer;

ntpClientDemo *ntp;

MqttStringSubscriptionCallback mqttCallback;
ApplicationSettingsStorage AppSettings;


void onIndex(HttpRequest &request, HttpResponse &response)
{
	TemplateFileStream *tmpl = new TemplateFileStream("index.html");
	auto &vars = tmpl->variables();
	response.sendTemplate(tmpl); // will be automatically deleted
}


time_t getTimeFromString(String time) {
	int min;
	int hour;

	int pos = time.indexOf(':');
	Serial.print("get Time from String: ");
	Serial.print(time);
	Serial.print(" pos of ':' ");
	Serial.print(pos);

	time_t unixTime;

	if (pos >= 0) {
		Serial.print(" ':' detected ");
		hour = time.substring(0,pos).toInt();
		min = time.substring(pos+1).toInt();
		unixTime = hour*3600+min*60;
		Serial.println (unixTime);
		return unixTime;
	} else {
		Serial.print(" no ':' detected ");
		unixTime = time.toInt();
		Serial.println(unixTime);
		return unixTime;
	}
}

String getStringFromTime(time_t time) {
	String stringTime;
	time /= 60;
	stringTime = String(time/60);
	time %= 60;
	stringTime += ":";
	stringTime += String(time);
	return stringTime;
}

void onSwitch(HttpRequest &request, HttpResponse &response)
{
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		debugf("POST Set Data");
		AppSettings.timeZone = request.getPostParameter("timeZone").toInt();
		SystemClock.setTimeZone(AppSettings.timeZone);
		AppSettings.switchOffTime = getTimeFromString(request.getPostParameter("switchOffTime"));
		AppSettings.switchOnTime = getTimeFromString(request.getPostParameter("switchOnTime"));
		AppSettings.maxOnTime = request.getPostParameter("maxOnTime").toInt();
		if (request.getPostParameter("switch") == "1") {
			digOutlet->changeState(manualSwitch_On);
		} else {
			digOutlet->changeState(manualSwitch_Off);
		}
		Serial.print("Set maxOnTime: ");
		Serial.println(AppSettings.maxOnTime);

		AppSettings.save();
	}

	TemplateFileStream *tmpl = new TemplateFileStream("switch.html");
	auto &vars = tmpl->variables();

	vars["switchon"] = (digOutlet->getState()==SwitchedOn) ? "checked='checked'" : "";
	vars["switchoff"] = (digOutlet->getState()==SwitchedOff) ? "checked='checked'" : "";

	vars["maxOnTime"] = String(AppSettings.maxOnTime);
	vars["remainingTime"] = String(digOutlet->getRemainingTime());
	vars["switchOffTime"] = getStringFromTime(AppSettings.switchOffTime);
	vars["switchOnTime"] = getStringFromTime(AppSettings.switchOnTime);
	vars["timeZone"] = String(AppSettings.timeZone);

	response.sendTemplate(tmpl); // will be automatically deleted
}


void onIpConfig(HttpRequest &request, HttpResponse &response)
{
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		AppSettings.dhcp = request.getPostParameter("dhcp") == "1";
		AppSettings.ip = request.getPostParameter("ip");
		AppSettings.netmask = request.getPostParameter("netmask");
		AppSettings.gateway = request.getPostParameter("gateway");
		debugf("Updating IP settings: %d", AppSettings.ip.isNull());
		AppSettings.save();
	}

	TemplateFileStream *tmpl = new TemplateFileStream("settings.html");
	auto &vars = tmpl->variables();

	bool dhcp = WifiStation.isEnabledDHCP();
	vars["dhcpon"] = dhcp ? "checked='checked'" : "";
	vars["dhcpoff"] = !dhcp ? "checked='checked'" : "";

	if (!WifiStation.getIP().isNull())
	{
		vars["ip"] = WifiStation.getIP().toString();
		vars["netmask"] = WifiStation.getNetworkMask().toString();
		vars["gateway"] = WifiStation.getNetworkGateway().toString();
	}
	else
	{
		vars["ip"] = "192.168.1.77";
		vars["netmask"] = "255.255.255.0";
		vars["gateway"] = "192.168.1.1";
	}

	response.sendTemplate(tmpl); // will be automatically deleted
}

void onFile(HttpRequest &request, HttpResponse &response)
{
	String file = request.getPath();
	if (file[0] == '/')
		file = file.substring(1);

	if (file[0] == '.')
		response.forbidden();
	else
	{
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

void onAjaxNetworkList(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	json["status"] = (bool)true;

	bool connected = WifiStation.isConnected();
	json["connected"] = connected;
	if (connected)
	{
		// Copy full string to JSON buffer memory
		json["network"]= WifiStation.getSSID();
	}

	JsonArray& netlist = json.createNestedArray("available");
	for (int i = 0; i < networks.count(); i++)
	{
		if (networks[i].hidden) continue;
		JsonObject &item = netlist.createNestedObject();
		item["id"] = (int)networks[i].getHashId();
		// Copy full string to JSON buffer memory
		item["title"] = networks[i].ssid;
		item["signal"] = networks[i].rssi;
		item["encryption"] = networks[i].getAuthorizationMethodName();
	}

	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}

void makeConnection()
{
	WifiStation.enable(true);
	WifiStation.config(network, password);

	AppSettings.ssid = network;
	AppSettings.password = password;
	AppSettings.save();

	network = ""; // task completed
}

void onAjaxConnect(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	String curNet = request.getPostParameter("network");
	String curPass = request.getPostParameter("password");

	bool updating = curNet.length() > 0 && (WifiStation.getSSID() != curNet || WifiStation.getPassword() != curPass);
	bool connectingNow = WifiStation.getConnectionStatus() == eSCS_Connecting || network.length() > 0;

	if (updating && connectingNow)
	{
		debugf("wrong action: %s %s, (updating: %d, connectingNow: %d)", network.c_str(), password.c_str(), updating, connectingNow);
		json["status"] = (bool)false;
		json["connected"] = (bool)false;
	}
	else
	{
		json["status"] = (bool)true;
		if (updating)
		{
			network = curNet;
			password = curPass;
			debugf("CONNECT TO: %s %s", network.c_str(), password.c_str());
			json["connected"] = false;
			connectionTimer.initializeMs(1200, makeConnection).startOnce();
		}
		else
		{
			json["connected"] = WifiStation.isConnected();
			debugf("Network already selected. Current status: %s", WifiStation.getConnectionStatusName());
		}
	}

	if (!updating && !connectingNow && WifiStation.isConnectionFailed())
		json["error"] = WifiStation.getConnectionStatusName();

	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}

void startWebServer()
{
	server.listen(80);
	server.addPath("/", onIndex);
	server.addPath("/ipconfig", onIpConfig);
	server.addPath("/switch", onSwitch);
	server.addPath("/ajax/get-networks", onAjaxNetworkList);
	server.addPath("/ajax/connect", onAjaxConnect);
	server.setDefaultHandler(onFile);
}

void startFTP()
{
	if (!fileExist("index.html"))
		fileSetContent("index.html", "<h3>Please connect to FTP and upload files from folder 'web/build' (details in code)</h3>");

	// Start FTP server
	ftp.listen(21);
	ftp.addUser("me", "123"); // FTP account
}


void outletWorker() {
	static bool prevValue;
	bool actValue;

	digOutlet->outletWorker();
	actValue = digOutlet->getState();

/*	Serial.print("worker ActValue: ");
	Serial.print(actValue);
	Serial.print(" PrevValue: ");
	Serial.println(prevValue);*/

	if (prevValue != actValue) {
//		Serial.println("Publish");
		prevValue = actValue;
		publishMqttMessage(actValue);
	}
}

// Callback for messages, arrived from MQTT server
void onMqttMessageReceived(String topic, String message)
{
	if (topic.compareTo(actorTopic)==0) {
		debugf("got actor message");

		DynamicJsonBuffer jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(message);
		String state = root["state"].asString();
		if (state.toInt() == 0) {
			digOutlet->changeState(mqttSwitch_Off);
			debugf("Off");
		} else {
			digOutlet->changeState(mqttSwitch_On);
			debugf("On");
		}
	}
}


// Will be called when system initialization was completed
void startServers()
{
	startFTP();
	startWebServer();
	digOutlet = new DigitalOutlet(OUT_PIN);
	workOutletTimer=outletWorker;
	outletTimer.initializeMs(1000,workOutletTimer).start();

	mqttCallback = onMqttMessageReceived;
	startMqttClient(mqttCallback);
}

void networkScanCompleted(bool succeeded, BssList list)
{
	if (succeeded)
	{
		for (int i = 0; i < list.count(); i++)
			if (!list[i].hidden && list[i].ssid.length() > 0)
				networks.add(list[i]);
	}
	networks.sort([](const BssInfo& a, const BssInfo& b){ return b.rssi - a.rssi; } );
}


void wifiAccessibilityWorker() {
	if (!WifiStation.isConnected()) {
		// Start AP for configuration
		if (!WifiAccessPoint.isEnabled()) {
			WifiAccessPoint.enable(true);
			WifiAccessPoint.config("Sming Configuration", "", AUTH_OPEN);
			WifiStation.enable(false);
		}
	} else {
		// Stop AP for configuration
		if (WifiAccessPoint.isEnabled()) {
			WifiAccessPoint.enable(false);
			WifiStation.enable(false);
		}
	}
}


// Will be called when WiFi station was connected to AP
void connectOk()
{
	if(ntp) {
		delete ntp;
	}
	ntp = new ntpClientDemo();
}
// Will be called when WiFi station timeout was reached
void connectFail()
{
	debugf("I'm NOT CONNECTED!");
	WifiStation.waitConnection(connectOk, 10, connectFail); // Repeat and check again
}

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial
	AppSettings.load();

	WifiStation.enable(true);
	WifiAccessPoint.enable(false);

	if (AppSettings.exist())
	{
		WifiStation.config(AppSettings.ssid, AppSettings.password);
		if (!AppSettings.dhcp && !AppSettings.ip.isNull())
			WifiStation.setIP(AppSettings.ip, AppSettings.netmask, AppSettings.gateway);
	}

	WifiStation.startScan(networkScanCompleted);

	wifiStatusWorker = wifiAccessibilityWorker;
	wifiStatusTimer.initializeMs(10000,wifiStatusWorker).start();

	// set timezone hourly difference to UTC
	SystemClock.setTimeZone(AppSettings.timeZone);


	// Run WEB server on system ready
	System.onReady(startServers);

	// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(connectOk, 30, connectFail); // We recommend 20+ seconds at start
}
