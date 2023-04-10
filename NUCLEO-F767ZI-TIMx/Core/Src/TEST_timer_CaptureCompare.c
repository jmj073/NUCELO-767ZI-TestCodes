/*
 * TEST_timer_InputCapture.c
 *
 *  Created on: 2022. 8. 27.
 *      Author: JMJ
 */


#if 0 /* input capture */

// 동작 안하는 코드임

#include "main.h"
#include "usart.h"
#include <stdio.h>

static TIM_HandleTypeDef htim1;
static DMA_HandleTypeDef hdma_tim1_up;

static TIM_HandleTypeDef htim3;
static DMA_HandleTypeDef hdma_tim3_ch1_trig;

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	if (htim->Instance == TIM1) {
		__HAL_RCC_TIM1_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();

		GPIO_InitStruct.Pin = GPIO_PIN_5;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		hdma_tim1_up.Instance = DMA2_Stream5;
		hdma_tim1_up.Init.Channel = DMA_CHANNEL_6;
		hdma_tim1_up.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_tim1_up.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_tim1_up.Init.MemInc = DMA_MINC_ENABLE;
		hdma_tim1_up.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
		hdma_tim1_up.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
		hdma_tim1_up.Init.Mode = DMA_CIRCULAR;
		hdma_tim1_up.Init.Priority = DMA_PRIORITY_LOW;
		hdma_tim1_up.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		if (HAL_DMA_Init(&hdma_tim1_up) != HAL_OK) {
			Error_Handler();
		}
	}
}

void HAL_TIM_IC_MspInit(TIM_HandleTypeDef* htim_ic)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	if (htim_ic->Instance == TIM3) {
		__HAL_RCC_TIM3_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();

		GPIO_InitStruct.Pin = GPIO_PIN_6;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		hdma_tim3_ch1_trig.Instance = DMA1_Stream4;
		hdma_tim3_ch1_trig.Init.Channel = DMA_CHANNEL_5;
		hdma_tim3_ch1_trig.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_tim3_ch1_trig.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_tim3_ch1_trig.Init.MemInc = DMA_MINC_ENABLE;
		hdma_tim3_ch1_trig.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
		hdma_tim3_ch1_trig.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
		hdma_tim3_ch1_trig.Init.Mode = DMA_NORMAL;
		hdma_tim3_ch1_trig.Init.Priority = DMA_PRIORITY_LOW;
		hdma_tim3_ch1_trig.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_Init(&hdma_tim3_ch1_trig);

		__HAL_LINKDMA(htim_ic, hdma[TIM_DMA_ID_CC1], hdma_tim3_ch1_trig);

		HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM3_IRQn);

		HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
	}
}

static void TIM1_Init()
{
	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 1000 - 1;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 48000 - 1;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

	HAL_TIM_Base_Init(&htim1);
}

static void TIM3_Init()
{
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 48000 - 1;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = (1 << 16) - 1;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

	TIM_IC_InitTypeDef configIC = {
		.ICPolarity 	= TIM_INPUTCHANNELPOLARITY_FALLING,
		.ICSelection 	= TIM_ICSELECTION_DIRECTTI,
		.ICPrescaler 	= TIM_ICPSC_DIV2,
		.ICFilter 		= 0,
	};

/*
Base_Init대신 IC_Init을 쓴다.
둘다 TIM_Base_SetConfig을 내부적으로 호출한다.
또한 MspInit도 Base가 아닌 IC를 써야 한다.
 */
	HAL_TIM_IC_Init(&htim3);
	HAL_TIM_IC_ConfigChannel(&htim3, &configIC, TIM_CHANNEL_1);
}

static const uint32_t DDDD[2] = { GPIO_PIN_5, GPIO_PIN_5 << 16 };

static uint16_t captures[2];
static volatile uint8_t capture_done;

void TestFunction()
{
	char msg[30];

	TIM1_Init();
	TIM3_Init();


	HAL_TIM_Base_Start(&htim1);
	HAL_DMA_Start(&hdma_tim1_up, (uint32_t)DDDD, (uint32_t)&GPIOA->BSRR, 2);
	__HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_UPDATE);

	if (HAL_TIM_IC_Start_DMA(&htim3, TIM_CHANNEL_1, (uint32_t*)captures, 2) != HAL_OK) {
		Error_Handler();
	}

	while (1) {
		if (capture_done) {
			// overflow로 인해 (captures[0] > captures[1])일 때도 문제 없다.
			uint16_t diff_capture = captures[1] - captures[0];
			float frequency = HAL_RCC_GetPCLK1Freq() / (htim3.Instance->PSC + 1);
			frequency /= diff_capture;

			size_t len = sprintf(msg, "Input frequency: %.3f\r\n", frequency);
			HAL_UART_Transmit(&huart3, (uint8_t*)msg, len, HAL_MAX_DELAY);

			while (1);
		}
	}
}

void DMA1_Stream4_IRQHandler() {
	capture_done = 1;
}

#endif /* input capture */

#if 0 /* output compare */

#include "main.h"

static TIM_HandleTypeDef htim3;

static volatile uint16_t CH1_FREQ;
static volatile uint16_t CH2_FREQ;

uint16_t computePulse(const TIM_HandleTypeDef* htim, uint32_t freq)
{
	uint32_t tim_freq = HAL_RCC_GetPCLK1Freq() * 2 / (htim->Instance->PSC + 1);
	return (uint16_t)(tim_freq / freq);
}

void HAL_TIM_OC_MspInit(TIM_HandleTypeDef *htim)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	if (htim->Instance == TIM3) {
		__HAL_RCC_TIM3_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();

		/* TIM3_CH1 | TIM3_CH2 */
		GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	    HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
	    HAL_NVIC_EnableIRQ(TIM3_IRQn);
	}
}

static void TIM3_Init()
{
	TIM_OC_InitTypeDef sConfigOC;

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 2000 - 1;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = (1 << 16) - 1;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_OC_Init(&htim3);

	CH1_FREQ = computePulse(&htim3, 2); /* 25kHZ switching frequency */
	CH2_FREQ = computePulse(&htim3, 4); /* 50kHZ switching frequency */

	sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);

	sConfigOC.Pulse = 0;
	HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2);

}

static void GPIO_Init()
{
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void TestFunction()
{
	TIM3_Init();
	GPIO_Init();

	HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_1);
	HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_2);

	while (1) {
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4));
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5));
	}
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	uint16_t pulse;
	uint16_t arr = __HAL_TIM_GET_AUTORELOAD(htim);

	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
		pulse = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
		__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, (pulse + CH1_FREQ) % (arr + 1));
	}
	else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
		pulse = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
		__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_2, (pulse + CH2_FREQ) % (arr + 1));
	}
}

void TIM3_IRQHandler()
{
	HAL_TIM_IRQHandler(&htim3);
}

#endif /* output compare */






