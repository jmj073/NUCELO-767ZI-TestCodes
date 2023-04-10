/*
 * [H]TEST_DAC_DMA.c
 *
 *  Created on: Dec 31, 2022
 *      Author: JMJ
 */

/* TODO: 작동 안함. 고쳐야 됨. */

#if 0

#include "main.h"
#include <math.h>

#define STEP_SIZE 200

static DAC_HandleTypeDef hdac;
static TIM_HandleTypeDef htim6;
static DMA_HandleTypeDef hdma_dac_ch1;

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM6) {
		__HAL_RCC_TIM6_CLK_ENABLE();
	}
}

static void TIM6_Init() { // timer clock:s 96 MHz
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	htim6.Instance = TIM6;
	htim6.Init.Prescaler = 96 - 1;
	htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim6.Init.Period = 100 - 1;
	HAL_TIM_Base_Init(&htim6);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig);
}

void HAL_DAC_MspInit(DAC_HandleTypeDef* hdac) {
	GPIO_InitTypeDef gpio_init = { 0 };

	if (hdac->Instance == DAC) {
		__HAL_RCC_DAC_CLK_ENABLE();

		__HAL_RCC_GPIOA_CLK_ENABLE();
	    /* PA4 ------> DAC_OUT1 */
		gpio_init.Pin = GPIO_PIN_4;
		gpio_init.Mode = GPIO_MODE_ANALOG;
		gpio_init.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &gpio_init);

		__HAL_RCC_DMA1_CLK_ENABLE();
		hdma_dac_ch1.Instance = DMA1_Stream5;
		hdma_dac_ch1.Init.Channel = DMA_CHANNEL_7;
		hdma_dac_ch1.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_dac_ch1.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_dac_ch1.Init.MemInc = DMA_MINC_ENABLE;
		hdma_dac_ch1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
		hdma_dac_ch1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
		hdma_dac_ch1.Init.Mode = DMA_CIRCULAR;
		hdma_dac_ch1.Init.Priority = DMA_PRIORITY_LOW;
		HAL_DMA_Init(&hdma_dac_ch1);
		__HAL_LINKDMA(hdac, DMA_Handle1, hdma_dac_ch1);
	}

}

static void DAC_Init() {
	DAC_ChannelConfTypeDef sConfig = { 0 };

	hdac.Instance = DAC;

	HAL_DAC_Init(&hdac);

	sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
	sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
	HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1);
}

void TestFunction() {
	uint16_t data[STEP_SIZE];

	for (size_t i = 0; i < STEP_SIZE; ++i) {
		uint16_t v =  rint( (sinf(((2 * M_PI) / STEP_SIZE) * i) + 1) * 2048);
		data[i] = v < 4096 ? v : 4095;
	}

	DAC_Init();
	TIM6_Init();


	HAL_TIM_Base_Start(&htim6);
	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)data, STEP_SIZE, DAC_ALIGN_12B_R);

	while (1);
}

#endif
