/*
 * [L]TEST_DAC_NOISE_WAVE.c
 *
 *  Created on: 2023. 1. 2.
 *      Author: JMJ
 */

/*
 * variable-amplitude pseudonoise를 생성하기 위해
 * LFSR(Linear Feedback Shift Register)가 사용된다.
 * 이 레지스터는 특정 계산 알고리즘에 따라 각 trigger event 후
 * 3개의 APB1 클록 주기에 업데이트된다.
 *
 * LFSR값은 DAC_CR 레지스터의 MAMPx[3:0] 비트를 통해
 * 부분적으로 또는 전체적으로 마스킹될 수 있다.
 *
 * LFSR값은 DAC_DHRx이 DAC_DORx로 전송되는 시점에
 * DAC_DHRx과 overflow 없이 더해진후 DAC_DORx에 저장된다.
 *
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

	DAC->CR |= (
		// WAVE1: Only used if bit TEN1 = 1
		0b01 << DAC_CR_WAVE1_Pos 	| // noise wave generation enable
		0b1011 << DAC_CR_MAMP1_Pos	| // amplitude: 4095

		0b111 << DAC_CR_TSEL1_Pos 	| // software trigger
		DAC_CR_TEN1					| // trigger enable
	0);
}

void TestFunction() {
	GPIO_Init();
	DAC_Init();

	DAC->CR |= DAC_CR_EN1;
	DAC_WAKEUP();

	DAC->DHR12R1 = 0;

	while (1) {
		DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
		HAL_Delay(250);
	}
}

#endif

#if 0

#include "main.h"

static inline
void DAC_WAKEUP() {
	volatile uint32_t counter;
	// DAC channel wakeup time
	counter = (10 * (SystemCoreClock / 1000000)); // 10us
	while (counter) counter--;
}

static inline
uint16_t LFS(uint16_t a) {
	uint16_t xr = (a ^ (a >> 1) ^ (a >> 4) ^ (a >> 6) ^ !a) & 1;
	return (xr << 11) | (a >> 1);
}

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
void DAC_Init() {
	__HAL_RCC_DAC_CLK_ENABLE();

	DAC->CR |= (
		// WAVE1: Only used if bit TEN1 = 1
		0b01 << DAC_CR_WAVE1_Pos 	| // noise wave generation enable
		0b1011 << DAC_CR_MAMP1_Pos	| // amplitude: 4095

		0b111 << DAC_CR_TSEL1_Pos 	| // software trigger
		DAC_CR_TEN1					| // trigger enable
	0);
}

void TestFunction() {
	GPIO_Init();
	DAC_Init();

	DAC->CR |= DAC_CR_EN1;
	DAC->CR |= DAC_CR_EN2;
	DAC_WAKEUP();

	uint16_t a = 0xAAA;

	DAC->DHR12R1 = 0;
	while (1) {
		DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
		DAC->DHR12R2 = a;
		a = LFS(a);
		HAL_Delay(250);
	}
}

#endif




















