/*****************************************************************************
 * Copyright (c) 2019, Nations Technologies Inc.
 *
 * All rights reserved.
 * ****************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Nations' name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY NATIONS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL NATIONS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ****************************************************************************/

/**
 * @file main.c
 * @author Nations 
 * @version v1.0.0
 *
 * @copyright Copyright (c) 2019, Nations Technologies Inc. All rights reserved.
 */
#include "main.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "delay.h"
#include "led.h"
#include "key.h"
#include "transform.h"
#include "adc.h"
#include "RTT_printf.h"
#include "flash.h"

//**************************** 引脚定义 ****************************
#define STARTBTN GPIO_PIN_0 //开始按钮引脚，PA0
#define SETBTN	 GPIO_PIN_15 //开始按钮引脚，PA15
#define IRSENSOR_A 14 //PB0 IR2
#define IRSENSOR_B 15 //PB1 IR1

//**************************** 模式定义 ****************************
uint8_t device_mode = 0;	//工作模式，0：空闲/停止; 1: 计数；2：复位；3：设置菜单
uint8_t return_mode = 0; //返回模式，返回上一次的模式

//计数模式，0：Inventory Mode,只用一个IR led计数，所以不能区分方向。
//********* 1: Dispense Mode,用两个IR led计数，可以区分方向。
uint8_t countingMode = 0; 

//**************************** 全局变量定义 ************************
char printf_buf[BUFFER_SIZE_UP];
uint8_t digit1 = 0; //第一个数码管要显示的数
uint8_t digit2 = 0; //第二个
uint8_t digit3 = 0; //第三个
uint16_t state_a = 0x01; //IR_a的状态，向前或向后
uint16_t state_b = 0x02; //IR_b的状态
uint16_t state_previous = 0x00;
bool bool_state_previous = 1;
int16_t count = 0; //计数值
int16_t displayBuf = 0; //显示的值，用在translateSegments()
float ratio = 1.0; //编带上的孔与实际零件的比例，一个孔代表几个零件
int scanDelay = 5; //等待时间
byte numPos[10] = {0}; //存储displayBuf的每位数字
byte pitch = 4; //编带上零件的间距，mm

//adc DMA存放数组
uint16_t ADCConvertedValue[2];

//***************************** 结构体定义 **************************
SystemTPDF pitch_mode;

//**************************** Function *****************************
uint16_t analogRead(uint8_t gpio)
{
	if(gpio == IRSENSOR_A)
	{
		ADCConvertedValue[0] = ADC_GetData(ADC_CH_8_PB0);
		RTT_printf("valur0 = %d,\n", ADCConvertedValue[0]);
		return ADCConvertedValue[0];
	}
	else if(gpio == IRSENSOR_B)
	{
		ADCConvertedValue[1] = ADC_GetData(ADC_CH_9_PB1);
		RTT_printf("valur1 = %d,\n", ADCConvertedValue[1]);
		return ADCConvertedValue[1];
	}
}

void TestLed()
{
	GPIO_SetBits(PORT_COP, GPIO_COP);
  while (1)
  {
		GPIO_TogglePin(PORT_SEGIN, GPIO_SEGIN);
		delay_ms(1000);
  }
}

