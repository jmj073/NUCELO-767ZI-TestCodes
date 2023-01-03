/*
 * TEST_ADC_EXT_TRG.c
 *
 *  Created on: 2022. 10. 5.
 *      Author: JMJ
 */

#if 0 /* external trigger */
/* using TIM2_TRGO */

#include <stdio.h>
#include "usart.h"

static void TIM2_Init()
{
	__HAL_RCC_TIM2_CLK_ENABLE();

	TIM2->PSC = 48000 - 1;
	TIM2->ARR = 2000 - 1;

	TIM2->CR2 |= 0b010 << TIM_CR2_MMS_Pos; // UEV is selected as TRGO
}

static void ADC1_Init()
{
	__HAL_RCC_ADC1_CLK_ENABLE();

	// ADC(숫자 없음)은 common(공통)이므로 모든 ADC(1, 2, 3, ...)에 적용된다.
	ADC->CCR |= (
		0b00 << ADC_CCR_ADCPRE_Pos 	| // PCLK2 divided by 2
	0);

	ADC1->CR1 |= (
		0b00 << ADC_CR1_RES_Pos		| // resolution: 12-bit
	0);

	ADC1->CR2 |= (
		0b01 << ADC_CR2_EXTEN_Pos	| // trigger detection on the rising edge
		0b1011 << ADC_CR2_EXTSEL_Pos| // TIM2_TRGO
		ADC_CR2_EOCS				| // enable overrun detection
	0);

	/* channel configuration */
	ADC1->SQR3 |= 18 << ADC_SQR3_SQ1_Pos; // Assign channel18(temperature sensor) to SQ1
	ADC1->SMPR1 |= 0b111 << ADC_SMPR1_SMP18_Pos; // sampling time: 480 ADCCLK
}

void TestFunction()
{
	ADC1_Init();
	TIM2_Init();

	ADC->CCR |= ADC_CCR_TSVREFE; // temperature sensor channel enable
	ADC1->CR2 |= ADC_CR2_ADON; // Enable ADC
	ADC_STAB();

	TIM2->CR1 |= TIM_CR1_CEN;

	while (1) {
		while ( !(ADC1->SR & ADC_SR_EOC) ); // waiting for End Of Conversion

		uint16_t raw_value = ADC1->DR;
		float temp = ((float)raw_value) / ((1 << 12) - 1) * 3300;
		// temperature sensor electrical characteristics 참고
		temp = (temp - 760.0) / 2.5 + 25;

		char msg[64];

		int sz = sprintf(msg, "ADC Raw: %hu\r\n", raw_value);
		HAL_UART_Transmit(&huart3, (uint8_t*)msg, sz, HAL_MAX_DELAY);

		sz = sprintf(msg, "Temperature: %f\r\n", temp);
		HAL_UART_Transmit(&huart3, (uint8_t*)msg, sz, HAL_MAX_DELAY);

		if (ADC1->SR & ADC_SR_OVR) {
			HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
			break;
		}
	}
}

#endif /* external trigger */
