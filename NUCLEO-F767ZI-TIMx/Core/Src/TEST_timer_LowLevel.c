/*
 * TEST_timer_LowLevel.c
 *
 *  Created on: 2022. 8. 28.
 *      Author: JMJ
 */

#if 0 /* basic timer test */

#include "main.h"

static void TIM6_Init()
{
	__HAL_RCC_TIM6_CLK_ENABLE();

	TIM6->PSC = 1000 - 1;
	TIM6->ARR = 48000 - 1;
	TIM6->DIER |= TIM_DIER_UIE;

	TIM6->CR1 |= TIM_CR1_ARPE;

	HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
}

void TestFunction()
{
	TIM6_Init();

	TIM6->CR1 |= TIM_CR1_CEN; // counter enable

	while (1);
}

void TIM6_DAC_IRQHandler()
{
	if (TIM6->SR & TIM_SR_UIF) {
		TIM6->SR &= ~TIM_SR_UIF;

		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
	}
}

#endif /* basic timer test */

// ====================general purpose timer clock selections====================

#if 0 /* internal clock (CK_INT) */

#include "main.h"

static void TIM3_Init()
{
	__HAL_RCC_TIM3_CLK_ENABLE();

	TIM3->PSC = 1000 - 1;
	TIM3->ARR = 48000 - 1;
	TIM3->DIER |= TIM_DIER_UIE;

	TIM3->CR1 |= TIM_CR1_ARPE;

	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

void TestFunction()
{
	TIM3_Init();

	TIM3->CR1 |= TIM_CR1_CEN; // counter enable

	while (1);
}

void TIM3_IRQHandler()
{
	if (TIM3->SR & TIM_SR_UIF) {
		TIM3->SR &= ~TIM_SR_UIF;

		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
	}
}

#endif /* internal clock source */

#if 0 /* external clock mode 1 */

#include "main.h"

static void MCO_Init()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);

	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

// TI2 입력의 상승 에지에 응답하여 카운트하도록 업카운터를 구성=====================================
#if 0

static void TIM3_GPIO_Init()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef gpio = {
		.Pin = GPIO_PIN_7,
		.Mode = GPIO_MODE_AF_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_LOW,
		.Alternate = GPIO_AF2_TIM3,
	};

	HAL_GPIO_Init(GPIOA, &gpio);
}

static void TIM3_Init()
{
	__HAL_RCC_TIM3_CLK_ENABLE();
	TIM3_GPIO_Init();


	/* capture/compare selection은 external clock mode 1과 상관이 없지만 필터 설정을 위해서는
	 * input 모드로 설정되어 있어야 하기 때문에 그냥 00만 아니면(not output mode) 되는 듯 하다.
	 * 물론 필터를 default값을 쓰면 굳이 해주지 않아도 될듯.
	 */
	TIM3->CCMR1 |= (
			(0b01 << TIM_CCMR1_CC2S_Pos) 	| // Capture/Compare 2 selection (IC2 is mapped on TI2)
			(0 << TIM_CCMR1_IC2F_Pos)		  // filter
	);

	// Capture/Compare 1 output Polarity
	TIM3->CCER &= ~(TIM_CCER_CC2P | TIM_CCER_CC2NP); // noninverted/rising edge


	TIM3->SMCR |= (
		0b111 << TIM_SMCR_SMS_Pos	| // Slave mode selection (external clock mode 1)
		0b110 << TIM_SMCR_TS_Pos	  // Trigger selection (Filtered Timer Input 2(TI2FP2))
	);

	TIM3->PSC = 1000 - 1;
	TIM3->ARR = 8000 - 1;

	TIM3->DIER |= TIM_DIER_UIE; // update interrupt enable
	TIM3->CR1 |= TIM_CR1_ARPE; // auto-reload preload enable

	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

#endif
// =================================================================================================
#if 0
static void TIM3_GPIO_Init()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef gpio = {
		.Pin = GPIO_PIN_6,
		.Mode = GPIO_MODE_AF_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_LOW,
		.Alternate = GPIO_AF2_TIM3,
	};

	HAL_GPIO_Init(GPIOA, &gpio);
}

