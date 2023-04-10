/*
 * TEST_timer_with_DMA.c
 *
 *  Created on: 2022. 8. 23.
 *      Author: JMJ
 */

//! "stm32f7xx_hal_conf.h"에서 HAL_TIM_MODULE_ENABLED 주석을 풀어야함

#if 0

#include "main.h"

// DMA1을 사용하였기 때문에 동작을 하지 않는다.
// DMA의 port가 GPIO 주변장치에 포트 연결이 되어 있지 않은 듯 하다

static DMA_HandleTypeDef hdma_tim7_up;
static TIM_HandleTypeDef htim7;

static void DMA_Init()
{
	hdma_tim7_up.Instance = DMA1_Stream4;
	hdma_tim7_up.Init.Channel = DMA_CHANNEL_1;
	hdma_tim7_up.Init.Direction = DMA_MEMORY_TO_PERIPH;
	hdma_tim7_up.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_tim7_up.Init.MemInc = DMA_MINC_ENABLE;
	hdma_tim7_up.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	hdma_tim7_up.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
	hdma_tim7_up.Init.Mode = DMA_CIRCULAR;
	hdma_tim7_up.Init.Priority = DMA_PRIORITY_LOW;

	hdma_tim7_up.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

	if (HAL_DMA_Init(&hdma_tim7_up) != HAL_OK) {
		Error_Handler();
	}
}

static void TIM7_Init()
{
	// 96MHz
	htim7.Instance = TIM7;
	htim7.Init.Prescaler = 48000 - 1;
	htim7.Init.Period = 1000 - 1;

	__HAL_RCC_TIM7_CLK_ENABLE();
	HAL_TIM_Base_Init(&htim7);
}

void TestFunction() {
	static const uint32_t data[2] = { LD1_Pin, LD1_Pin << 16 };

	TIM7_Init();
	DMA_Init();

	HAL_TIM_Base_Start(&htim7);
	HAL_DMA_Start(&hdma_tim7_up, (uint32_t)data, (uint32_t)&LD1_GPIO_Port->BSRR, 2);
	__HAL_TIM_ENABLE_DMA(&htim7, TIM_DMA_UPDATE);

	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
	while (1);
}

#endif

#if 0

#include "main.h"

static DMA_HandleTypeDef hdma_tim1_up;
static TIM_HandleTypeDef htim1;

static void DMA_Init()
{
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

static void TIM1_Init()
{
	// 96MHz
	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 48000 - 1;
	htim1.Init.Period = 1000 - 1;

	__HAL_RCC_TIM1_CLK_ENABLE();
	HAL_TIM_Base_Init(&htim1);
}

void TestFunction() {
	static const uint32_t data[2] = { LD1_Pin, LD1_Pin << 16 };

	TIM1_Init();
	DMA_Init();

	HAL_TIM_Base_Start(&htim1);
	HAL_DMA_Start(&hdma_tim1_up, (uint32_t)data, (uint32_t)&LD1_GPIO_Port->BSRR, 2);
	__HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_UPDATE);

	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
	while (1);
}

#endif

#if 0

#include "tim.h"

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM3) {
		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
	}
}

void TestFunction()
{
	HAL_TIM_Base_Start_IT(&htim3);

	while (1);
}

#endif




