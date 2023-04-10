/*
 * test.c
 *
 *  Created on: Aug 10, 2022
 *      Author: JMJ
 *      Brief : EXTI 관련 test 코드. 하지만 EXTI만
 *      있는 것은 아니고 iterrupt 전체라고 봐야 할 듯...
 */

#include "test.h"

#include "main.h"
#include "gpio.h"

#if 0 /* EXTI test */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == USER_Btn_Pin) {
		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
	}
}
#endif /* EXTI test */


#if 0 /* preemption test1 */
// 디바운싱이 없는것에 주의
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_0) { // preemption priority: 0
		for (uint32_t i = 0; i < 10; i++) {
			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
			/*
			HAL_Delay는 내부적으로 HAL_GetTick을 사용하는데,
			HAL_GetTick은 타이머 인터럽트를 사용하므로
			HAL_Delay를 사용할 수 없다
			 */
			for (volatile uint32_t i = 0; i < 1000000; i++);
		}
	}
	else if (GPIO_Pin == GPIO_PIN_3) { // preemption priority: 1
		for (uint32_t i = 0; i < 20; i++) {
			HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
			for (volatile uint32_t i = 0; i < 1000000; i++);
		}
	}
}

/*

+ 의문점: EXTI0이랑 EXTI3이랑 점멸 속도가 다르다. 왜 그런 걸까?

 */

#endif /* preemption test1 */

#if 0 /* preemption test2 */
// 디바운싱이 없는것에 주의
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_0) { // preemption priority: 1
		for (uint32_t i = 0; i < 10; i++) {
			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
			// timer 인터럽트보다 EXTI 인터럽트 우선순위를 낮춰서
			// HAL_Delay를 사용 가능하게 만든다
			// timer interrupt 의 preemption priority 는 0
			HAL_Delay(500);
		}
	}
	else if (GPIO_Pin == GPIO_PIN_3) { // preemption priority: 2
		for (uint32_t i = 0; i < 20; i++) {
			HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
			HAL_Delay(500);
		}
	}
}
#endif /* preemption test2 */

#ifdef TEST_DIVIDED_BY_ZERO /* divided by zero(EXTI는 아니지만...) */
// [출처] [STM32 HAL] Interrupt#2|작성자 이지훈
// stm32Xnxx_it.c 파일에 HardFault_Handler도 살펴보세용~

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == USER_Btn_Pin)
	{
		/* Divide By Zero Enabled  */
		SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;

		/* 강제로 Hard Fault 를 발생 시킨다 */
		volatile int i = 100;
		volatile int j = 0;
		volatile int k = i / j;

		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
	}
}

void FaultHandler(uint32_t* FaultArgs)
{
  volatile uint32_t r0;
  volatile uint32_t r1;
  volatile uint32_t r2;
  volatile uint32_t r3;
  volatile uint32_t r12;
  volatile uint32_t lr;
  volatile uint32_t pc;
  volatile uint32_t psr;

  r0 = FaultArgs[0];
  r1 = FaultArgs[1];
  r2 = FaultArgs[2];
  r3 = FaultArgs[3];
  r12 = FaultArgs[4];
  lr = FaultArgs[5];
  pc = FaultArgs[6];
  psr = FaultArgs[7];

  /* 레지스터 값들을 저장하거나 확인하는 코드 추가 */

  /* 시스템을 재시작 하거나 대기한다. */
  HAL_NVIC_SystemReset();
}

#endif /* divided by zero */


#if 0 /* active register(IABR) test */
// IRQ 활성화 해야한 다는 것도 잊지 마시고,
// 핀을 floating 상태로 두지도 마시라

void TEST_IABR()
{
	EXTI->SWIER = (1 << 0); // interrupt를 pending 상태로 설정
	while (1);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_0) { // preemption priority: 1
		EXTI->SWIER = (1 << 3); // interrupt를 pending 상태로 설정
		while(1);
	}
	else if (GPIO_Pin == GPIO_PIN_3) { // preemption priority: 0
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, HAL_NVIC_GetActive(EXTI0_IRQn));
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, HAL_NVIC_GetActive(EXTI3_IRQn));
	}
}

#endif /* active register(IABR) test */

#if 1 /* priority grouping test */

void TEST_grouping()
{
	HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 0);
	HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);

	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);

	EXTI->SWIER = (1 << 0); // interrupt를 pending 상태로 설정
	while (1);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_0) { // priority: 0x40
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
		EXTI->SWIER = (1 << 3); // interrupt를 pending 상태로 설정
		while(1);
	}
	else if (GPIO_Pin == GPIO_PIN_3) { // priority: 0x00
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
	}
}

#endif /* priority grouping test */



