static void TIM3_Init()
{
	__HAL_RCC_TIM3_CLK_ENABLE();
	TIM3_GPIO_Init();

//	TIM3->CCMR1 |= ({
//			(0b01 << TIM_CCMR1_CC2S_Pos) 	| // Capture/Compare 2 selection (IC2 is mapped on TI1)
//			(0 << TIM_CCMR1_IC1F_Pos)		; // filter
//	});

	// Capture/Compare 1 output Polarity
	TIM3->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC1NP); // noninverted/rising edge


	TIM3->SMCR |= (
		0b111 << TIM_SMCR_SMS_Pos	| // Slave mode selection (external clock mode 1)
		0b101 << TIM_SMCR_TS_Pos	  // Trigger selection (Filtered Timer Input 2(TI2FP2))
	);

	TIM3->PSC = 1000 - 1;
	TIM3->ARR = 8000 - 1;

	TIM3->DIER |= TIM_DIER_UIE; // update interrupt enable
	TIM3->CR1 |= TIM_CR1_ARPE; // auto-reload preload enable

	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
}
#endif
// =================================================================================================

static void TIM3_GPIO_Init()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef gpio = { 0 };

	gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_LOW;
	gpio.Alternate = GPIO_AF2_TIM3,
	HAL_GPIO_Init(GPIOA, &gpio);
}

static void TIM3_Init()
{
	__HAL_RCC_TIM3_CLK_ENABLE();
	TIM3_GPIO_Init();

	TIM3->SMCR |= (
		0b111 << TIM_SMCR_SMS_Pos	| // Slave mode selection (external clock mode 1)
		0b100 << TIM_SMCR_TS_Pos	  // Trigger selection (TI1 Edge Detector (TI1F_ED))
//		0b101 << TIM_SMCR_TS_Pos	  // Filtered Timer Input 1 (TI1FP1)
	);

	// Capture/Compare 1 output Polarity
	TIM3->CCER |= (TIM_CCER_CC1P | TIM_CCER_CC1NP); // noninverted/both edges

	TIM3->PSC = 1000 - 1;
	TIM3->ARR = 8000 - 1;

	TIM3->DIER |= TIM_DIER_UIE; // update interrupt enable
	TIM3->CR1 |= TIM_CR1_ARPE; // auto-reload preload enable

	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

void TestFunction()
{
	MCO_Init();
	TIM3_Init();

	TIM3->CR1 |= TIM_CR1_CEN; // counter enable

	while (1);
}

void TIM3_IRQHandler()
{
	if (TIM3->SR & TIM_SR_UIF) {
		TIM3->SR &= ~TIM_SR_UIF;

		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
	}
}

#endif /* external clock mode 1 */

#if 0 /* external clock mode 2 */

#include "main.h"

static void MCO_Init()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);

	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void TIM3_GPIO_Init()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef gpio = { 0 };

	/* TIM3_ETR */
	gpio.Pin = GPIO_PIN_2;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_LOW;
	gpio.Alternate = GPIO_AF2_TIM3,
	HAL_GPIO_Init(GPIOD, &gpio);
}

