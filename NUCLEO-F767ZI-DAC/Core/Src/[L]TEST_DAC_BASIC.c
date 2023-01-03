/*
 * [L]TEST_DAC_BASIC.c
 *
 *  Created on: Dec 31, 2022
 *      Author: JMJ
 */

#if 0

#include "main.h"

static inline
void DAC_WAKEUP() {
	volatile uint32_t counter;
	// DAC channel wakeup time
	counter = (10 * (SystemCoreClock / 1000000)); // 10us
	while (counter) counter--;
}

static
void GPIO_Init() {
	GPIO_InitTypeDef gpio_init = { 0 };

	__HAL_RCC_GPIOA_CLK_ENABLE();
	/* PA4 ------> DAC_OUT1 */
	gpio_init.Pin = GPIO_PIN_4;
	gpio_init.Mode = GPIO_MODE_ANALOG;
	gpio_init.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpio_init);
}

static
void DAC_Init() {
	__HAL_RCC_DAC_CLK_ENABLE();
}

void TestFunction() {
	GPIO_Init();
	DAC_Init();

	// 1====================================================
	DAC->CR |= DAC_CR_EN1;
	DAC_WAKEUP();

	while (1) {
		for (uint16_t i = 2000; i < 4000; i += 4) {
			DAC->DHR12R1 = i;
			HAL_Delay(1);
		}
		for (uint16_t i = 4000; i > 2000; i -= 4) {
			DAC->DHR12R1 = i;
			HAL_Delay(1);
		}
	}

	// 2====================================================
//	DAC->DHR12R1 = 0;
//	HAL_Delay(1);
//
//	DAC->CR |= DAC_CR_EN1;
//	DAC_WAKEUP();
//
//	while (1);
}

#endif
