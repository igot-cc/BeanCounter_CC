#ifndef __FLASH_H
#define __FLASH_H

#include "main.h"

// EEPROM Addresses
#define FLASH_WRITE_START_ADDR (0x08000000 + FLASH_WRPR_Pages120to127)  //((uint32_t)0x08008000)

typedef struct{
	uint16_t pitch;
	uint16_t mode;
}SystemTPDF;

//erase flash pages
void FLASH_Erase_Pages(uint32_t start_addr, uint8_t page_num);
//program flash word by word from start_addr
void FLASH_Program(uint32_t start_addr, char* data, uint16_t len);
void loadSettings();

#endif