static void TIM3_Init()
{
	__HAL_RCC_TIM3_CLK_ENABLE();
	TIM3_GPIO_Init();

	TIM3->SMCR |= (
		TIM_SMCR_ETP				| // External trigger polarity (ETR is inverted, falling edge)
		TIM_SMCR_ETPS_0				| // External trigger prescaler (ETRP frequency  divided by 2)
		TIM_SMCR_ECE				| // External clock enable (external clock mode 2 enabled)
		0b1111 << TIM_SMCR_ETF_Pos	| // External trigger filter (fSAMPLING=fDTS/32, N=8)
	0);

	// Capture/Compare 1 output Polarity
	TIM3->CCER |= (TIM_CCER_CC1P | TIM_CCER_CC1NP); // noninverted/both edges

	TIM3->PSC = 0;
	TIM3->ARR = 1;

	TIM3->DIER |= TIM_DIER_UIE; // update interrupt enable
	TIM3->CR1 |= TIM_CR1_ARPE; // auto-reload preload enable

	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

void TestFunction()
{
	MCO_Init();
	TIM3_Init();

	TIM3->CR1 |= TIM_CR1_CEN; // counter enable

	while (1);
}

void TIM3_IRQHandler()
{
	if (TIM3->SR & TIM_SR_UIF) {
		TIM3->SR &= ~TIM_SR_UIF;

		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
	}
}

#endif /* external clock mode 2 */

// ==============================================================================

#if 0 /* input capture mode */

#include "main.h"

#define stream DMA2_Stream5

static const uint32_t DMA_DATA[2] = { GPIO_PIN_5, GPIO_PIN_5 << 16 };

static void TIM1_Msp_Init()
{
	/* DMA */
	stream->CR |= (
			6UL << DMA_SxCR_CHSEL_Pos 	| // select channel 6
			DMA_SxCR_CIRC				| // circular mode
			0b01 << DMA_SxCR_DIR_Pos	| // memory-to-peripheral
			0b10 << DMA_SxCR_MSIZE_Pos	| // msize=word
			0b10 << DMA_SxCR_PSIZE_Pos	| // psize=word
			DMA_SxCR_MINC				| // mem increase
	0);

	stream->NDTR = 2; // num of data
	stream->PAR = (uint32_t)&GPIOA->BSRR; // periph addr
	stream->M0AR = (uint32_t)DMA_DATA; // mem addr

	/* GPIO */
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef gpio = { 0 };

	gpio.Pin = GPIO_PIN_5;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOA, &gpio);
}

static void TIM1_Init()
{
	// 500ms마다 DMA request 생성
	__HAL_RCC_TIM1_CLK_ENABLE();
	TIM1_Msp_Init();

	TIM1->PSC = 1000 - 1;
	TIM1->ARR = 48000 - 1;
	TIM1->DIER |= TIM_DIER_UDE;

	TIM1->CR1 |= TIM_CR1_ARPE;
}

static void TIM3_Msp_Init()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef gpio = { 0 };

	/* TIM3_CH1 */
	gpio.Pin = GPIO_PIN_6;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_LOW;
	gpio.Alternate = GPIO_AF2_TIM3;
	HAL_GPIO_Init(GPIOA, &gpio);

//	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
//	HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

static void TIM3_Init() // 96MHz
{
	__HAL_RCC_TIM3_CLK_ENABLE();
	TIM3_Msp_Init();

	TIM3->CCMR1 |= (
		0b01 << TIM_CCMR1_CC1S_Pos  | // Capture/Compare Selection (IC1 is mapped on TI1)
		0b011 << TIM_CCMR1_IC1F_Pos | // input capture filter (f=CK_INT, N=8)
		0b01 << TIM_CCMR1_IC1PSC_Pos| // input capture prescaler (capture is done once every 2 events)
	0);

	TIM3->CCER |= (
		TIM_CCER_CC1P				| // Capture/Compare Polarity (falling edge)
	0);

	TIM3->PSC = 48000 - 1; // 2KHz
	TIM3->ARR = (1 << 16) - 1;

	TIM3->CR1 |= TIM_CR1_ARPE; // auto-reload preload enable

//	TIM3->DIER |= TIM_DIER_CC1IE; // capture/compare 1 interrupt enable
}

//static void DMA_TEST()
//{
//	__HAL_RCC_GPIOA_CLK_ENABLE();
//	GPIO_InitTypeDef gpio = { 0 };
//
//	gpio.Pin = GPIO_PIN_6;
//	gpio.Mode = GPIO_MODE_INPUT;
//	gpio.Pull = GPIO_NOPULL;
//	gpio.Speed = GPIO_SPEED_LOW;
//	HAL_GPIO_Init(GPIOA, &gpio);
//
//	while (1) {
//		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6));
//	}
//}

#include "usart.h"
#include <stdio.h>

/*
 * PA_5 -> PA_6
 */

