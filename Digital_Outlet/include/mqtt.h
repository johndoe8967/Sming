/*
 * mqtt.h
 *
 *  Created on: 02.01.2016
 *      Author: johndoe
 */

#ifndef INCLUDE_MQTT_H_
#define INCLUDE_MQTT_H_
#include <SmingCore/SmingCore.h>

extern void startMqttClient(MqttStringSubscriptionCallback mqttCallback);
extern void publishMqttMessage(bool value);

#endif /* INCLUDE_MQTT_H_ */
