/*
 * [L]TEST_DAC_TRIG.c
 *
 *  Created on: 2023. 1. 1.
 *      Author: JMJ
 */

#include "main.h"

static inline
void DAC_WAKEUP() {
	volatile uint32_t counter;
	// DAC channel wakeup time
	counter = (10 * (SystemCoreClock / (uint32_t)1e6)); // 10us
	while (counter) counter--;
}

#if 0 /* software trigger */

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

	DAC->CR |= (
		0b111 << DAC_CR_TSEL1_Pos 	| // Software trigger
		DAC_CR_TEN1					| // trigger enable
	0);
}

void TestFunction() {
	GPIO_Init();
	DAC_Init();

	DAC->CR |= DAC_CR_EN1;
	DAC_WAKEUP();

	/*
	 * trigger signal이 와야 DHR(Data Holding Register)에 있는 데이터를
	 * DOR(Data Output Register)로 전송하므로 LED가 부드럽게 밝기가 변하지 않고,
	 * 깜박거리게 된다.
	 */

	while (1) {
		for (uint16_t i = 2000; i < 4000; i += 4) {
			DAC->DHR12R1 = i;
			HAL_Delay(1);
		}
		DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1; // software trigger 신호를 보낸다.

		for (uint16_t i = 4000; i > 2000; i -= 4) {
			DAC->DHR12R1 = i;
			HAL_Delay(1);
		}
		DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1; // software trigger 신호를 보낸다.
	}

//	while (1) {
//		DAC->DHR12R1 = 4095;
//		DAC->DHR8R1 = 0;
//		DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
//
//		HAL_Delay(500);
//
//		DAC->DHR8R1 = 0;
//		DAC->DHR12R1 = 4095;
//		DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
//
//		HAL_Delay(500);
//	}
}

#endif /* software trigger */

#if 0

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
void TIM6_Init() { // APB1 tim clk: 96MHz
	__HAL_RCC_TIM6_CLK_ENABLE();

	TIM6->PSC = 48000 - 1;
	TIM6->ARR = 500 - 1;

	TIM6->CR1 |= TIM_CR1_ARPE;

	TIM6->CR2 |= 0b010 << TIM_CR2_MMS_Pos; // update event를 TRGO로 설정
}

static
void DAC_Init() {
	__HAL_RCC_DAC_CLK_ENABLE();

	/* channel1 */
	DAC->CR |= (
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

	TIM6->CR1 |= TIM_CR1_CEN;

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
}

#endif
