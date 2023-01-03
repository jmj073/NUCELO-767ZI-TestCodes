/*
 * [L]TEST_DAC_DUAL_MODE.c
 *
 *  Created on: 2023. 1. 3.
 *      Author: JMJ
 */


#include "main.h"

static inline
void DAC_WAKEUP() {
	volatile uint32_t counter;
	// DAC channel wakeup time
	counter = (10 * (SystemCoreClock / 1000000)); // 10us
	while (counter) counter--;
}

#if 0 /* Independent trigger without wave generation */

static
void GPIO_Init() {
	GPIO_InitTypeDef gpio_init = { 0 };

	__HAL_RCC_GPIOA_CLK_ENABLE();
	/* PA4 ------> DAC_OUT1 */
	gpio_init.Pin = GPIO_PIN_4 | GPIO_PIN_5;
	gpio_init.Mode = GPIO_MODE_ANALOG;
	gpio_init.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpio_init);
}

static
void TIM2_Init() { // APB1 tim_clk: 96MHz
	__HAL_RCC_TIM2_CLK_ENABLE();

	TIM2->PSC = 48000 - 1;
	TIM2->ARR = 1000 - 1;

	TIM2->CR1 |= TIM_CR1_ARPE;
	TIM2->CR2 |= 0b010 << TIM_CR2_MMS_Pos; // update event를 TRGO로 설정
}

static
void TIM5_Init() { // APB1 tim_clk: 96MHz
	__HAL_RCC_TIM5_CLK_ENABLE();

	TIM5->PSC = 48000 - 1;
	TIM5->ARR = 500 - 1;

	TIM5->CR1 |= TIM_CR1_ARPE;
	TIM5->CR2 |= 0b010 << TIM_CR2_MMS_Pos; // update event를 TRGO로 설정

	TIM5->SMCR |= (
		0b000 << TIM_SMCR_TS_Pos	| // trigger selection: ITR0(TIM2)
		0b110 << TIM_SMCR_SMS_Pos	| //The counter starts at a rising edge of the trigger TRGI
	0);
}

static
void DAC_Init() {
	__HAL_RCC_DAC_CLK_ENABLE();

	DAC->CR |= (
		/* channel 1 */
		0b100 << DAC_CR_TSEL1_Pos 	| // Timer 2 TRGO event
		DAC_CR_TEN1					| // trigger enable
		/* channel 2 */
		0b011 << DAC_CR_TSEL2_Pos 	| // Timer 5 TRGO event
		DAC_CR_TEN2					| // trigger enable
	0);
}

void TestFunction() {
	GPIO_Init();
	TIM2_Init();
	TIM5_Init();
	DAC_Init();

	DAC->CR |= DAC_CR_EN1;
	DAC->CR |= DAC_CR_EN2;
	DAC_WAKEUP();
	TIM2->CR1 |= TIM_CR1_CEN;

	while (1) {
		for (uint16_t i = 2000; i < 4000; i += 4) {
			DAC->DHR12RD = (i << 16) | i;
			HAL_Delay(1);
		}
		for (uint16_t i = 4000; i > 2000; i -= 4) {
			DAC->DHR12RD = (i << 16) | i;
			HAL_Delay(1);
		}
	}
}

#endif /* Independent trigger without wave generation */

#if 0 /* Independent trigger with single LFSR generation */

static
void GPIO_Init() {
	GPIO_InitTypeDef gpio_init = { 0 };

	__HAL_RCC_GPIOA_CLK_ENABLE();
	/* PA4 ------> DAC_OUT1 */
	gpio_init.Pin = GPIO_PIN_4 | GPIO_PIN_5;
	gpio_init.Mode = GPIO_MODE_ANALOG;
	gpio_init.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpio_init);
}

static
void TIM2_Init() { // APB1 tim_clk: 96MHz
	__HAL_RCC_TIM2_CLK_ENABLE();

	TIM2->PSC = 48000 - 1;
	TIM2->ARR = 1000 - 1;

	TIM2->CR1 |= TIM_CR1_ARPE;
	TIM2->CR2 |= 0b010 << TIM_CR2_MMS_Pos; // update event를 TRGO로 설정
}

