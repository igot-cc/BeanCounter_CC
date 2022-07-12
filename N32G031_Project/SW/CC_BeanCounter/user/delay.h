#ifndef __DELAY_H
#define __DELAY_H

#include "N32G031.h"

void systick_delay_init(void);
void delay_us(u32 nus);
void delay_ms(u16 nms);

#endif
