#include <stdint.h>
#include "transform.h"
#include "n32g031_gpio.h"
#include "delay.h"

uint8_t bitRead(uint8_t num, uint8_t nbit)
{
	uint8_t bit_hex = 1;
	bit_hex = bit_hex << (nbit-1);
	num = num & bit_hex;
	if(num)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

//控制7段数码管引脚，PA端口
void digitalWrite(uint16_t gpio_pin, uint8_t level)
{
	if(level)
	{
		GPIO_SetBits(GPIOA, gpio_pin);
	}
	else if(!level)
	{
		GPIO_ResetBits(GPIOA, gpio_pin);
	}
}

//写cop引脚，PB端口
void digitNumWrite(uint16_t gpio_pin, uint8_t level)
{
	if(level)
	{
		GPIO_SetBits(GPIOB, gpio_pin);
	}
	else if(!level)
	{
		GPIO_ResetBits(GPIOB, gpio_pin);
	}
}

void delayMicroseconds(int scanDelay)
{
	delay_ms(scanDelay);
}
