#ifndef __LED_H
#define __LED_H

#include "main.h"
#include "stdio.h"

//PORT define
#define PORT_SEGIN	GPIOA
#define PORT_COP  	GPIOB
#define PORT_IR  		GPIOB
#define PORT_SW  		GPIOA
#define PORT_IRO  		GPIOA

//gpio pin define
#define GPIO_SEGIN	GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7
#define GPIO_COP		GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_7
#define GPIO_IR			GPIO_PIN_0 | GPIO_PIN_1
#define GPIO_SW			GPIO_PIN_0 | GPIO_PIN_15
#define GPIO_IRO		GPIO_PIN_8

//LED引脚定义
#define PA_7 GPIO_PIN_1
#define PA_6 GPIO_PIN_2
#define PA_5 GPIO_PIN_3
#define PA_4 GPIO_PIN_4
#define PA_3 GPIO_PIN_6
#define PA_2 GPIO_PIN_7
#define PA_1 GPIO_PIN_5

//第几个数码管定义
#define DIGIT1_CATHODE GPIO_PIN_7	//COP1，PB7
#define DIGIT2_CATHODE GPIO_PIN_6	//COP2，PB6
#define DIGIT3_CATHODE GPIO_PIN_4	//COP3，PB4

void GPIO_Init(void);

#endif