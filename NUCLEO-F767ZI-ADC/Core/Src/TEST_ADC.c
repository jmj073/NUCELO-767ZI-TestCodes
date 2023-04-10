/*
 * TEST_ADC.c
 *
 *  Created on: 2022. 9. 20.
 *      Author: JMJ
 */

#if 0 /* polling mode with internal temperature sensor */
// single channel, continuous

#include "usart.h"
#include <stdio.h>

static ADC_HandleTypeDef hadc1;

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance == ADC1)
  {
    __HAL_RCC_ADC1_CLK_ENABLE();
  }

}

static void ADC1_Init()
{
	  ADC_ChannelConfTypeDef sConfig = { 0 };

	  hadc1.Instance = ADC1;
	  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
	  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	  hadc1.Init.ScanConvMode = DISABLE;
	  hadc1.Init.ContinuousConvMode = ENABLE;
	  hadc1.Init.DiscontinuousConvMode = DISABLE;
	  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	  // single channel mode는 rank수를 1과 같다고 가정하기 때문에 NbrOfConversion은 무의미
	  hadc1.Init.NbrOfConversion = 1;
	  hadc1.Init.DMAContinuousRequests = DISABLE;
	  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
	  if (HAL_ADC_Init(&hadc1) != HAL_OK) {
		  Error_Handler();
	  }

	  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
	  sConfig.Rank = 1;
	  // 480으로 설정하는 이유는 1°C의 정확도를 위해선 최소 10us의 샘플링 시간이 필요하기 때문
	  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
	  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		  Error_Handler();
	  }
}

void TestFunction()
{
//	HAL_UART_Transmit(&huart3, (uint8_t*)"hello!", 6, HAL_MAX_DELAY);
//	while (1);

	ADC1_Init();

	HAL_ADC_Start(&hadc1);

	while (1) {
		char msg[64];

		HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);

		uint16_t raw_value = HAL_ADC_GetValue(&hadc1);
		float temp = ((float)raw_value) / ((1 << 12) - 1) * 3300;
		// temperature sensor electrical characteristics 참고
		temp = (temp - 760.0) / 2.5 + 25;

		int sz = sprintf(msg, "ADC Raw: %hu\r\n", raw_value);
		HAL_UART_Transmit(&huart3, (uint8_t*)msg, sz, HAL_MAX_DELAY);

		sz = sprintf(msg, "Temperature: %f\r\n", temp);
		HAL_UART_Transmit(&huart3, (uint8_t*)msg, sz, HAL_MAX_DELAY);
	}
}

#endif /* polling mode with internal temperature sensor */

#if 0 /* interrupt mode */
// interrupt를 사용하는 것을 빼고는 위와 같다.

#include "usart.h"
#include <stdio.h>

static ADC_HandleTypeDef hadc1;

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance == ADC1)
  {
    __HAL_RCC_ADC1_CLK_ENABLE();

    HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ADC_IRQn);
  }

}

static void ADC1_Init()
{
	  ADC_ChannelConfTypeDef sConfig = { 0 };

	  hadc1.Instance = ADC1;
	  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
	  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	  hadc1.Init.ScanConvMode = DISABLE;
	  hadc1.Init.ContinuousConvMode = ENABLE;
	  hadc1.Init.DiscontinuousConvMode = DISABLE;
	  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	  hadc1.Init.NbrOfConversion = 1;
	  hadc1.Init.DMAContinuousRequests = DISABLE;
	  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
	  if (HAL_ADC_Init(&hadc1) != HAL_OK) {
		  Error_Handler();
	  }

	  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
	  sConfig.Rank = 1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
	  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		  Error_Handler();
	  }
}

void TestFunction()
{
	ADC1_Init();

	HAL_ADC_Start_IT(&hadc1);

	while (1);
}

void ADC_IRQHandler()
{
	HAL_ADC_IRQHandler(&hadc1);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if (hadc->Instance == ADC1) {
		char msg[64];

		uint16_t raw_value = HAL_ADC_GetValue(&hadc1);
		float temp = ((float)raw_value) / ((1 << 12) - 1) * 3300;
		// temperature sensor electrical characteristics 참고
		temp = (temp - 760.0) / 2.5 + 25;

		int sz = sprintf(msg, "ADC Raw: %hu\r\n", raw_value);
		HAL_UART_Transmit(&huart3, (uint8_t*)msg, sz, HAL_MAX_DELAY);

		sz = sprintf(msg, "Temperature: %f\r\n", temp);
		HAL_UART_Transmit(&huart3, (uint8_t*)msg, sz, HAL_MAX_DELAY);
	}
}

#endif /* interrupt mode */

#if 0 /* dma mode */
// multi-channel, single

#include "usart.h"
#include <stdio.h>

static ADC_HandleTypeDef hadc1;
static DMA_HandleTypeDef hdma_adc1;

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance == ADC1)
  {
    __HAL_RCC_ADC1_CLK_ENABLE();

	__HAL_RCC_DMA2_CLK_ENABLE();

	HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

    hdma_adc1.Instance = DMA2_Stream0;
    hdma_adc1.Init.Channel = DMA_CHANNEL_0;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_NORMAL;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_adc1);

    __HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc1);

    HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ADC_IRQn);
  }

}