void TestFunction()
{
	char buf[64];

	TIM3_Init();
	TIM1_Init();

	TIM3->CR1 |= TIM_CR1_CEN; // counter enable
	TIM3->CCER |= TIM_CCER_CC1E; // capture enable

	TIM1->CR1 |= TIM_CR1_CEN; // counter enable
	stream->CR |= DMA_SxCR_EN; // stream enable

	uint16_t prev_capture = 0;
	while (1) {
		if (TIM3->SR & TIM_SR_CC1IF) {
			uint16_t curr_capture = TIM3->CCR1;
			// uint16_t로 캐스팅하는 이유는 '산술 변환'을 검색해 보세요
			uint16_t period = (uint16_t)(curr_capture - prev_capture) / (HAL_RCC_GetPCLK1Freq() / 500 / (TIM3->PSC + 1)); // ms

			size_t sz = sprintf(buf, "%hu: %hu\r\n", period, curr_capture);
			HAL_UART_Transmit(&huart3, (uint8_t*)buf, sz, HAL_MAX_DELAY);

			prev_capture = curr_capture;
		}
	}
}

//#include <stdio.h>
//
//static char buf[1024];
//
//void TestFunction()
//{
//	size_t size = 0;
//
//	size += sprintf(buf + size, "HLCK	: %10lu\r\n", HAL_RCC_GetHCLKFreq());
//	size += sprintf(buf + size, "PCLK1	: %10lu\r\n", HAL_RCC_GetPCLK1Freq());
//	size += sprintf(buf + size, "PCLK2	: %10lu\r\n", HAL_RCC_GetPCLK2Freq());
//
//	HAL_UART_Transmit(&huart3, (uint8_t*)buf, size, HAL_MAX_DELAY);
//
//	while (1);
//}

#endif /* input capture mode */

#if 0 /* over capture */

#include "usart.h"
#include <stdio.h>

static void TIM3_Init() // 96MHz
{
	__HAL_RCC_TIM3_CLK_ENABLE();

	TIM3->CCMR1 |= (
		0b01 << TIM_CCMR1_CC1S_Pos  | // Capture/Compare Selection (IC1 is mapped on TI1)
		0b011 << TIM_CCMR1_IC1F_Pos | // input capture filter (f=CK_INT, N=8)
	0);

	TIM3->CCER |= (
		TIM_CCER_CC1P				| // Capture/Compare Polarity (falling edge)
	0);

	TIM3->PSC = (1 << 16) -1;
	TIM3->ARR = (1 << 16) - 1;

//	TIM3->DIER |= TIM_DIER_CC1IE; // capture/compare 1 interrupt enable
}

void TestFunction()
{
	char buf[64];

	TIM3_Init();

	TIM3->CCER |= TIM_CCER_CC1E; // capture/compare enable

	TIM3->CNT = 1234;
	TIM3->EGR = TIM_EGR_CC1G;

	if (TIM3->SR & TIM_SR_CC1IF) {
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
	}

	TIM3->CNT = 5678;
	TIM3->EGR = TIM_EGR_CC1G;

	if (TIM3->SR & TIM_SR_CC1OF) {
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
	}

	size_t sz = 0;
	sz += sprintf(buf + sz, "%lu : ", TIM3->CCR1);
	TIM3->SR &= ~TIM_SR_CC1OF;
	sz += sprintf(buf + sz, "%lu\r\n", TIM3->CCR1);

	HAL_UART_Transmit(&huart3, (uint8_t*)buf, sz, HAL_MAX_DELAY);

	while (1);
}

#endif /* over capture */

#if 0 /* output compare mode */

#include "main.h"

static void TIM3_MspInit()
{
	__HAL_RCC_TIM3_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef gpio = { 0 };

	/* TIM3_CH1 */
	gpio.Pin = GPIO_PIN_6;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_LOW;
	gpio.Alternate = GPIO_AF2_TIM3;
	HAL_GPIO_Init(GPIOA, &gpio);
}

static void TIM3_Init()
{
	TIM3_MspInit();

	TIM3->PSC = 1000 - 1;
	TIM3->ARR = 48000 - 1;

	TIM3->CCMR1 |= (
		0b011 << TIM_CCMR1_OC1M_Pos | // toggle
	0);

	TIM3->CCER |= TIM_CCER_CC1P; // OC1 active low

	TIM3->CR1 |= TIM_CR1_ARPE;

	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

static void GPIO_Init()
{
	GPIO_InitTypeDef gpio = { 0 };

	gpio.Pin = GPIO_PIN_5;
	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpio);
}

