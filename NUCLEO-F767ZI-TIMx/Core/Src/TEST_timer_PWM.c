/*
 * TEST_timer_PWM.c
 *
 *  Created on: Sep 10, 2022
 *      Author: JMJ
 */

#if 0 /* led dimming */

#include "main.h"

static TIM_HandleTypeDef htim3;

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	if (htim->Instance == TIM3) {
		__HAL_RCC_TIM3_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();

		/* TIM3_CH3(LD1_PIN, PB0) */
		GPIO_InitStruct.Pin = GPIO_PIN_0;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

//	    HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
//	    HAL_NVIC_EnableIRQ(TIM3_IRQn);
	}
}

static void TIM3_Init()
{
	TIM_OC_InitTypeDef sConfigOC = { 0 };

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 9600 - 1;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 100 - 1;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_PWM_Init(&htim3);

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3);

}

void TestFunction()
{
	TIM3_Init();

	HAL_TIM_OC_Start(&htim3, TIM_CHANNEL_3);

	while (1) {
		for (int brightness = 100; brightness >= 0; brightness--) {
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, brightness);
			HAL_Delay(10);
		}
		for (int brightness = 0; brightness <= 100; brightness++) {
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, brightness);
			HAL_Delay(10);
		}
	}
}

#endif /* led dimming */
