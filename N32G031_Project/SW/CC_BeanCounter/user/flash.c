#include <stdint.h>
#include <string.h>
#include "n32g031_flash.h"
#include "flash.h"

extern SystemTPDF pitch_mode;
extern byte pitch; //编带上零件的间距，mm
extern float ratio; //编带上的孔与实际零件的比例，一个孔代表几个零件
extern uint8_t countingMode; 

// Check EEPROM for user settings and load them
void loadSettings()
{
    uint16_t nvmPitch = 0, nvmMode = 0;
//    EEPROM.get(EEPROM_ADDR_PITCH, nvmPitch);
//    EEPROM.get(EEPROM_ADDR_MODE, nvmMode);
		memcpy(&pitch_mode, (char*)FLASH_WRITE_START_ADDR, sizeof(pitch_mode));

    // We're hypervigilant about writing a valid value to pitch,
    // This way even if EEPROM gets corrupted by low battery,
    // At least the pitch ends up in a known and user-correctable
    // state
    if (nvmPitch == 1 || nvmPitch == 2 || nvmPitch == 4 || nvmPitch == 8 || nvmPitch == 12 || nvmPitch == 16)
    {
        pitch = nvmPitch;
        ratio = 4.0 / pitch;
    }

    if (nvmMode == 0x00)
    {
        countingMode = 0;
    }
    else if (nvmMode == 0x01)
    {
        countingMode = 1;
    }
    return;
}

/*!
    \brief      erase fmc pages from FMC_WRITE_START_ADDR to FMC_WRITE_END_ADDR
    \param[in]  start address & page number
    \param[out] none
    \retval     none
*/
void FLASH_Erase_Pages(uint32_t start_addr, uint8_t page_num)
{
    uint8_t i;

    /* unlock the flash program/erase controller */
    FLASH_Unlock();

    /* clear all pending flags */
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR);
    
    /* erase the flash pages */
    for(i = 0U; i < page_num; i++){
        FLASH_EraseOnePage(FLASH_WRITE_START_ADDR);
        FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR);
    }

    /* lock the main FLASH after the erase operation */
    FLASH_Lock();
}


/*!
    \brief      program fmc word by word from FMC_WRITE_START_ADDR to FMC_WRITE_END_ADDR
    \param[in]  start address & data print & data number
    \param[out] none
    \retval     none
*/
void FLASH_Program(uint32_t start_addr, char* data, uint16_t len)
{
	uint32_t i,word_buf;
	
	/* unlock the flash program/erase controller */
	FLASH_Unlock();

	/* program flash */
	for(i=0;i<len;i+=4)
	{
		memcpy(&word_buf, &data[i], 4);
		FLASH_ProgramWord(start_addr, word_buf);
		start_addr += 4;
		FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR);
	}

	/* lock the main FMC after the program operation */
	FLASH_Lock();
}