/*
 * TEST_timer_MasterSlave.c
 *
 *  Created on: Aug 25, 2022
 *      Author: JMJ
 */

#if 0

//TIM_ETRPOLARITY_INVERTED         	TIM_SMCR_ETP
//TIM_ETRPOLARITY_NONINVERTED      	0x00000000U
//
//TIM_SMCR_ETP	1 << 15
//
//
//TIM_INPUTCHANNELPOLARITY_RISING		0x00000000U
//TIM_INPUTCHANNELPOLARITY_FALLING 	TIM_CCER_CC1P
//TIM_INPUTCHANNELPOLARITY_BOTHEDGE	(TIM_CCER_CC1P | TIM_CCER_CC1NP)
//
//TIM_CCER_CC1P		1 << 1
//TIM_CCER_CC1NP	1 << 3

#include "main.h"

// APB2 => 96MHz

static TIM_HandleTypeDef htim1; // master
static TIM_HandleTypeDef htim3; // slave

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
	 GPIO_InitTypeDef GPIO_InitStruct;

	if (htim->Instance == TIM1) {
		__HAL_RCC_TIM1_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();

		GPIO_InitStruct.Pin = GPIO_PIN_8;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLDOWN;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;

		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}
	else if(htim->Instance == TIM3) {
		__HAL_RCC_TIM3_CLK_ENABLE();

		HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM3_IRQn);
	}
}

static void TIM1_Init()
{
	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 1000 -1;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 24000 - 1;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;

	TIM_SlaveConfigTypeDef slave_cfg = {
		.SlaveMode = TIM_SLAVEMODE_TRIGGER, // TRGI로 카운터 시작 제어
		.InputTrigger = TIM_TS_TI1FP1,
		.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING,
		.TriggerFilter = 15, // 일종의 debouncing인가...? 뭐하는 놈이냐?
	};

	TIM_MasterConfigTypeDef master_cfg = {
		.MasterOutputTrigger = TIM_TRGO_UPDATE,
		.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE,
	};

	HAL_TIM_Base_Init(&htim1);
	HAL_TIM_SlaveConfigSynchro(&htim1, &slave_cfg);
	HAL_TIMEx_MasterConfigSynchronization(&htim1, &master_cfg);
}


static void TIM3_Init()
{
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 0;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 1;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.RepetitionCounter = 0;

	TIM_SlaveConfigTypeDef slave_cfg = {
		.SlaveMode = TIM_SLAVEMODE_EXTERNAL1, // TRGI로 카운터 시작 제어
		.InputTrigger = TIM_TS_ITR0,
	};

	HAL_TIM_Base_Init(&htim3);
	HAL_TIM_SlaveConfigSynchro(&htim3, &slave_cfg);
}

void TestFunction()
{
	TIM1_Init();
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

#endif
