/*
 * [L]TEST_DAC_DUAL_CHANNEL.c
 *
 *  Created on: 2023. 1. 3.
 *      Author: JMJ
 */

/*
소위 "dual mode"라 말할 수 있을 것 같은데,
이 "dual mode"들은 공통적으로
(DAC_DHR12RD, DAC_DHR12LD or DAC_DHR8RD)
레지스터를 사용한다.

# mode list

  1. Independent trigger without wave generation
  2. Independent trigger with single LFSR generation
  3. Independent trigger with different LFSR generation
  4. Independent trigger with single triangle generation
  5. Independent trigger with different triangle generation
  6. Simultaneous software start
  7. Simultaneous trigger without wave generation
  8. Simultaneous trigger with single LFSR generation
  9. Simultaneous trigger with different LFSR generation
 10. Simultaneous trigger with single triangle generation
 11. Simultaneous trigger with different triangle generation

# mode 분류 구성요소

 + trigger: independent or simultaneous
   independent  => TSEL1[2:0] != TSEL2[2:0]
   simultaneous => TSEL1[2:0] == TSEL2[2:0]

 + single or different(Only when waveform generation is enabled)
   single    => MAMP1[3:0] == MAMP2[3:0]
   different => MAMP1[3:0] != MAMP2[3:0]

 + wave gen: LFSR, triangle or none
   LFSR     => WAVE[1:0] == 01
   triangle => WAVE[1:0] == 10

 + mode 6번은 trigger 없다.
*/

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
	gpio_init.Pin = GPIO_PIN_4 | GPIO_PIN_5;
	gpio_init.Mode = GPIO_MODE_ANALOG;
	gpio_init.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpio_init);
}

#if 0 /* Independent trigger without wave generation */

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
	TIM5->ARR = 1000 - 1;

	TIM5->SMCR |= (
		0b000 << TIM_SMCR_TS_Pos	| // trigger selection: ITR0(TIM2)
		0b110 << TIM_SMCR_SMS_Pos	| //The counter starts at a rising edge of the trigger TRGI
	0);

	TIM5->CR1 |= TIM_CR1_ARPE;
	TIM5->CR2 |= 0b010 << TIM_CR2_MMS_Pos; // update event를 TRGO로 설정

	TIM5->EGR = TIM_EGR_UG;
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

#if 0 /* Independent trigger with different LFSR generation */
// single과 다른 점은 두 channel의 LFSR mask가 같은가 다른가인 듯하다.
#endif /* Independent trigger with different LFSR generation */

#if 0 /* Simultaneous software start */

static
void DAC_Init() {
	__HAL_RCC_DAC_CLK_ENABLE();
}

void TestFunction() {
	GPIO_Init();
	DAC_Init();

	DAC->CR |= DAC_CR_EN1;
	DAC->CR |= DAC_CR_EN2;
	DAC_WAKEUP();

	while (1) {
		for (uint16_t i = 2000; i < 4000; i += 4) {
			DAC->DHR12RD = ((6000 - i) << 16) | i;
			HAL_Delay(1);
		}
		for (uint16_t i = 4000; i > 2000; i -= 4) {
			DAC->DHR12RD = ((6000 - i) << 16) | i;
			HAL_Delay(1);
		}
	}
}

#endif /* Simultaneous software start */

#if 0 /* Simultaneous trigger without wave generation */

static
void TIM6_Init() { // APB1 tim clk: 96MHz
	__HAL_RCC_TIM6_CLK_ENABLE();

	TIM6->PSC = 48000 - 1;
	TIM6->ARR = 500 - 1;

	TIM6->CR2 |= 0b010 << TIM_CR2_MMS_Pos;
	TIM6->CR1 |= TIM_CR1_ARPE;
}

static
void DAC_Init() {
	__HAL_RCC_DAC_CLK_ENABLE();

	DAC->CR |= (
		/* channel 1 */
		0b000 << DAC_CR_TSEL1_Pos 	| // Timer 6 TRGO event
		DAC_CR_TEN1					| // trigger enable

		/* channel 2 */
		0b000 << DAC_CR_TSEL2_Pos 	| // Timer 6 TRGO event
		DAC_CR_TEN2					| // trigger enable
	0);
}

void TestFunction() {
	TIM6_Init();
	GPIO_Init();
	DAC_Init();

	DAC->CR |= DAC_CR_EN1;
	DAC->CR |= DAC_CR_EN2;
	DAC_WAKEUP();

	TIM6->CR1 |= TIM_CR1_CEN;

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

#endif /* Simultaneous trigger without wave generation */






