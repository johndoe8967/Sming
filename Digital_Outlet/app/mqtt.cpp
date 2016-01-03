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
	mqtt->subscribe(actorTopic);

}

// Publish our message
void publishMqttMessage(bool value)
{
	if (mqtt->getConnectionState() != eTCS_Connected)
		restartMqttClient(); // Auto reconnect

	Serial.println("Let's publish message now!");
	String data = "{\"value\":";
	data += String(value);
	data += "}";
	mqtt->publish(sensorTopic, data); // or publishWithQoS
}


// Run MQTT client
void startMqttClient(MqttStringSubscriptionCallback mqttCallback)
{
	mqtt = new MqttClient("mqtt.devicehub.net", 1883, mqttCallback);
	restartMqttClient();
}
