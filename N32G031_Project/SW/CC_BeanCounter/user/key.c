#include <stdint.h>
#include "key.h"
#include "led.h"
#include "n32g031_gpio.h"

uint8_t digitalRead(uint16_t gpio)
{
	uint8_t gpio_level;
	gpio_level = GPIO_ReadInputDataBit(PORT_SW, gpio);
	
	return gpio_level;
}
	
