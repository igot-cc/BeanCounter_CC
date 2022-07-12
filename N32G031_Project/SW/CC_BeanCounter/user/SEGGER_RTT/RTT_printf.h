#ifndef __RTT_PRINTF_H
#define __RTT_PRINTF_H

#include "SEGGER_RTT.h"
#include <stdint.h>
#include <stdio.h>

extern uint8_t print_type;

extern char printf_buf[];

#define	RTT_printf(...)					SEGGER_RTT_Write(0, printf_buf, sprintf(printf_buf, __VA_ARGS__))
//#define	RTT_printf(...)					SEGGER_RTT_printf(0,__VA_ARGS__)

#define	RTT_printf_red(...)			SEGGER_RTT_printf(0,"%s",RTT_CTRL_TEXT_BRIGHT_RED); RTT_printf(__VA_ARGS__)
#define	RTT_printf_green(...)		SEGGER_RTT_printf(0,"%s",RTT_CTRL_TEXT_BRIGHT_GREEN); RTT_printf(__VA_ARGS__)
#define	RTT_printf_yellow(...)		SEGGER_RTT_printf(0,"%s",RTT_CTRL_TEXT_BRIGHT_YELLOW); RTT_printf(__VA_ARGS__)
#define	RTT_printf_white(...)		SEGGER_RTT_printf(0,"%s",RTT_CTRL_TEXT_BRIGHT_WHITE); RTT_printf(__VA_ARGS__)

#endif