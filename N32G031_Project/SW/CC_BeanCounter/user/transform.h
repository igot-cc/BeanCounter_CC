#ifndef __TRANSFORM_H
#define __TRANSFORM_H

#include <stdint.h>

uint8_t bitRead(uint8_t num, uint8_t nbit);
void digitalWrite(uint16_t gpio_pin, uint8_t level);
void digitNumWrite(uint16_t gpio_pin, uint8_t level);
void delayMicroseconds(int scanDelay);

#endif