//这是实际处理采集到的元器件数量的函数，更新计数值，有两种模式countingMode,一个IR或两个IR
//return:更新了counting 和 displayBuf
uint16_t adc_a; 
uint16_t adc_b;
uint16_t bool_state_current;
void updateCount(void)
{
  int state_current = 0;
	 
	if (countingMode == 1)
    {
        /**************** Dispense Mode ****************/
        // Fetch the ADC
        adc_a = analogRead(IRSENSOR_A); 
        adc_b = analogRead(IRSENSOR_B);

        // 建立一个死区，防止计数错误
        if (adc_a > 450)
        {
            state_a = 0x01;
        }
        else if (adc_a < 300)
        {
            state_a = 0x00;
        }
        if (adc_b > 450)
        {
            state_b = 0x02;
        }
        else if (adc_b < 300)
        {
            state_b = 0x00;
        }

        state_current = state_a + state_b;

        //state_current != state_previous, 编带移动了, 开始计数
        if (state_current != state_previous)
        {
            // The state transitions for tape moving "up" are: 
            // 00 01 11 10
            // and for a tape moving "down" are: 
            // 00 10 11 01
            // but because of the geometry involved, the "00" 
            // state is very short, we shouldn't rely on detecting 
            // it. So we only increment/decrement on the transition 
            // out of "11" which is our longest state.
            if (state_previous == 0x03)
            {
                switch (state_current)
                {
                case 1:
                    count++;
                    // We don't keep track of which way we're counting
                    // so instead we just count into the negative and 
                    // display the absolute value
                    displayBuf = abs(count) * ratio; // multiply by hole-to-part ratio
                    break;
                case 2:
                    count--;
                    displayBuf = abs(count) * ratio;
                    break;
                }
            }
            state_previous = state_current;
        }

    }
    else
    {
        /**************** Inventory Mode ****************/
        // Fetch the ADC
        adc_a = analogRead(IRSENSOR_A);

        // Create a bool to store the sensor state
//        uint16_t bool_state_current;

        // We need to do the deadzone trick here as well for stability
        if (adc_a > 450)
        {
            bool_state_current = 1;
        }
        else if (adc_a < 300)
        {
            bool_state_current = 0;
        }
        
        // If the state we just measured is different than the last time we were 
        // polled, it means the tape has moved! Time to count!
        if (bool_state_current != bool_state_previous)
        {
            if (bool_state_current == 1)
            {
                count++;
                displayBuf = count * ratio; // multiply by hole-to-part ratio
            }
            bool_state_previous = bool_state_current;
        }
    }
}

//扫描数码管的每个位，更新显示
void updateLED()
{

    digitalWrite(PA_7, bitRead(digit1, 1));
    digitalWrite(PA_6, bitRead(digit1, 2));
    digitalWrite(PA_5, bitRead(digit1, 3));
    digitalWrite(PA_4, bitRead(digit1, 4));
    digitalWrite(PA_3, bitRead(digit1, 6));
    digitalWrite(PA_2, bitRead(digit1, 7));
    digitalWrite(PA_1, bitRead(digit1, 5));
  
    digitNumWrite(DIGIT1_CATHODE, 1);
    if (device_mode == 1)
    {
      updateCount(); //更新计数值
			delay_ms(1);
    }
    else
    {
        delayMicroseconds(scanDelay);
    }
    digitNumWrite(DIGIT1_CATHODE, 0);

    digitalWrite(PA_7, bitRead(digit2, 1));
    digitalWrite(PA_6, bitRead(digit2, 2));
    digitalWrite(PA_5, bitRead(digit2, 3));
    digitalWrite(PA_4, bitRead(digit2, 4));
    digitalWrite(PA_3, bitRead(digit2, 6));
    digitalWrite(PA_2, bitRead(digit2, 7));
    digitalWrite(PA_1, bitRead(digit2, 5));

    digitNumWrite(DIGIT2_CATHODE, 1);
    if (device_mode == 1)
    {
      updateCount();
			delay_ms(1);
    }
    else
    {
        delayMicroseconds(scanDelay);
    }
    digitNumWrite(DIGIT2_CATHODE, 0);

    digitalWrite(PA_7, bitRead(digit3, 1));
    digitalWrite(PA_6, bitRead(digit3, 2));
    digitalWrite(PA_5, bitRead(digit3, 3));
    digitalWrite(PA_4, bitRead(digit3, 4));
    digitalWrite(PA_3, bitRead(digit3, 6));
    digitalWrite(PA_2, bitRead(digit3, 7));
    digitalWrite(PA_1, bitRead(digit3, 5));

    digitNumWrite(DIGIT3_CATHODE, 1);
    if (device_mode == 1)
    {
      updateCount();
			delay_ms(1);
    }
    else
    {
        delayMicroseconds(scanDelay);
    }
    digitNumWrite(DIGIT3_CATHODE, 0);
}

