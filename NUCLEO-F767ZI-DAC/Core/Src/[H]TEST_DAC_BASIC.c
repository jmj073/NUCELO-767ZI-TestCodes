/*
 * [H]TEST_DAC_BASIC.c
 *
 *  Created on: Dec 31, 2022
 *      Author: JMJ
 */

#if 0

#include "main.h"

static DAC_HandleTypeDef hdac;

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
	}

}

static void DAC_Init() {
	DAC_ChannelConfTypeDef sConfig = { 0 };

	hdac.Instance = DAC;

	HAL_DAC_Init(&hdac);

	sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
	sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
	HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1);
}

void TestFunction() {
	DAC_Init();

	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);

	while (1) {
		int i = 2000;
		for (; i < 4000; i += 4) {
			HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, i);
			HAL_Delay(1);
		}
		for (; i > 2000; i -= 4) {
			HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, i);
			HAL_Delay(1);
		}
	}
}

#endif
