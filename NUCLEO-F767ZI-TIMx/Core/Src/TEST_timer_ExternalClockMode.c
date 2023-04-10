/*
 * TEST_timer_ExternalClockMode.c
 *
 *  Created on: 2022. 8. 25.
 *      Author: JMJ
 */

#include "main.h"

#if 0 /* External Clock Mode 1 */

static TIM_HandleTypeDef htim3;

// PD2: TIM3_ETR1
// PA8: MCO(16MHz)

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

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	if(htim->Instance == TIM3)
	{
		/* USER CODE BEGIN TIM3_MspInit 0 */

		/* USER CODE END TIM3_MspInit 0 */
		/* TIM3 clock enable */
		__HAL_RCC_TIM3_CLK_ENABLE();

		__HAL_RCC_GPIOD_CLK_ENABLE();
		/**TIM3 GPIO Configuration
		PD2     ------> TIM3_ETR
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_2;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
		HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM3_IRQn);
	}
}

static void TIM3_Init()
{
	  TIM_SlaveConfigTypeDef sSlaveConfig = {0};

	  htim3.Instance = TIM3;
	  htim3.Init.Prescaler = 1000 - 1;
	  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	  htim3.Init.Period = 8000 -1;
	  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	  if (HAL_TIM_Base_Init(&htim3) != HAL_OK) Error_Handler();

	  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
	  sSlaveConfig.InputTrigger = TIM_TS_ETRF;
	  sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_NONINVERTED;
	  sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
	  sSlaveConfig.TriggerFilter = 0;
	  if (HAL_TIM_SlaveConfigSynchro(&htim3, &sSlaveConfig) != HAL_OK) Error_Handler();
}

void TestFunction()
{
	MCO_Init();
	TIM3_Init();

	HAL_TIM_Base_Start_IT(&htim3);

	while (1);
}

void TIM3_IRQHandler()
{
	if (__HAL_TIM_GET_FLAG(&htim3, TIM_FLAG_UPDATE)) {
		__HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE );
		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
	}
}

#endif /* External Clock Mode 1 */

#if 0 /* External Clock Mode 2 */

static TIM_HandleTypeDef htim3;

// PD2: TIM3_ETR2
// PA8: MCO(16MHz)

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

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	if(htim->Instance == TIM3)
	{
		__HAL_RCC_TIM3_CLK_ENABLE();

		__HAL_RCC_GPIOD_CLK_ENABLE();
		/**TIM3 GPIO Configuration
		PD2     ------> TIM3_ETR
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_2;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
		HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM3_IRQn);
	}
}

static void TIM3_Init()
{
	TIM_ClockConfigTypeDef clk_cfg = { 0, };

	// 16MHz
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 1000 - 1;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 8000 - 1;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(&htim3);

	clk_cfg.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
	clk_cfg.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
	clk_cfg.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
	clk_cfg.ClockFilter = 0;
	HAL_TIM_ConfigClockSource(&htim3, &clk_cfg);
}

void TestFunction()
{
	MCO_Init();
	TIM3_Init();

	HAL_TIM_Base_Start_IT(&htim3);

	while (1);
}

void TIM3_IRQHandler()
{
	if (__HAL_TIM_GET_FLAG(&htim3, TIM_FLAG_UPDATE)) {
		__HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE );
		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
	}
}

#endif /* External Clock Mode 2 */
