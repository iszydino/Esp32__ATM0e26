#ifndef MQTT_NODE_RED_H
#define MQTT_NODE_RED_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

class MqttNodeRed {
public:
  MqttNodeRed(
    const char* ssid,
    const char* password,
    const char* mqttServer,
    uint16_t mqttPort,
    const char* topic
  );

  void begin();
  void loop();

  bool isConnected();
  void publishMeter(float voltage, float current, float powerKW, float freq, float pf);

private:
  const char* _ssid;
  const char* _password;
  const char* _mqttServer;
  uint16_t _mqttPort;
  const char* _topic;

  WiFiClient _wifiClient;
  PubSubClient _mqttClient;

  void connectWiFi();
  void connectMQTT();
};

#endif