static
void TIM5_Init() { // APB1 tim_clk: 96MHz
	__HAL_RCC_TIM5_CLK_ENABLE();

	TIM5->PSC = 48000 - 1;
	TIM5->ARR = 500 - 1;

	TIM5->CR1 |= TIM_CR1_ARPE;
	TIM5->CR2 |= 0b010 << TIM_CR2_MMS_Pos; // update event를 TRGO로 설정

	TIM5->SMCR |= (
		0b000 << TIM_SMCR_TS_Pos	| // trigger selection: ITR0(TIM2)
		0b110 << TIM_SMCR_SMS_Pos	| //The counter starts at a rising edge of the trigger TRGI
	0);
}

static
void DAC_Init() {
	__HAL_RCC_DAC_CLK_ENABLE();

	DAC->CR |= (
		/* channel 1 */
		0b100 << DAC_CR_TSEL1_Pos 	| // Timer 2 TRGO event
		DAC_CR_TEN1					| // trigger enable
		0b01 << DAC_CR_WAVE1_Pos	| // noise wave generation
		0b1010 << DAC_CR_MAMP1_Pos	| // 2047

		/* channel 2 */
		0b011 << DAC_CR_TSEL2_Pos 	| // Timer 5 TRGO event
		DAC_CR_TEN2					| // trigger enable
		0b01 << DAC_CR_WAVE2_Pos	| // noise wave generation
		0b1010 << DAC_CR_MAMP2_Pos	| // 2047
	0);

}

void TestFunction() {
	GPIO_Init();
	TIM2_Init();
	TIM5_Init();
	DAC_Init();

	DAC->CR |= DAC_CR_EN1;
	DAC->CR |= DAC_CR_EN2;
	DAC_WAKEUP();
	TIM2->CR1 |= TIM_CR1_CEN;

	DAC->DHR12RD = (2048 << 16) | 2048;
	while (1);
}

#endif /* Independent trigger with single LFSR generation */

#if 1 /* Independent trigger with different LFSR generation */

// TODO: 마저 작업

static
void GPIO_Init() {
	GPIO_InitTypeDef gpio_init = { 0 };

	__HAL_RCC_GPIOA_CLK_ENABLE();
	/* PA4 ------> DAC_OUT1 */
	gpio_init.Pin = GPIO_PIN_4 | GPIO_PIN_5;
	gpio_init.Mode = GPIO_MODE_ANALOG;
	gpio_init.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpio_init);
}

static
void TIM2_Init() { // APB1 tim_clk: 96MHz
	__HAL_RCC_TIM2_CLK_ENABLE();

	TIM2->PSC = 48000 - 1;
	TIM2->ARR = 1000 - 1;

	TIM2->CR1 |= TIM_CR1_ARPE;
	TIM2->CR2 |= 0b010 << TIM_CR2_MMS_Pos; // update event를 TRGO로 설정
}

static
void TIM5_Init() { // APB1 tim_clk: 96MHz
	__HAL_RCC_TIM5_CLK_ENABLE();

	TIM5->PSC = 48000 - 1;
	TIM5->ARR = 500 - 1;

	TIM5->CR1 |= TIM_CR1_ARPE;
	TIM5->CR2 |= 0b010 << TIM_CR2_MMS_Pos; // update event를 TRGO로 설정

	TIM5->SMCR |= (
		0b000 << TIM_SMCR_TS_Pos	| // trigger selection: ITR0(TIM2)
		0b110 << TIM_SMCR_SMS_Pos	| //The counter starts at a rising edge of the trigger TRGI
	0);
}

static
void DAC_Init() {
	__HAL_RCC_DAC_CLK_ENABLE();

	DAC->CR |= (
		/* channel 1 */
		0b100 << DAC_CR_TSEL1_Pos 	| // Timer 2 TRGO event
		DAC_CR_TEN1					| // trigger enable
		0b01 << DAC_CR_WAVE1_Pos	| // noise wave generation
		0b1010 << DAC_CR_MAMP1_Pos	| // 2047

		/* channel 2 */
		0b011 << DAC_CR_TSEL2_Pos 	| // Timer 5 TRGO event
		DAC_CR_TEN2					| // trigger enable
		0b01 << DAC_CR_WAVE2_Pos	| // noise wave generation
		0b1010 << DAC_CR_MAMP2_Pos	| // 2047
	0);

}

void TestFunction() {
	GPIO_Init();
	TIM2_Init();
	TIM5_Init();
	DAC_Init();

	DAC->CR |= DAC_CR_EN1;
	DAC->CR |= DAC_CR_EN2;
	DAC_WAKEUP();
	TIM2->CR1 |= TIM_CR1_CEN;

	DAC->DHR12RD = (2048 << 16) | 2048;
	while (1);
}

#endif /* Independent trigger with different LFSR generation */