static void ADC1_Init()
{
	  ADC_ChannelConfTypeDef sConfig = { 0 };

	  hadc1.Instance = ADC1;
	  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
	  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	  hadc1.Init.ScanConvMode = ENABLE;
	  hadc1.Init.ContinuousConvMode = DISABLE;
	  hadc1.Init.DiscontinuousConvMode = DISABLE;
	  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	  hadc1.Init.NbrOfConversion = 3;
	  hadc1.Init.DMAContinuousRequests = DISABLE;
	  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
	  HAL_ADC_Init(&hadc1);

	  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
	  sConfig.Rank = 1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
	  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

	  sConfig.Rank = 2;
	  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

	  sConfig.Rank = 3;
	  HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

static volatile uint8_t convCompleted;
static uint16_t rawValues[3];

void TestFunction()
{
	ADC1_Init();

	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)rawValues, 3);

	while (!convCompleted);

	HAL_ADC_Stop_DMA(&hadc1);

	for (uint8_t i = 0; i < hadc1.Init.NbrOfConversion; i++) {
		char msg[64];

		uint16_t raw_value = rawValues[i];
		float temp = ((float)raw_value) / ((1 << 12) - 1) * 3300;
		// temperature sensor electrical characteristics 참고
		temp = (temp - 760.0) / 2.5 + 25;

		int sz = sprintf(msg, "%d ADC Raw: %hu\r\n", (int) i, raw_value);
		HAL_UART_Transmit(&huart3, (uint8_t*)msg, sz, HAL_MAX_DELAY);

		sz = sprintf(msg, "Temperature: %f\r\n", temp);
		HAL_UART_Transmit(&huart3, (uint8_t*)msg, sz, HAL_MAX_DELAY);
	}

	while (1);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if (hadc->Instance == ADC1) {
		convCompleted = 1;
	}
}

void ADC_IRQHandler()
{
	HAL_ADC_IRQHandler(&hadc1);
}

void DMA2_Stream0_IRQHandler() {
  HAL_DMA_IRQHandler(&hdma_adc1);
}

#endif /* dma mode */

#if 0 /* external trigger(timer-driven) conversion */

#include "usart.h"
#include <stdio.h>

static ADC_HandleTypeDef hadc1;
static DMA_HandleTypeDef hdma_adc1;
static TIM_HandleTypeDef htim2;

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance == ADC1)
  {
    __HAL_RCC_ADC1_CLK_ENABLE();
	__HAL_RCC_DMA2_CLK_ENABLE();

	HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

    hdma_adc1.Instance = DMA2_Stream0;
    hdma_adc1.Init.Channel = DMA_CHANNEL_0;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_adc1);

    __HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc1);

    HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ADC_IRQn);
  }

}

static void ADC1_Init()
{
	  ADC_ChannelConfTypeDef sConfig = { 0 };

	  hadc1.Instance = ADC1;
	  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
	  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	  hadc1.Init.ScanConvMode = ENABLE; // CR1_SCAN
	  hadc1.Init.ContinuousConvMode = DISABLE; // CR2_CONT
	  hadc1.Init.DiscontinuousConvMode = DISABLE; // CR1_DISCEN
	  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
	  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIG2_T2_TRGO;
	  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	  hadc1.Init.NbrOfConversion = 3;
	  hadc1.Init.DMAContinuousRequests = ENABLE;
	  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
	  HAL_ADC_Init(&hadc1);

	  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
	  sConfig.Rank = 1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
	  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

	  sConfig.Rank = 2;
	  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

	  sConfig.Rank = 3;
	  HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM2) {
		__HAL_RCC_TIM2_CLK_ENABLE();
	}
}

static void TIM2_Init()
{
	  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	  TIM_MasterConfigTypeDef sMasterConfig = {0};

	  htim2.Instance = TIM2;
	  htim2.Init.Prescaler = 48000 - 1;
	  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	  htim2.Init.Period = 2000 - 1;
	  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	  HAL_TIM_Base_Init(&htim2);

	  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	  HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);

	  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	  HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);
}

static volatile uint8_t convCompleted;
static uint16_t rawValues[3];

void TestFunction()
{
	ADC1_Init();
	TIM2_Init();

	HAL_TIM_Base_Start(&htim2);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)rawValues, 3);

	while (1) {
		while (!convCompleted);

		for (uint8_t i = 0; i < hadc1.Init.NbrOfConversion; i++) {
			char msg[64];

			// temperature sensor electrical characteristics 참고
			uint16_t raw_value = rawValues[i];
			float temp = ((float)raw_value) / ((1 << 12) - 1) * 3300;
			temp = (temp - 760.0) / 2.5 + 25;

			int sz = sprintf(msg, "%d ADC Raw: %hu\r\n", (int)i, raw_value);
			HAL_UART_Transmit(&huart3, (uint8_t*)msg, sz, HAL_MAX_DELAY);

			sz = sprintf(msg, "Temperature: %f\r\n", temp);
			HAL_UART_Transmit(&huart3, (uint8_t*)msg, sz, HAL_MAX_DELAY);
		}

		convCompleted = 0;
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if (hadc->Instance == ADC1) {
		convCompleted = 1;
	}
}

void ADC_IRQHandler()
{
	HAL_ADC_IRQHandler(&hadc1);
}

void DMA2_Stream0_IRQHandler() {
  HAL_DMA_IRQHandler(&hdma_adc1);
}

#endif /* external trigger(timer-driven) conversion */