void numPositions(int num)
{
	int pos = 0;
	
//	for (int i = 0; i < 10; i++)
//	{
//			numPos[i] = 0;
//	}
	memset(numPos, 0, sizeof(numPos));
	while (num != 0)
	{
			numPos[pos] = num % 10;
			num = num / 10;
			pos++;
	}
	return;
}

// 数字查表，或数字对应的七段码，eg：segLUT[6]显示6
const byte segLUT[10] = {
    0x77,//0b01110111,
    0x24,//0b00100100,
    0x5D,//0b01011101,
    0x5B,//0b01011011,
    0x3A,//0b00111010,
    0x6B,//0b01101011,
    0x6F,//0b01101111,
    0x52,//0b01010010,
    0x7F,//0b01111111,
    0x7A //0b01111010
};

void translateSegments(int displayBuf)
{
	numPositions(displayBuf);
	digit3 = segLUT[numPos[0]];
	digit2 = segLUT[numPos[1]];
	digit1 = segLUT[numPos[2]];
}

//mode0:IDLE/STOP
//此模式不计数，只显示最后一次计数值，忽略IR中断，会关闭IR供电，省电
void Mode0_Stop(void)
{
	int blinkTimer = 0;
	
	if (digitalRead(STARTBTN) == 0)
    {
        device_mode = 2;
        return_mode = 1;
        return;
    }

		GPIO_ResetBits(PORT_IRO, GPIO_IRO);

    while (device_mode == 0)
    {

        blinkTimer++;

        updateLED();

        if (digitalRead(STARTBTN) == 0)
        {
            delay_ms(500);
            device_mode = 1;
        }

        if (digitalRead(SETBTN) == 0)
        {
            delay_ms(500);
            device_mode = 3;
            return_mode = 0;
        }

        if(blinkTimer > 125)
        {
            translateSegments(displayBuf);
        }
        else
        {
            digit1 = 0;
        }
    }
	
}

//mode1:counting
void Mode1_Counting(void)
{
	if (digitalRead(STARTBTN) == 0)
    {
        device_mode = 2;
        return_mode = 0;
        return;
    }

    digitalWrite(GPIO_IRO, 1); //开红外发射管

    while (device_mode == 1)
    {

        updateLED();

        if (digitalRead(STARTBTN) == 0)
        {
            delay_ms(500);
            device_mode = 0;
        }

        if (digitalRead(SETBTN) == 0)
        {
            delay_ms(500);
            device_mode = 3;
            return_mode = 1;
        }

        translateSegments(displayBuf);
    }

}

//mode2:reset
void Mode2_Reset(void)
{
	int i = 0;
	while (device_mode == 2)
    {
        for (i = 0; i < 100; i++)
        {
            updateLED();
        }
        delay_ms(300);
        if (digitalRead(STARTBTN) == 1)
        {
            count = 0;
            displayBuf = 0;
            device_mode = return_mode;
        }
    }
	
}

