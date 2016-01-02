#include "mqtt.h"
// Forward declarations
void startMqttClient();

Timer procTimer;

// MQTT client
// For quickly check you can use: http://www.hivemq.com/demos/websocket-client/ (Connection= test.mosquitto.org:8080)
MqttClient *mqtt;

void restartMqttClient()
{
//	if(!mqtt->setWill("last/will","The connection from this device is lost:(", 1, true)) {
//		debugf("Unable to set the last will and testament. Most probably there is not enough memory on the device.");
//	}
	mqtt->connect("esp8266");
	mqtt->subscribe("/a/a9c96ed3-6c55-49de-b5d6-252b641a0c08/p/6305/d/78217324-f049-43ed-be9c-e2b3474f17ca/actuator/DigitalOutlet/state");

}

// Publish our message
void publishMqttMessage(bool value)
{
	if (mqtt->getConnectionState() != eTCS_Connected)
		restartMqttClient(); // Auto reconnect

	Serial.println("Let's publish message now!");
	String data = "{\"value\":0}";
	mqtt->publish("/a/a9c96ed3-6c55-49de-b5d6-252b641a0c08/p/6305/d/78217324-f049-43ed-be9c-e2b3474f17ca/sensor/Door/data", data); // or publishWithQoS
}


// Run MQTT client
void startMqttClient(MqttStringSubscriptionCallback mqttCallback)
{
	mqtt = new MqttClient("mqtt.devicehub.net", 1883, mqttCallback);
	restartMqttClient();
}
