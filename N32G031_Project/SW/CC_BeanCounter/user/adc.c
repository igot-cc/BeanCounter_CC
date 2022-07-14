#include "n32g031_gpio.h"
#include "n32g031_adc.h"
#include "adc.h"

extern uint16_t ADCConvertedValue;

/**
 * @brief  Configures the ADC GPIO ports.
 */
void ADC_GPIO_Init(void)
{
	  GPIO_InitType GPIO_InitStructure;
	

		/* Enable DMA clocks */
//    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA, ENABLE);
		/* Enable GPIOB clocks */
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
	
	//IR 端口输入，下拉
	
//	GPIO_InitStruct(&GPIO_InitStructure);
//  GPIO_InitStructure.Pin = GPIO_IR;
//  GPIO_InitStructure.GPIO_Mode = GPIO_MODE_INPUT;
//	GPIO_InitStructure.GPIO_Pull = GPIO_PULL_DOWN;
//  GPIO_InitPeripheral(PORT_IR, &GPIO_InitStructure);
	
    GPIO_InitStruct(&GPIO_InitStructure);
    /* Configure PB0/PB1 as analog input -------------------------*/
    GPIO_InitStructure.Pin       = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_MODE_ANALOG;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
}

/**
 * @brief  Configures the different system clocks.
 */
void RCC_Configuration(void)
{
	ErrorStatus HSIStartUpStatus;

//	/* Enable DMA clocks */
	RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA, ENABLE);
 
	/* Enable ADC clocks */
	RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC, ENABLE);
	/* enable ADC 1M clock */
	RCC_EnableHsi(ENABLE);
	/* Wait til1 HSI is ready*/
	HSIStartUpStatus = RCC_WaitHsiStable(); 
	if(HSIStartUpStatus == SUCCESS)
	{
	}
	else
	{
			/* If HSI fails to start-up, the application will have wrong clock configuration. User can add here some code to deal with this error*/
			/* Go to infinitel1oop*/
			while(1)
			{
			}
	}
	RCC_ConfigAdc1mClk(RCC_ADC1MCLK_SRC_HSI, RCC_ADC1MCLK_DIV8);
	/* RCC_ADCHCLK_DIV16*/
	ADC_ConfigClk(ADC_CTRL3_CKMOD_AHB, RCC_ADCHCLK_DIV16); //ADC 3M Hz
}


void ADC_Initial(void)
{
	ADC_InitType ADC_InitStructure;
	DMA_InitType DMA_InitStructure;
	
	RCC_Configuration();
	ADC_GPIO_Init();
	
//	/* DMA channel1 configuration ----------------------------------------------*/
//	DMA_DeInit(DMA_CH1);
//	DMA_InitStructure.PeriphAddr     = (uint32_t)&ADC->DAT;
//	DMA_InitStructure.MemAddr        = (uint32_t)ADCConvertedValue;
//	DMA_InitStructure.Direction      = DMA_DIR_PERIPH_SRC; //外设到DMA
//	DMA_InitStructure.BufSize        = 2;
//	DMA_InitStructure.PeriphInc      = DMA_PERIPH_INC_DISABLE;
//	DMA_InitStructure.DMA_MemoryInc  = DMA_MEM_INC_ENABLE;
//	DMA_InitStructure.PeriphDataSize = DMA_PERIPH_DATA_SIZE_HALFWORD;
//	DMA_InitStructure.MemDataSize    = DMA_MemoryDataSize_HalfWord;
//	DMA_InitStructure.CircularMode   = DMA_MODE_CIRCULAR; //不断传输
//	DMA_InitStructure.Priority       = DMA_PRIORITY_HIGH;
//	DMA_InitStructure.Mem2Mem        = DMA_M2M_DISABLE;
//	DMA_Init(DMA_CH1, &DMA_InitStructure);
//	DMA_RequestRemap(DMA_REMAP_ADC, DMA, DMA_CH1, ENABLE);
//	
//	/* Enable DMA channel1 */
//	DMA_EnableChannel(DMA_CH1, ENABLE);
//	
//	/* ADC_DMA configuration ------------------------------------------------------*/
//	ADC_InitStructure.MultiChEn      = ENABLE;
//	ADC_InitStructure.ContinueConvEn = ENABLE;
//	ADC_InitStructure.ExtTrigSelect  = ADC_EXT_TRIGCONV_NONE;
//	ADC_InitStructure.DatAlign       = ADC_DAT_ALIGN_R;
//	ADC_InitStructure.ChsNumber      = 2; //顺序进行规则转换的ADC通道数目
//	ADC_Init(ADC, &ADC_InitStructure);
//	/* ADC1 regular channel8 configuration */
//	ADC_ConfigRegularChannel(ADC, ADC_CH_8_PB0, 1, ADC_SAMP_TIME_56CYCLES5); //第一个序列是PB0
//	ADC_ConfigRegularChannel(ADC, ADC_CH_9_PB1, 2, ADC_SAMP_TIME_56CYCLES5); //第二个序列是PB1
//	/* Enable ADC DMA */
//	ADC_EnableDMA(ADC, ENABLE);


		/* ADC configuration ------------------------------------------------------*/
    ADC_InitStructure.MultiChEn      = DISABLE;
    ADC_InitStructure.ContinueConvEn = DISABLE;
    ADC_InitStructure.ExtTrigSelect  = ADC_EXT_TRIGCONV_NONE;
    ADC_InitStructure.DatAlign       = ADC_DAT_ALIGN_R;
    ADC_InitStructure.ChsNumber      = 1;
    ADC_Init(ADC, &ADC_InitStructure);

	/* Enable ADC */
	ADC_Enable(ADC, ENABLE);
	/* Check ADC Ready */
	while(ADC_GetFlagStatusNew(ADC,ADC_FLAG_RDY) == RESET)
			;
	while(ADC_GetFlagStatusNew(ADC,ADC_FLAG_PD_RDY))
        ;
//	/* Start ADC Software Conversion */
    ADC_EnableSoftwareStartConv(ADC, ENABLE);
}

uint16_t ADC_GetData(uint8_t ADC_Channel)
{
    uint16_t dat;
    ADC_ConfigRegularChannel(ADC, ADC_Channel, 1, ADC_SAMP_TIME_56CYCLES5);
    /* Start ADC Software Conversion */
    ADC_EnableSoftwareStartConv(ADC,ENABLE);
    while(ADC_GetFlagStatus(ADC,ADC_FLAG_ENDC)==0){
    }
    ADC_ClearFlag(ADC,ADC_FLAG_ENDC);
    ADC_ClearFlag(ADC,ADC_FLAG_STR);
    dat=ADC_GetDat(ADC);
    return dat;
}