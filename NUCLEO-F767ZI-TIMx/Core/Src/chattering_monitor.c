/*
 * chattering_monitor.c
 *
 *  Created on: 2022. 10. 5.
 *      Author: JMJ
 */

#if 0

#include "usart.h"
#include <stdio.h>

#define UART3_printf(fmt, ...)\
do {\
	char buf[256];\
	int sz = sprintf(buf, fmt, ##__VA_ARGS__);\
	HAL_UART_Transmit(&huart3, (uint8_t*)buf, sz, HAL_MAX_DELAY);\
} while (0)

#define stream DMA1_Stream2

static uint32_t data;
static volatile uint16_t cplt;

static void TIM5_MspInit()
{
	/* DMA */
	stream->CR |= (
			6UL << DMA_SxCR_CHSEL_Pos 	| // select channel 6
			DMA_SxCR_CIRC				| // circular mode
			0b00 << DMA_SxCR_DIR_Pos	| // peripheral-to-memory
			0b10 << DMA_SxCR_MSIZE_Pos	| // msize=word
			0b10 << DMA_SxCR_PSIZE_Pos	| // psize=word
//			DMA_SxCR_MINC				| // mem increase

			DMA_SxCR_TCIE				| // transfer cplt interrupt enable
	0);

	stream->NDTR = 1; // num of data
	stream->PAR = (uint32_t)&TIM5->CCR1; // periph addr
	stream->M0AR = (uint32_t)data; // mem addr

	HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);

	/* GPIO */
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef gpio = { 0 };

	/* TIM5_CH1 */
	gpio.Pin = GPIO_PIN_0;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_LOW;
	gpio.Alternate = GPIO_AF2_TIM5;
	HAL_GPIO_Init(GPIOA, &gpio);
}

static void TIM5_Init()
{
	__HAL_RCC_TIM5_CLK_ENABLE();
	TIM5_MspInit();

	TIM5->PSC = 96;
	TIM5->ARR = 1e6;

	TIM5->CR1 |= (
		TIM_CR1_OPM					| // one-pulse mode
	0);

	TIM5->CCMR1 |= (
		0b01 << TIM_CCMR1_CC1S_Pos	| // Capture/Compare Selection (IC1 is mapped on TI1)
		0b000 << TIM_CCMR1_IC1F_Pos	| // No filter, sampling is done at fDTS
		0b00 << TIM_CCMR1_IC1PSC_Pos| // no prescaler
	0);

	TIM5->CCER |= (
		TIM_CCER_CC1NP|TIM_CCER_CC1P| // both edges
	0);

	TIM5->SMCR |= (
		0b110 << TIM_SMCR_SMS_Pos	| // trigger mode
		0b101 << TIM_SMCR_TS_Pos	| // TI1FP1
	0);

	TIM5->EGR = TIM_EGR_UG;
	TIM5->SR = ~TIM_SR_UIF;

	TIM5->DIER |= (
		TIM_DIER_CC1DE				| // CC1 DMA request enable
		TIM_DIER_UIE				| // update interrupt enable
	0);

	HAL_NVIC_SetPriority(TIM5_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(TIM5_IRQn);
}


void TestFunction()
{
	TIM5_Init();

	stream->CR |= DMA_SxCR_EN;
	TIM5->CCER |= TIM_CCER_CC1E;

	while (!cplt);

	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
	UART3_printf("%lu : %lu\r\n", data, TIM5->CCR1);

	while (1);
}

void TIM5_IRQHandler()
{
	if (TIM5->SR & TIM_SR_UIF) {
		TIM5->SR = ~TIM_SR_UIF;

		cplt = 1;
		TIM5->CCER &= ~TIM_CCER_CC1E;
		TIM5->SMCR &= ~TIM_SMCR_SMS_Msk;
		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);

	}
}

void DMA1_Stream2_IRQHandler()
{
	if (DMA1->LISR & DMA_LISR_TCIF2) {
		DMA1->LIFCR = DMA_LIFCR_CTCIF2;
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
	}
}

