/*
 * [L]TEST_ADC_with_DMA.c
 *
 *  Created on: 2022. 10. 5.
 *      Author: JMJ
 */

#include "usart.h"
#include <stdio.h>

static inline void ADC_STAB() { // stabilization for accuracy
	volatile uint32_t counter = 0;
    counter = (10 * (SystemCoreClock / (uint32_t)1e6));
    while (counter) counter--;
}

#define UART3_printf(fmt, ...)\
do {\
	char buf[256];\
	int sz = sprintf(buf, fmt, ##__VA_ARGS__);\
	HAL_UART_Transmit(&huart3, (uint8_t*)buf, sz, HAL_MAX_DELAY);\
} while (0)

#if 0 /* multi-channel, single conversion */

#define stream DMA2_Stream0

static uint16_t data[2];

static void ADC1_Msp_Init()
{
	/* DMA */
	__HAL_RCC_DMA2_CLK_ENABLE();

	stream->CR |= (
			0UL << DMA_SxCR_CHSEL_Pos 	| // select channel 0
//			DMA_SxCR_CIRC				| // circular mode
			0b00 << DMA_SxCR_DIR_Pos	| // peripheral-to-memory
			0b01 << DMA_SxCR_MSIZE_Pos	| // msize=half-word
			0b01 << DMA_SxCR_PSIZE_Pos	| // psize=half-word
			DMA_SxCR_MINC				| // mem increase
	0);

	stream->NDTR = 2; // num of data
	stream->PAR = (uint32_t)&ADC1->DR; // periph addr
	stream->M0AR = (uint32_t)data; // mem addr

	/* GPIO */
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitTypeDef gpio = { 0 };

	/* ADC123_IN12 */
	gpio.Pin = GPIO_PIN_2;
	gpio.Mode = GPIO_MODE_ANALOG;

	HAL_GPIO_Init(GPIOC, &gpio);

	HAL_NVIC_SetPriority(ADC_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(ADC_IRQn);
}

static void ADC1_Init()
{
	__HAL_RCC_ADC1_CLK_ENABLE();
	ADC1_Msp_Init();

	// ADC(숫자 없음)는 common(공통)이므로 모든 ADC(1, 2, 3, ...)에 적용된다.
	ADC->CCR |= (
		0b00 << ADC_CCR_ADCPRE_Pos 	| // PCLK2 divided by 2
	0);

	ADC1->CR1 |= (
		0b00 << ADC_CR1_RES_Pos		| // resolution: 12-bit
		ADC_CR1_SCAN				| // enable scan mode

		ADC_CR1_OVRIE				| // overrun interrupt enable
//		ADC_CR1_EOCIE				| // EOC interrupt enable
	0);

	ADC1->CR2 |= (
//		ADC_CR2_CONT				| // continuous conversion mode
//		ADC_CR2_EOCS				| // enable overrun detection
		ADC_CR2_DMA					| // DMA mode enable
	0);

	/* channel configuration */
	ADC1->SQR1 |= (2 - 1) << ADC_SQR1_L_Pos; // sequence length
	/* SQ1 */
	ADC1->SQR3 |= 18 << ADC_SQR3_SQ1_Pos; // Assign channel18(temperature sensor) to SQ1
	ADC1->SMPR1 |= 0b111 << ADC_SMPR1_SMP18_Pos; // sampling time: 480 ADCCLK
	/* SQ2 */
	ADC1->SQR3 |= 12 << ADC_SQR3_SQ2_Pos; // temperature sensor와 구분될 수 있는 무엇이든
	ADC1->SMPR1 |= 0b111 << ADC_SMPR1_SMP12_Pos; // sampling time: 480 ADCCLK
}

void TestFunction()
{
	ADC1_Init();

	ADC->CCR |= ADC_CCR_TSVREFE; // temperature sensor channel enable
	ADC1->CR2 |= ADC_CR2_ADON; // Enable ADC
	ADC_STAB();

	stream->CR |= DMA_SxCR_EN;
	ADC1->CR2 |= ADC_CR2_SWSTART; // start conversion

	while ( !(DMA2->LISR & DMA_LISR_TCIF0) );
	DMA2->LIFCR = DMA_LIFCR_CTCIF0;

	for (int i = 0; i < 2; i++) {
		UART3_printf("v%d: %hu\r\n", i, data[i]);
	}

	UART3_printf("v%d: %hu\r\n", 2, (uint16_t)ADC1->DR);

	stream->CR |= DMA_SxCR_EN;
	ADC1->CR2 |= ADC_CR2_SWSTART; // start conversion

	while (1);
}

void ADC_IRQHandler()
{
	if (ADC1->SR & ADC_SR_OVR) {
		ADC1->SR &= ~ADC_SR_OVR;
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
	}
	if (ADC1->SR & ADC_SR_EOC) {
		ADC1->SR &= ~ADC_SR_EOC;
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
	}
}

#endif /* multi-channel, single conversion */

#if 0 /* ADC_CR2_DDS */

/*
DMA_CIRC, ADC_DMA:
	00:
		NDTR만큼 전송 후 스트림 비활성화 => 다시 활성화 필요
		스트림 비활성화에 따른 request 비활성화 => ADC_CR2_DMA 비트 0으로 했다가 다시 1로 해야됨
	01:
		NDTR만큼 전송 후 스트림 비활성화 => 다시 활성화 필요
		request는 꺼지지 않음

	10:
		stream은 꺼지지 않음
		DMA last transfer후 request 비활성화 => ADC_CR2_DMA 비트 0으로 했다가 다시 1로 해야됨

	11:
		stream은 꺼지지 않음
		request는 꺼지지 않음
 */

#define stream DMA2_Stream0

static uint16_t data[4];

static void ADC1_Msp_Init()
{
	/* DMA */
	__HAL_RCC_DMA2_CLK_ENABLE();

	stream->CR |= (
			0UL << DMA_SxCR_CHSEL_Pos 	| // select channel 0
			DMA_SxCR_CIRC				| // circular mode
			0b00 << DMA_SxCR_DIR_Pos	| // peripheral-to-memory
			0b01 << DMA_SxCR_MSIZE_Pos	| // msize=half-word
			0b01 << DMA_SxCR_PSIZE_Pos	| // psize=half-word
			DMA_SxCR_MINC				| // mem increase
	0);

	stream->NDTR = 4; // num of data
	stream->PAR = (uint32_t)&ADC1->DR; // periph addr
	stream->M0AR = (uint32_t)data; // mem addr

	/* GPIO */
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitTypeDef gpio = { 0 };

	/* ADC123_IN12 */
	gpio.Pin = GPIO_PIN_2;
	gpio.Mode = GPIO_MODE_ANALOG;

	HAL_GPIO_Init(GPIOC, &gpio);

	HAL_NVIC_SetPriority(ADC_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(ADC_IRQn);
}

static void ADC1_Init()
{
	__HAL_RCC_ADC1_CLK_ENABLE();
	ADC1_Msp_Init();

	// ADC(숫자 없음)는 common(공통)이므로 모든 ADC(1, 2, 3, ...)에 적용된다.
	ADC->CCR |= (
		0b00 << ADC_CCR_ADCPRE_Pos 	| // PCLK2 divided by 2
	0);

	ADC1->CR1 |= (
		0b00 << ADC_CR1_RES_Pos		| // resolution: 12-bit
		ADC_CR1_SCAN				| // enable scan mode

		ADC_CR1_OVRIE				| // overrun interrupt enable
//		ADC_CR1_EOCIE				| // EOC interrupt enable
	0);

	ADC1->CR2 |= (
//		ADC_CR2_CONT				| // continuous conversion mode
//		ADC_CR2_EOCS				| // enable overrun detection
		ADC_CR2_DMA					| // DMA mode enable
		ADC_CR2_DDS					| // DMA disable selection
	0);

	/* channel configuration */
	ADC1->SQR1 |= (2 - 1) << ADC_SQR1_L_Pos; // sequence length
	/* SQ1 */
	ADC1->SQR3 |= 18 << ADC_SQR3_SQ1_Pos; // Assign channel18(temperature sensor) to SQ1
	ADC1->SMPR1 |= 0b111 << ADC_SMPR1_SMP18_Pos; // sampling time: 480 ADCCLK
	/* SQ2 */
	ADC1->SQR3 |= 12 << ADC_SQR3_SQ2_Pos; // temperature sensor와 구분될 수 있는 무엇이든
	ADC1->SMPR1 |= 0b111 << ADC_SMPR1_SMP12_Pos; // sampling time: 480 ADCCLK
}

void TestFunction()
{
	ADC1_Init();

	ADC->CCR |= ADC_CCR_TSVREFE; // temperature sensor channel enable
	ADC1->CR2 |= ADC_CR2_ADON; // Enable ADC
	ADC_STAB();

	stream->CR |= DMA_SxCR_EN;

	while (1) {
		ADC1->CR2 |= ADC_CR2_SWSTART; // start conversion

		while ( !(DMA2->LISR & DMA_LISR_HTIF0) );
		DMA2->LIFCR = DMA_LIFCR_CHTIF0;

		for (int i = 0; i < 2; i++)
			UART3_printf("v%d: %hu\r\n", i, data[i]);
		HAL_Delay(1000);


		ADC1->CR2 |= ADC_CR2_SWSTART; // start conversion

		while ( !(DMA2->LISR & DMA_LISR_TCIF0) );
		DMA2->LIFCR = DMA_LIFCR_CTCIF0;

		for (int i = 2; i < 4; i++)
			UART3_printf("v%d: %hu\r\n", i, data[i]);
		HAL_Delay(1000);

		UART3_printf("\r\n");
//		stream->CR |= DMA_SxCR_EN;
//		ADC1->CR2 &= ~ADC_CR2_DMA;
//		ADC1->CR2 |= ADC_CR2_DMA;
	}
}

void ADC_IRQHandler()
{
	if (ADC1->SR & ADC_SR_OVR) {
		ADC1->SR &= ~ADC_SR_OVR;
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
	}
//	if (ADC1->SR & ADC_SR_EOC) {
//		ADC1->SR &= ~ADC_SR_EOC;
//		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
//	}
}

#endif /* ADC_CR2_DDS */






















