#include "OledDisplay.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

OledDisplay::OledDisplay(uint8_t sdaPin, uint8_t sclPin, uint8_t address)
  : _sdaPin(sdaPin),
    _sclPin(sclPin),
    _address(address),
    _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1) {}

bool OledDisplay::begin() {
  Wire.begin(_sdaPin, _sclPin);

  if (!_display.begin(SSD1306_SWITCHCAPVCC, _address)) {
    return false;
  }

  _display.clearDisplay();
  _display.setTextColor(SSD1306_WHITE);
  _display.setTextSize(1);
  _display.setCursor(0, 0);
  _display.println("ATM90E26 METER");
  _display.println("OLED READY");
  _display.display();

  return true;
}

void OledDisplay::showMeter(float voltage, float current, float powerKW, float freq, float pf) {
  float powerW = powerKW * 1000.0;

  _display.clearDisplay();
  _display.setTextColor(SSD1306_WHITE);

  _display.setTextSize(1);
  _display.setCursor(0, 0);
  _display.println("ATM90E26 POWER");

  _display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  _display.setCursor(0, 14);
  _display.print("U: ");
  _display.print(voltage, 1);
  _display.println(" V");

  _display.setCursor(0, 24);
  _display.print("I: ");
  _display.print(current, 2);
  _display.println(" A");

  _display.setCursor(0, 34);
  _display.print("P: ");
  _display.print(powerW, 1);
  _display.println(" W");

  _display.setCursor(0, 44);
  _display.print("F: ");
  _display.print(freq, 2);
  _display.println(" Hz");

  _display.setCursor(0, 54);
  _display.print("PF:");
  _display.print(pf, 3);

  _display.display();
}