/*
 * mqtt.h
 *
 *  Created on: 02.01.2016
 *      Author: johndoe
 */

#ifndef INCLUDE_MQTT_H_
#define INCLUDE_MQTT_H_
#include <SmingCore/SmingCore.h>

const char actorTopic[] = "/a/a9c96ed3-6c55-49de-b5d6-252b641a0c08/p/6305/d/78217324-f049-43ed-be9c-e2b3474f17ca/actuator/DigitalOutlet/state";
const char sensorTopic[]= "/a/a9c96ed3-6c55-49de-b5d6-252b641a0c08/p/6305/d/78217324-f049-43ed-be9c-e2b3474f17ca/sensor/Door/data";


extern void startMqttClient(MqttStringSubscriptionCallback mqttCallback);
extern void publishMqttMessage(bool value);

#endif /* INCLUDE_MQTT_H_ */
