/*
 * TEST_timer_Encoder.c
 *
 *  Created on: Sep 15, 2022
 *      Author: JMJ
 */

#if 0

#include "usart.h"
#include <stdio.h>

static TIM_HandleTypeDef htim3;

void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef* htim)
{
	if (htim->Instance == TIM3) {
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
}

static void TIM3_Init()
{
	  TIM_Encoder_InitTypeDef sEncoderConfig;

	  htim3.Instance = TIM3;
	  htim3.Init.Prescaler = 0;
	  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	  htim3.Init.Period = 65535;

	  sEncoderConfig.EncoderMode = TIM_ENCODERMODE_TI1;

	  sEncoderConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
	  sEncoderConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
	  sEncoderConfig.IC1Prescaler = TIM_ICPSC_DIV1;
	  sEncoderConfig.IC1Filter = 15;

	  sEncoderConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
	  sEncoderConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
	  sEncoderConfig.IC2Prescaler = TIM_ICPSC_DIV1;
	  sEncoderConfig.IC2Filter = 15;

	  HAL_TIM_Encoder_Init(&htim3, &sEncoderConfig);
}

void TestFunction()
{
	char buf[64];

	TIM3_Init();

	 HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);

	while (1) {
		size_t sz = sprintf(buf, "%hu\r\n", (uint16_t)TIM3->CNT);
		HAL_UART_Transmit(&huart3, (uint8_t*)buf, sz, HAL_MAX_DELAY);

		HAL_Delay(500);
	}
}

#endif
