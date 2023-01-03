/*
 * [L]TEST_DAC_TRIANGLE_WAVE.c
 *
 *  Created on: 2023. 1. 2.
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
void TIM6_Init() { // APB1 tim_clk: 96MHz
	__HAL_RCC_TIM6_CLK_ENABLE();

	TIM6->PSC = 1 - 1;
	TIM6->ARR = 23443 - 1;

	TIM6->CR1 |= TIM_CR1_ARPE;
	TIM6->CR2 |= 0b010 << TIM_CR2_MMS_Pos; // update event를 TRGO로 설정
}

static
void DAC_Init() {
	__HAL_RCC_DAC_CLK_ENABLE();

	DAC->CR |= (
		// WAVE1: Only used if bit TEN1 = 1
		0b10 << DAC_CR_WAVE1_Pos 	| // triangle wave generation enable
		0b1011 << DAC_CR_MAMP1_Pos	| // amplitude: 4095

		0b000 << DAC_CR_TSEL1_Pos 	| // Timer 6 TRGO event
		DAC_CR_TEN1					| // trigger enable
	0);
}

void TestFunction() {
	GPIO_Init();
	TIM6_Init();
	DAC_Init();

	DAC->CR |= DAC_CR_EN1;
	DAC_WAKEUP();
	DAC->DHR12R1 = 0;
	TIM6->CR1 |= TIM_CR1_CEN;

	while (1);
}

#endif