//mode3:set menu
void Mode3_SetMenu(void)
{
	uint16_t mode3state = 0; //模式3状态，0：，1: ,2: 
	uint16_t nvmPitch, nvmMode;

    while (device_mode == 3)
    {

        switch(mode3state)
        {
            case 0: //通过器件间距设置比列ratio
                if (digitalRead(STARTBTN) == 0)
                {
                    delay_ms(200);
                    switch (pitch)
                    {
                    case 1:
                        pitch = 2;
                        break;
                    case 2:
                        pitch = 4;
                        break;
                    case 4:
                        pitch = 8;
                        break;
                    case 8:
                        pitch = 12;
                        break;
                    case 12:
                        pitch = 16;
                        break;
                    case 16:
                        pitch = 1;
                        break;
                    }
                    ratio = 4.0 / pitch;
                }
                if (digitalRead(SETBTN) == 0)
                {
                    delay_ms(200);
                    mode3state = 1;
                }
                numPositions(pitch);
                digit3 = segLUT[numPos[0]];
                digit2 = segLUT[numPos[1]];
                digit1 = 0x7C; //0b01111100; // "P" for Pitch
                updateLED();
                break;
            
            case 1:
                if (digitalRead(STARTBTN) == 0)
                {
                    delay_ms(200);
                    switch (countingMode)
                    {
                    case 0:
                        countingMode = 1;
                        count = 0;
                        displayBuf = 0;
                        break;
                    case 1:
                        countingMode = 0;
                        count = 0;
                        displayBuf = 0;
                        break;
                    }
                }
                if (digitalRead(SETBTN) == 0)
                {
                    delay_ms(200);
                    mode3state = 2;
                }
                if(countingMode == 0){
                    digit1 = 0x24; //0b00100100; // I
                    digit2 = 0x0E; //0b00001110; // n
                    digit3 = 0x07; //0b00000111; // v
                }else{
                    digit1 = 0x1F; //0b00011111; // d
                    digit2 = 0x24; //0b00100100; // I
                    digit3 = 0x6B; //0b01101011; // S
                }
                updateLED();
                break;

            case 2:
                // Check EEPROM to see if we've changed the settings
                // and if we have, save the new ones
								nvmPitch = pitch_mode.pitch;
						    nvmMode = pitch_mode.mode;
                if (nvmPitch != pitch)
                {
									pitch_mode.pitch = pitch;
                }
                if (nvmMode != countingMode)
                {
									pitch_mode.mode = countingMode;
                }
								FLASH_Program(FLASH_WRITE_START_ADDR, (char*)&pitch_mode, sizeof(pitch_mode));
                device_mode = return_mode; // Return to whichever mode sent us here
                break;
        }

    }

}

/**
 * @brief  Main program.
 */

int main(void)
{
	GPIO_Init();
	systick_delay_init();
	ADC_Initial();
	
//	TestLed();
	
	loadSettings(); //从flash加载pitch和mode的设置值

  while (1)
  {
//		GPIO_SetBits(PORT_COP, DIGIT1_CATHODE);
		
//		if(!digitalRead(STARTBTN))
//		{
//			GPIO_TogglePin(PORT_SEGIN, GPIO_SEGIN);
//			delay_msMicroseconds(scanDelay);
//		}
//		if(!digitalRead(SETBTN))
//		{
//			GPIO_TogglePin(PORT_SEGIN, GPIO_SEGIN);
//			delay_msMicroseconds(scanDelay);
//		}
		
//		digitalWrite(PA_7, bitRead(segLUT[3], 1));
//    digitalWrite(PA_6, bitRead(segLUT[3], 2));
//    digitalWrite(PA_5, bitRead(segLUT[3], 3));
//    digitalWrite(PA_4, bitRead(segLUT[3], 4));
//    digitalWrite(PA_3, bitRead(segLUT[3], 6));
//    digitalWrite(PA_2, bitRead(segLUT[3], 7));
//    digitalWrite(PA_1, bitRead(segLUT[3], 5));
		
//			GPIO_SetBits(PORT_IRO, GPIO_IRO);
//			RTT_printf("adc0: %d \n,", ADC_GetData(ADC_CH_8_PB0));
//			RTT_printf("adc1: %d \n,", ADC_GetData(ADC_CH_9_PB1));
			
//			RTT_printf("valur0 = %d,\n", ADCConvertedValue[0]);
//			RTT_printf("valur1 = %d,\n", ADCConvertedValue[1]);
	
		switch(device_mode)
		{
			case 0:
				Mode0_Stop();
				break;
			case 1:
				Mode1_Counting();
				break;
			case 2:
				Mode2_Reset();
				break;
			case 3:
				Mode3_SetMenu();
				break;
			default:
        device_mode = 0;
        break;
		}

//digitalWrite(GPIO_IRO, 1); //开红外发射管
//RTT_printf("IR1: %d\n", GPIO_ReadInputDataBit(GPIOB, GPIO_PIN_0));
//delay_ms(1000);
  }
}

/**
 * @brief Assert failed function by user.
 * @param file The name of the call that failed.
 * @param line The source line number of the call that failed.
 */
#ifdef USE_FULL_ASSERT
void assert_failed(const uint8_t* expr, const uint8_t* file, uint32_t line)
{
    while (1)
    {
    }
}
#endif // USE_FULL_ASSERT