void TestFunction()
{
	TIM3_Init();
	GPIO_Init();

	TIM3->CCR1 = 1234;

	TIM3->CCER |= TIM_CCER_CC1E; // output enable
	TIM3->CR1 |= TIM_CR1_CEN; // counter enable

	while (1) {
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5));
	}
}

#endif /* output compare mode */

#if 0 /* PWM edge-aligned */

#include "main.h"

static void TIM3_MspInit()
{
	__HAL_RCC_TIM3_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef gpio = { 0 };

	/* TIM3_CH1 */
	gpio.Pin = GPIO_PIN_6;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_LOW;
	gpio.Alternate = GPIO_AF2_TIM3;
	HAL_GPIO_Init(GPIOA, &gpio);
}

static void TIM3_Init()
{
	/* TIM3 clocked 96MHz */

	TIM3_MspInit();

	TIM3->PSC = 9600 - 1;
	TIM3->ARR = 100 - 1; // 10,000 / 100

	TIM3->CCMR1 |= (
		0b110 << TIM_CCMR1_OC1M_Pos | // PWM1
		TIM_CCMR1_OC1PE				| // preload enable
	0);

	TIM3->CR1 |= TIM_CR1_ARPE;

	//	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
	//	HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

void TestFunction()
{
	TIM3_Init();

	TIM3->CCER |= TIM_CCER_CC1E; // output enable
	TIM3->CR1 |= TIM_CR1_CEN; // counter enable

	while (1) {
		for (int brightness = 100; brightness >= 0; brightness--) {
			TIM3->CCR1 = brightness;
			HAL_Delay(10);
		}
		for (int brightness = 0; brightness <= 100; brightness++) {
			TIM3->CCR1 = brightness;
			HAL_Delay(10);
		}
	}
}

#endif /* PWM */

#if 0 /* OPM */

#include "main.h"

static void TIM3_MspInit()
{
	__HAL_RCC_TIM3_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef gpio = { 0 };

	/* TIM3_CH1 | TIM3_CH2 */
	gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_LOW;
	gpio.Alternate = GPIO_AF2_TIM3;
	HAL_GPIO_Init(GPIOA, &gpio);
}

static void TIM3_Init()
{
	/* TIM3 is clocked at 96MHz */

	TIM3_MspInit();

	TIM3->PSC = 48000 - 1;
	TIM3->ARR = 4000 - 1;

	/* CHANNEL1 config */

	TIM3->CCR1 = 2000;

	TIM3->CCMR1 |= (
		0b110 << TIM_CCMR1_OC1M_Pos | // PWM1
		TIM_CCMR1_OC1PE				| // preload enable
	0);


	TIM3->CCER |= TIM_CCER_CC1P; // active low

	/* CHANNEL2 config */

	/* etc...? */

	TIM3->SMCR |= (
		0b110 << TIM_SMCR_TS_Pos 	| // TI2FP2 -> TRGI
		0b110 << TIM_SMCR_SMS_Pos	| // trigger mode
	0);

	TIM3->CR1 |= (
		TIM_CR1_ARPE				| // arr preload enable
		TIM_CR1_OPM					| // one-pulse mode enable
	0);


	TIM3->EGR = TIM_EGR_UG;

	//	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
	//	HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

static void GPIO_Init()
{
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOA_CLK_ENABLE();

	/* PA4 -> TIM3_CH2 */
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void TestFunction()
{
	TIM3_Init();
	GPIO_Init();

	TIM3->CCER |= TIM_CCER_CC1E; // output enable

	HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

	while (1) {
		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
		HAL_Delay(1000);
	}
}

#endif /* OPM */

#if 0 /* timer synchronization */

#include "main.h"

static void TIM4_Init() // master timer
{
	__HAL_RCC_TIM4_CLK_ENABLE();

	TIM4->PSC = 1000 - 1;
	TIM4->ARR = 24000 - 1;

	TIM4->CR2 |= 0b010 << TIM_CR2_MMS_Pos; // UEV is selected as TRGO
}

static void TIM3_Init() // slave timer
{
	__HAL_RCC_TIM3_CLK_ENABLE();

	TIM3->SMCR |= (
		0b111 << TIM_SMCR_SMS_Pos	| // external clock mode 1
		0b011 << TIM_SMCR_TS_Pos	| // ITR3(TIM4)
	0);

	TIM3->PSC = 1 - 1;
	TIM3->ARR = 2 - 1;

	TIM3->DIER |= TIM_DIER_UIE; // update interrupt enable
	TIM3->CR1 |= TIM_CR1_ARPE; // auto-reload preload enable

	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

void TestFunction()
{
	TIM3_Init();
	TIM4_Init();

	TIM3->CR1 |= TIM_CR1_CEN; // counter enable
	TIM4->CR1 |= TIM_CR1_CEN; // counter enable

	while (1);
}

void TIM3_IRQHandler()
{
	if (TIM3->SR & TIM_SR_UIF) {
		TIM3->SR &= ~TIM_SR_UIF;

		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
	}
}

#endif /* timer synchronization */

#if 0 /* encoder mode */

#include "usart.h"
#include <stdio.h>

static void TIM3_MspInit()
{
	__HAL_RCC_TIM3_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitTypeDef gpio = { 0 };

	/* TIM3_CH1(CLK) | TIM3_CH2(DT) */
	gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Alternate = GPIO_AF2_TIM3;
	HAL_GPIO_Init(GPIOC, &gpio);
}

static void TIM3_Init()
{
	TIM3_MspInit();

	/* TI1FP1 */

	TIM3->CCMR1 |= (
		0b01 << TIM_CCMR1_CC1S_Pos 		| //
		0b1111 << TIM_CCMR1_IC1F_Pos 	| // filter
	0);

//	TIM3->CCER |= TIM_CCER_CC1P; // inverted

	/* TI2FP2 */

	TIM3->CCMR1 |= (
		0b01 << TIM_CCMR1_CC2S_Pos 		| //
		0b1111 << TIM_CCMR1_IC2F_Pos 	| // filter
	0);

//	TIM3->CCER |= TIM_CCER_CC2P; // inverted


	TIM3->SMCR |= (
		0b0001 << TIM_SMCR_SMS_Pos		| // encoder mode 1
		0b1111 << TIM_CCMR1_IC1F_Pos 	| // filter
	0);

	TIM3->PSC = 0;
	TIM3->ARR = 1001;

	TIM3->CR1 |= TIM_CR1_ARPE;

	TIM3->EGR = TIM_EGR_UG;
}

void TestFunction()
{
	char buf[64];

	TIM3_Init();

	TIM3->CR1 |= TIM_CR1_CEN;

	while (1) {
		size_t sz = sprintf(buf, "%hu\r\n", (uint16_t)TIM3->CNT);
		HAL_UART_Transmit(&huart3, (uint8_t*)buf, sz, HAL_MAX_DELAY);

		HAL_Delay(500);
	}
}

#if 0 /* encoder test */

#include "main.h"

#define ENCODER_GPIO	GPIOC
//#define ENCODER_SW		GPIO_PIN_4
#define ENCODER_CLK		GPIO_PIN_6
#define ENCODER_DT		GPIO_PIN_7

static void GPIO_Init()
{
	GPIO_InitTypeDef gpio = { 0 };

	__HAL_RCC_GPIOC_CLK_ENABLE();

	/* input */
	gpio.Pin = ENCODER_DT | ENCODER_CLK ; // | ENCODER_SW;
	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(ENCODER_GPIO, &gpio);
}

void TestFunction()
{
	GPIO_Init();

	while (1) {
//		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, HAL_GPIO_ReadPin(ENCODER_GPIO, ENCODER_SW));
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, HAL_GPIO_ReadPin(ENCODER_GPIO, ENCODER_DT));
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, HAL_GPIO_ReadPin(ENCODER_GPIO, ENCODER_CLK));
	}
}

#endif /* encoder test */

#endif /* endcoder mode */

















