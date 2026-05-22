#include "ATM90E26_UART.h"

// ================= Register =================
#define REG_SOFTRESET  0x00
#define REG_SYSSTATUS  0x01
#define REG_FUNCEN     0x02
#define REG_SAGTH      0x03
#define REG_LASTDATA   0x06

#define REG_CALSTART   0x20
#define REG_PLCONSTH   0x21
#define REG_PLCONSTL   0x22
#define REG_LGAIN      0x23
#define REG_LPHI       0x24
#define REG_PSTARTTH   0x27
#define REG_PNOLTH     0x28
#define REG_QSTARTTH   0x29
#define REG_QNOLTH     0x2A
#define REG_MMODE      0x2B
#define REG_CS1        0x2C

#define REG_ADJSTART   0x30
#define REG_UGAIN      0x31
#define REG_IGAINL     0x32
#define REG_UOFFSET    0x34
#define REG_IOFFSETL   0x35
#define REG_POFFSETL   0x37
#define REG_QOFFSETL   0x38
#define REG_CS2        0x3B

#define REG_ENSTATUS   0x46
#define REG_IRMS       0x48
#define REG_URMS       0x49
#define REG_PMEAN      0x4A
#define REG_FREQ       0x4C
#define REG_POWERF     0x4D

ATM90E26_UART::ATM90E26_UART(
  HardwareSerial &serial,
  int rxPin,
  int txPin,
  int csPin,
  int sclkPin
)
  : _serial(serial),
    _rxPin(rxPin),
    _txPin(txPin),
    _csPin(csPin),
    _sclkPin(sclkPin) {}

void ATM90E26_UART::begin(uint32_t baud) {
  pinMode(_csPin, OUTPUT);
  digitalWrite(_csPin, HIGH);

  pinMode(_sclkPin, OUTPUT);
  digitalWrite(_sclkPin, LOW);

  _serial.begin(baud, SERIAL_8N1, _rxPin, _txPin);
  delay(100);

  init();
}

bool ATM90E26_UART::readRegister(uint8_t reg, uint16_t &value) {
  while (_serial.available()) _serial.read();

  uint8_t addr = 0x80 | (reg & 0x7F);

  _serial.write(0xFE);
  _serial.write(addr);
  _serial.write(addr);
  _serial.flush();

  uint32_t t0 = millis();
  while (_serial.available() < 3 && millis() - t0 < 120) {
    delay(1);
  }

  if (_serial.available() < 3) return false;

  uint8_t msb = _serial.read();
  uint8_t lsb = _serial.read();
  uint8_t rxchk = _serial.read();

  if ((uint8_t)(msb + lsb) != rxchk) {
    return false;
  }

  value = ((uint16_t)msb << 8) | lsb;
  return true;
}

bool ATM90E26_UART::writeRegister(uint8_t reg, uint16_t data) {
  while (_serial.available()) _serial.read();

  uint8_t addr = reg & 0x7F;
  uint8_t msb = data >> 8;
  uint8_t lsb = data & 0xFF;
  uint8_t chk = addr + msb + lsb;

  _serial.write(0xFE);
  _serial.write(addr);
  _serial.write(msb);
  _serial.write(lsb);
  _serial.write(chk);
  _serial.flush();

  delay(30);
  return true;
}

void ATM90E26_UART::init() {
  // Có thể comment SoftReset nếu bạn thấy reset làm URMS về 0.
  writeRegister(REG_SOFTRESET, 0x789A);
  delay(300);

  writeRegister(REG_FUNCEN, 0x0030);
  writeRegister(REG_SAGTH,  0x1F2F);

  writeRegister(REG_CALSTART, 0x5678);
  writeRegister(REG_PLCONSTH, 0x00B9);
  writeRegister(REG_PLCONSTL, 0xC1F3);
  writeRegister(REG_LGAIN,    0x1D39);
  writeRegister(REG_LPHI,     0x0000);
  writeRegister(REG_PSTARTTH, 0x08BD);
  writeRegister(REG_PNOLTH,   0x0000);
  writeRegister(REG_QSTARTTH, 0x0AEC);
  writeRegister(REG_QNOLTH,   0x0000);
  writeRegister(REG_MMODE,    0x9422);
  writeRegister(REG_CS1,      0x4A34);

  writeRegister(REG_ADJSTART, 0x5678);

  // Giá trị từng đọc được điện áp với mạch của bạn
  writeRegister(REG_UGAIN,    0x1000);
  writeRegister(REG_IGAINL,   0x6E49);

  writeRegister(REG_UOFFSET,  0x0000);
  writeRegister(REG_IOFFSETL, 0x0000);
  writeRegister(REG_POFFSETL, 0x0000);
  writeRegister(REG_QOFFSETL, 0x0000);
  writeRegister(REG_CS2,      0xD294);

  writeRegister(REG_CALSTART, 0x8765);
  writeRegister(REG_ADJSTART, 0x8765);

  delay(300);
}

int16_t ATM90E26_UART::toSigned16(uint16_t value) {
  return (int16_t)value;
}

float ATM90E26_UART::readVoltage() {
  uint16_t raw;
  if (!readRegister(REG_URMS, raw)) return NAN;
  return raw / 100.0;
}

float ATM90E26_UART::readCurrent() {
  uint16_t raw;
  if (!readRegister(REG_IRMS, raw)) return NAN;
  return raw / 1000.0;
}

float ATM90E26_UART::readPower() {
  uint16_t raw;
  if (!readRegister(REG_PMEAN, raw)) return NAN;
  return toSigned16(raw) / 1000.0;
}

float ATM90E26_UART::readFrequency() {
  uint16_t raw;
  if (!readRegister(REG_FREQ, raw)) return NAN;
  return raw / 100.0;
}

float ATM90E26_UART::readPowerFactor() {
  uint16_t raw;
  if (!readRegister(REG_POWERF, raw)) return NAN;
  return toSigned16(raw) / 1000.0;
}

uint16_t ATM90E26_UART::readSysStatus() {
  uint16_t raw = 0xFFFF;
  if (!readRegister(REG_SYSSTATUS, raw)) return 0xFFFF;
  return raw;
}

uint16_t ATM90E26_UART::readEnStatus() {
  uint16_t raw = 0xFFFF;
  if (!readRegister(REG_ENSTATUS, raw)) return 0xFFFF;
  return raw;
}

uint16_t ATM90E26_UART::readLastData() {
  uint16_t raw = 0xFFFF;
  if (!readRegister(REG_LASTDATA, raw)) return 0xFFFF;
  return raw;
}