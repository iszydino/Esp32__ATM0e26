#ifndef ATM90E26_UART_H
#define ATM90E26_UART_H

#include <Arduino.h>

class ATM90E26_UART {
public:
  ATM90E26_UART(HardwareSerial &serial, int rxPin, int txPin, int csPin, int sclkPin);

  void begin(uint32_t baud = 9600);

  bool readRegister(uint8_t reg, uint16_t &value);
  bool writeRegister(uint8_t reg, uint16_t data);

  void init();

  float readVoltage();
  float readCurrent();
  float readPower();
  float readFrequency();
  float readPowerFactor();

  uint16_t readSysStatus();
  uint16_t readEnStatus();
  uint16_t readLastData();

private:
  HardwareSerial &_serial;

  int _rxPin;
  int _txPin;
  int _csPin;
  int _sclkPin;

  int16_t toSigned16(uint16_t value);
};

#endif