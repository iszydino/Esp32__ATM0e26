#include "MqttNodeRed.h"

MqttNodeRed::MqttNodeRed(
  const char* ssid,
  const char* password,
  const char* mqttServer,
  uint16_t mqttPort,
  const char* topic
)
  : _ssid(ssid),
    _password(password),
    _mqttServer(mqttServer),
    _mqttPort(mqttPort),
    _topic(topic),
    _mqttClient(_wifiClient) {}

void MqttNodeRed::begin() {
  connectWiFi();
  _mqttClient.setServer(_mqttServer, _mqttPort);
  connectMQTT();
}

void MqttNodeRed::loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!_mqttClient.connected()) {
    connectMQTT();
  }

  _mqttClient.loop();
}

bool MqttNodeRed::isConnected() {
  return WiFi.status() == WL_CONNECTED && _mqttClient.connected();
}

void MqttNodeRed::connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print("Connecting WiFi: ");
  Serial.println(_ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(_ssid, _password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("WiFi connected, IP: ");
  Serial.println(WiFi.localIP());
}

void MqttNodeRed::connectMQTT() {
  while (!_mqttClient.connected()) {
    String clientId = "ESP32_ATM90E26_";
    clientId += String((uint32_t)ESP.getEfuseMac(), HEX);

    Serial.print("Connecting MQTT... ");

    if (_mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.println(_mqttClient.state());
      delay(2000);
    }
  }
}

void MqttNodeRed::publishMeter(float voltage, float current, float powerKW, float freq, float pf) {
  loop();

  char payload[256];

  snprintf(
    payload,
    sizeof(payload),
    "{\"voltage\":%.2f,\"current\":%.3f,\"power_kw\":%.3f,\"frequency\":%.2f,\"pf\":%.3f}",
    voltage,
    current,
    powerKW,
    freq,
    pf
  );

  bool ok = _mqttClient.publish(_topic, payload);

  Serial.print("MQTT publish ");
  Serial.print(ok ? "OK: " : "FAILED: ");
  Serial.println(payload);
}