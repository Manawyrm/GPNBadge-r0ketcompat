#include <stdint.h>
#include <Arduino.h>
#include <Wire.h>
#include "gpn-input.hpp"

uint8_t portExpanderConfig = 0;

int getInputRaw() {
  uint16_t adc = analogRead(0);

  if (adc < UP + OFFSET && adc > UP - OFFSET)             return BTN_UP;
  else if (adc < DOWN + OFFSET && adc > DOWN - OFFSET)    return BTN_DOWN;
  else if (adc < RIGHT + OFFSET && adc > RIGHT - OFFSET)  return BTN_RIGHT;
  else if (adc < LEFT + OFFSET && adc > LEFT - OFFSET)    return BTN_LEFT;
  if (digitalRead(GPIO_BOOT) == 1) return BTN_ENTER;
  return 0;
}
void setGPIO(byte channel, boolean level) {
  bitWrite(portExpanderConfig, channel, level);
  Wire.beginTransmission(I2C_PCA);
  Wire.write(portExpanderConfig);
  Wire.endTransmission();
}

void setAnalogMUX(byte channel) {
  portExpanderConfig = portExpanderConfig & 0b11111000;
  portExpanderConfig = portExpanderConfig | channel;
  Wire.beginTransmission(I2C_PCA);
  Wire.write(portExpanderConfig);
  Wire.endTransmission();
}
