/*
 * TEST_Timer_OnePulseMode.c
 *
 *  Created on: 2022. 9. 11.
 *      Author: JMJ
 */

#if 0

#include "main.h"

static TIM_HandleTypeDef htim3;

void HAL_TIM_OnePulse_MspInit(TIM_HandleTypeDef *htim)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	if (htim->Instance == TIM3) {
		__HAL_RCC_TIM3_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();

		/* TIM3_CH1, TIM3_CH2 */
		GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

//	    HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
//	    HAL_NVIC_EnableIRQ(TIM3_IRQn);
	}
}

static void TIM3_Init()
{
	/* TIM3 is clocked at 96MHz */

	TIM_OnePulse_InitTypeDef sConfig = { 0 };

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 48000 - 1;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 4000 - 1;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_OnePulse_Init(&htim3, TIM_OPMODE_SINGLE);

	sConfig.OCMode = TIM_OCMODE_PWM1;
	sConfig.OCPolarity = TIM_OCPOLARITY_LOW;
	sConfig.Pulse = 2000; // -1을 하지 않는다는 것에 주의!

	sConfig.ICPolarity = TIM_ICPOLARITY_RISING;
	sConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfig.ICFilter = 0;

	HAL_TIM_OnePulse_ConfigChannel(&htim3, &sConfig, TIM_CHANNEL_1, TIM_CHANNEL_2);

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

	HAL_Delay(1000);

	HAL_TIM_OnePulse_Start(&htim3, TIM_CHANNEL_1);

	HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

	while (1) {
		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
		HAL_Delay(1000);
	}
}

#endif
