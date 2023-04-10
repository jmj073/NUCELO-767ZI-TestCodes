/*
 * TEST_timer_UpdateEvent.c
 *
 *  Created on: Aug 26, 2022
 *      Author: user
 */



#include "main.h"

#if 0 /* UDIS(Update Disable) 테스트 */

static TIM_HandleTypeDef htim7;

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM7) {
		__HAL_RCC_TIM7_CLK_ENABLE();

		HAL_NVIC_SetPriority(TIM7_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM7_IRQn);
	}
}

static void TIM7_Init()
{
	// 96MHz가 공급된다
	htim7.Instance = TIM7;
	htim7.Init.Prescaler = 1000 - 1;
	htim7.Init.Period = 48000 - 1;

	HAL_TIM_Base_Init(&htim7);
}

void TestFunction()
{
	TIM7_Init();

	HAL_TIM_Base_Start_IT(&htim7);

	htim7.Instance->CR1 |= TIM_CR1_UDIS;

	while (1) {
		// UG를 set하는 코드를 주석처리하면 LED2가 켜지지만
		// 주석 처리를 하지 않으면 켜지지 않는다
		htim7.Instance->EGR |= TIM_EGR_UG;
		if (htim7.Instance->CNT >= 40000) {
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
		}
//		HAL_Delay(200);
	}
}

void TIM7_IRQHandler()
{
	if (__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE)) {
		__HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE);

		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
	}
}


#endif /* UDIS(Update Disable) 테스트 */

#if 0 /* URS(Update Request Source) 테스트 */

static TIM_HandleTypeDef htim7;

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM7) {
		__HAL_RCC_TIM7_CLK_ENABLE();

		HAL_NVIC_SetPriority(TIM7_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM7_IRQn);
	}
}

static void TIM7_Init()
{
	// 96MHz가 공급된다
	htim7.Instance = TIM7;
	htim7.Init.Prescaler = 1000 - 1;
	htim7.Init.Period = 48000 - 1;

	HAL_TIM_Base_Init(&htim7);
}

void TestFunction()
{
	TIM7_Init();

	HAL_TIM_Base_Start_IT(&htim7);

/*
	URS가 UG의 update interrupt 생성을 막고 있기 때문에
	깜박임 이상증상(?)이 나타나지 않는다.
	또한 URS overflow/underflow 를 통한 interrupt 생성은 허용한다.
	DMA request 에도 똑같이 적용된다.
*/
	htim7.Instance->CR1 |= TIM_CR1_URS;

	while (1) {
/*
		UG 비트를 set 하는 것은 카운터를 초기화 하는
		효과가 있기 때문에 overflow 를 위해서는 충분히
		기다려 줘야 한다.이 테스트에서 overflow 는
		500ms마다 일어난다.
*/
		htim7.Instance->EGR |= TIM_EGR_UG;
		HAL_Delay(1000);
	}
}

void TIM7_IRQHandler()
{
	if (__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE)) {
		__HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE);

		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
	}
}


#endif

#if 0 /* URS(Update Request Source) 테스트 2 */
/*
TIM1의 TRGO(trigger out)을 update event로 설정 함으로써
URS는 update event의 generation과 관계가 없다는 것을 알 수 있다
 */

// master로 TIM1을 쓴 이유는 TIM3의
// ITR 매핑에 basic 타이머가 없었기 때문이다
static TIM_HandleTypeDef htim1;
static TIM_HandleTypeDef htim3;

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM1) {
		__HAL_RCC_TIM1_CLK_ENABLE();

//		HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 0, 0);
//		HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
	}
	else if (htim->Instance == TIM3) {
		__HAL_RCC_TIM3_CLK_ENABLE();

		HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM3_IRQn);
	}
}

static void TIM1_Init()
{
	// 96MHz가 공급된다
	htim1.Instance = TIM1;
	htim1.Init.Prescaler = (1 << 16) - 1;
	htim1.Init.Period = (1 << 16) - 1;

// TODO: master configuration을 하지 않아도
// 예상하던대로 동작한 이유를 찾아보세요
	TIM_MasterConfigTypeDef master_cfg = {
		.MasterOutputTrigger = TIM_TRGO_UPDATE,
		.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE,
	};

	HAL_TIM_Base_Init(&htim1);
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
		.SlaveMode = TIM_SLAVEMODE_EXTERNAL1,
		.InputTrigger = TIM_TS_ITR0,
	};

	HAL_TIM_Base_Init(&htim3);
	HAL_TIM_SlaveConfigSynchro(&htim3, &slave_cfg);
}

void TestFunction()
{
	TIM1_Init();
	TIM3_Init();

	HAL_TIM_Base_Start(&htim1);
	HAL_TIM_Base_Start_IT(&htim3);

	htim1.Instance->CR1 |= TIM_CR1_URS;

	while (1) {
		// HAL_TIM_GenerateEvent사용하여 수행할 수도 있다.
		htim1.Instance->EGR |= TIM_EGR_UG;
		HAL_Delay(250); // 0.5초마다 LED1이 토글된다.
	}
}

void TIM3_IRQHandler()
{
	if (__HAL_TIM_GET_FLAG(&htim3, TIM_FLAG_UPDATE)) {
		__HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE);

		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
	}
}


#endif
