#include <Arduino.h>

#include "ATM90E26_UART.h"
#include "OledDisplay.h"
#include "MqttNodeRed.h"

// ================= PIN ATM90E26 UART =================
#define ATM_RX   19   // ESP32 RX <- ATM90E26 SDO/UTX
#define ATM_TX   23   // ESP32 TX -> ATM90E26 SDI/URX
#define ATM_CS   5
#define ATM_SCLK 18

// ================= PIN OLED =================
#define OLED_SDA  21
#define OLED_SCL  22
#define OLED_ADDR 0x3C

// ================= WIFI + MQTT =================
#define WIFI_SSID "KimHoang"
#define WIFI_PASS "123456789"

#define MQTT_SERVER "192.168.137.1"
#define MQTT_PORT   1883
#define MQTT_TOPIC  "meter/atm90e26/data"

// ================= CALIBRATION =================
// Chỉnh theo công thức:
// VOLTAGE_SCALE = điện áp thực / điện áp raw
// CURRENT_SCALE = dòng thực / dòng raw sau khi trừ offset
#define VOLTAGE_SCALE    0.855
#define CURRENT_SCALE    1.0000
#define CURRENT_OFFSET_A 0.070

// ================= OBJECT =================
HardwareSerial ATMSerial(2);

ATM90E26_UART atm(ATMSerial, ATM_RX, ATM_TX, ATM_CS, ATM_SCLK);
OledDisplay oled(OLED_SDA, OLED_SCL, OLED_ADDR);
MqttNodeRed nodeRed(WIFI_SSID, WIFI_PASS, MQTT_SERVER, MQTT_PORT, MQTT_TOPIC);

// ================= TIMER =================
unsigned long lastReadMs = 0;
const unsigned long READ_INTERVAL_MS = 1000;

// ================= PRINT =================
void printTerminal(float voltageRaw, float voltageCal,
                   float currentRaw, float currentCal,
                   float powerKW, float freqHz, float pf) {
  Serial.println("----------------------------");

  Serial.print("Voltage raw: ");
  Serial.print(voltageRaw, 2);
  Serial.print(" V | calibrated: ");
  Serial.print(voltageCal, 2);
  Serial.println(" V");

  Serial.print("Current raw: ");
  Serial.print(currentRaw, 3);
  Serial.print(" A | calibrated: ");
  Serial.print(currentCal, 3);
  Serial.println(" A");

  Serial.print("Power calculated: ");
  Serial.print(powerKW, 3);
  Serial.println(" kW");

  Serial.print("Freq: ");
  Serial.print(freqHz, 2);
  Serial.println(" Hz");

  Serial.print("PF: ");
  Serial.println(pf, 3);

  Serial.print("SysStatus: 0x");
  Serial.println(atm.readSysStatus(), HEX);

  Serial.print("EnStatus : 0x");
  Serial.println(atm.readEnStatus(), HEX);
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("ATM90E26 + OLED + NODE-RED START");

  atm.begin(9600);

  if (!oled.begin()) {
    Serial.println("OLED init failed");
  } else {
    Serial.println("OLED OK");
  }

  nodeRed.begin();
}

// ================= LOOP =================
void loop() {
  nodeRed.loop();

  unsigned long now = millis();

  if (now - lastReadMs >= READ_INTERVAL_MS) {
    lastReadMs = now;

    float voltageRaw = atm.readVoltage();
    float currentRaw = atm.readCurrent();
    float freqHz     = atm.readFrequency();
    float pf         = atm.readPowerFactor();

    if (isnan(voltageRaw)) voltageRaw = 0.0;
    if (isnan(currentRaw)) currentRaw = 0.0;
    if (isnan(freqHz))     freqHz = 0.0;
    if (isnan(pf))         pf = 0.0;

    float voltageCal = voltageRaw * VOLTAGE_SCALE;

    float currentCal = currentRaw * CURRENT_SCALE;
    currentCal -= CURRENT_OFFSET_A;
    if (currentCal < 0.10) currentCal = 0.0;

    float powerKW = voltageCal * currentCal * pf / 1000.0;

    printTerminal(
      voltageRaw,
      voltageCal,
      currentRaw,
      currentCal,
      powerKW,
      freqHz,
      pf
    );

    oled.showMeter(voltageCal, currentCal, powerKW, freqHz, pf);

    nodeRed.publishMeter(voltageCal, currentCal, powerKW, freqHz, pf);
  }
}