#endif

#if 0

#include "usart.h"
#include <stdio.h>

#define UART3_printf(fmt, ...)\
do {\
	char buf[256];\
	int sz = sprintf(buf, fmt, ##__VA_ARGS__);\
	HAL_UART_Transmit(&huart3, (uint8_t*)buf, sz, HAL_MAX_DELAY);\
} while (0)

static void TIM5_MspInit()
{

	/* GPIO */
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef gpio = { 0 };

	/* TIM5_CH1 */
	gpio.Pin = GPIO_PIN_0;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_LOW;
	gpio.Alternate = GPIO_AF2_TIM5;
	HAL_GPIO_Init(GPIOA, &gpio);
}

static void TIM5_Init()
{
	__HAL_RCC_TIM5_CLK_ENABLE();
	TIM5_MspInit();

	TIM5->PSC = 0;
	TIM5->ARR = (uint32_t)1e6 * 96;

	TIM5->CR1 |= (
		TIM_CR1_OPM					| // one-pulse mode
	0);

	TIM5->CCMR1 |= (
		0b01 << TIM_CCMR1_CC1S_Pos	| // Capture/Compare Selection (IC1 is mapped on TI1)
		0b0000 << TIM_CCMR1_IC1F_Pos| // No filter, sampling is done at fDTS
		0b00 << TIM_CCMR1_IC1PSC_Pos| // no prescaler
	0);

	TIM5->CCER |= (
		TIM_CCER_CC1P				| // falling edge
	0);

	TIM5->SMCR |= (
		0b110 << TIM_SMCR_SMS_Pos	| // trigger mode
		0b101 << TIM_SMCR_TS_Pos	| // TI1FP1
	0);

	TIM5->EGR = TIM_EGR_UG;
	TIM5->SR = ~TIM_SR_UIF;

	TIM5->DIER |= (
		TIM_DIER_UIE				| // update interrupt enable
//		TIM_DIER_CC1IE				| // CC1 interrupt enable
	0);

	TIM5->CNT = 11;

	HAL_NVIC_SetPriority(TIM5_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(TIM5_IRQn);
}

static volatile uint8_t cplt;

void TestFunction()
{

	TIM5_Init();

	TIM5->CCER |= TIM_CCER_CC1E; // capture enable

	while (!cplt);

	UART3_printf("%lu\r\n", TIM5->CCR1);

	while (1);
}

void TIM5_IRQHandler()
{
	if (TIM5->SR & TIM_SR_UIF) {
		TIM5->SR = ~TIM_SR_UIF;

		TIM5->CCER &= ~TIM_CCER_CC1E;
		TIM5->SMCR &= ~TIM_SMCR_SMS_Msk;
		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
		cplt = 1;
	}
}

#endif

#if 0

#include "main.h"

static void GPIO_Init()
{
	/* GPIO */
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef gpio = { 0 };

	/* TIM5_CH1 */
	gpio.Pin = GPIO_PIN_0;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_LOW;
	gpio.Alternate = GPIO_AF2_TIM5;
	HAL_GPIO_Init(GPIOA, &gpio);
}

void TestFunction()
{
	GPIO_Init();

	uint16_t prev_pin = 0x0001U;
	uint16_t prev_btn = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);

	HAL_GPIO_WritePin(GPIOB, prev_pin, GPIO_PIN_SET);

	while (1) {
		uint16_t curr_btn = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);

		if (~curr_btn & prev_btn) {
		  HAL_GPIO_WritePin(GPIOB, prev_pin, GPIO_PIN_RESET);
		  prev_pin <<= 7; // LD1, LD2, LD3가 각각 0, 7, 14에 있다는 것을 이용
		  if (!prev_pin) prev_pin = 0x0001U;
		  HAL_GPIO_WritePin(GPIOB, prev_pin, GPIO_PIN_SET);
		}

		prev_btn = curr_btn;

//		HAL_Delay(1);
	}
}
#endif









