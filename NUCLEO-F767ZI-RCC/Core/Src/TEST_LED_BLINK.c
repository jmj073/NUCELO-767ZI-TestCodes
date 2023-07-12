/*
 * first_test(blink).c
 *
 *  Created on: Apr 18, 2023
 *      Author: user
 */

#include "main.h"
#include <stdint.h>

#define LD1 0
#define LD2 7
#define LD3 14

static
void GPIO_Init() {
	__IO uint32_t tmpreg;

	/* clock enable */
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN);
	// enable bit가 설정되고 난후 clock이 active되기
	// 전에 2개 peripheral clock cycle의 delay가 있다.
	// 따라서 READ_BIT를 통해 delay를 기다린다.
	tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN);

	/* user LD1(PB0) */
	GPIOB->OSPEEDR |= 0b00 << GPIO_OSPEEDR_OSPEEDR0_Pos; // low speed
	GPIOB->OTYPER |= 0b0 << GPIO_OTYPER_OT0_Pos; // output push-pull
	GPIOB->MODER |= 0b01 << GPIO_MODER_MODER0_Pos; // general purpose output mode

	/* user LD2(PB7) */
	GPIOB->OSPEEDR |= 0b00 << GPIO_OSPEEDR_OSPEEDR7_Pos; // low speed
	GPIOB->OTYPER |= 0b0 << GPIO_OTYPER_OT7_Pos; // output push-pull
	GPIOB->MODER |= 0b01 << GPIO_MODER_MODER7_Pos; // general purpose output mode

	/* user LD3(PB14) */
	GPIOB->OSPEEDR |= 0b00 << GPIO_OSPEEDR_OSPEEDR14_Pos; // low speed
	GPIOB->OTYPER |= 0b0 << GPIO_OTYPER_OT14_Pos; // output push-pull
	GPIOB->MODER |= 0b01 << GPIO_MODER_MODER14_Pos; // general purpose output mode

	UNUSED(tmpreg);
}

static
void delay() {
	for (volatile uint32_t i = 0; i < (uint32_t)1e6	/ 2; ++i);
}

void TestFunction() {
	GPIO_Init();

	/* test HSI oscillator ON */
	// system reset후에 HSI ocillator는 system clock으로 선택된다.
	// HSI clock signal은 내부 16 MHz RC oscillator로부터 생성된다.
	if (RCC->CR & RCC_CR_HSION) { // HSI oscillator가 켜져 있다면
		GPIOB->BSRR = 1 << LD2; // led2 on
	}

	/* AHB prescaler */
	// AHB precaler는 SYSCLK을 나눈다음 HCLK을 출력한다.
	// HCLK은 cortex core에 공급되므로 아래 코드와 같이
	// 4로 나누면 나누지 않는 것보다 blinking 속도가
	// 느려질 것이다.
//	RCC->CFGR |= 0b1001 << RCC_CFGR_HPRE_Pos; // system clock divided by 4

	/* blinking led */
	while (1) {
		GPIOB->BSRR = 1 << LD1; // on
		delay();
		GPIOB->BSRR = 1 << (LD1 + 16); // off
		delay();
	}
}
















