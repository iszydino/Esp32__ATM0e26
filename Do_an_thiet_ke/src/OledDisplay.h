#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class OledDisplay {
public:
  OledDisplay(uint8_t sdaPin, uint8_t sclPin, uint8_t address);

  bool begin();
  void showMeter(float voltage, float current, float powerKW, float freq, float pf);

private:
  uint8_t _sdaPin;
  uint8_t _sclPin;
  uint8_t _address;

  Adafruit_SSD1306 _display;
};

#endif