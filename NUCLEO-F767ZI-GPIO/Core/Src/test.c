/*
 * test.c
 *
 *  Created on: Aug 7, 2022
 *      Author: JMJ
 */

#include "test.h"

#include "main.h"
#include "gpio.h"


#if 0
void TEST_userled123_button()
{
	uint16_t prev_pin = 0x0001U;
	uint16_t prev_btn = HAL_GPIO_ReadPin(USER_Btn_GPIO_Port, USER_Btn_Pin);

	HAL_GPIO_WritePin(GPIOB, prev_pin, GPIO_PIN_SET);

	while (1) {
		uint16_t curr_btn = HAL_GPIO_ReadPin(USER_Btn_GPIO_Port, USER_Btn_Pin);

		if (curr_btn & ~prev_btn) {
		  HAL_GPIO_WritePin(GPIOB, prev_pin, GPIO_PIN_RESET);
		  prev_pin <<= 7; // LD1, LD2, LD3가 각각 0, 7, 14에 있다는 것을 이용
		  if (!prev_pin) prev_pin = 0x0001U;
		  HAL_GPIO_WritePin(GPIOB, prev_pin, GPIO_PIN_SET);
		}

		prev_btn = curr_btn;
	}
}
#endif

#if 1
void TEST_BSRR()
{
	uint8_t toggle = 0;

	while (1) {
		((uint16_t*)&LD1_GPIO_Port->BSRR)[toggle ^= 1] = LD1_Pin;
		HAL_Delay(500);
	}
}
#endif

#if 0

/*
 * cortex-m7은 bit-banding을 지원하지 않는다. 그리고 STM32F7은 cortex-m7을 사용한다.
 */

// bit_word_addr = bit_band_alias + (byte_offset x 32) + (bit_number x 4)
// 32 => 4(word) * 8(byte bit num), 4 => 4(word) * 1(offset)
#define BIT_BAND_ADDR()


#define SRAM_BITBAND_BASE     (0x20000000)
#define SRAM_BITBAND_ALIAS    (0x22000000)

#define PERIPH_BITBAND_BASE     (0x40000000)
#define PERIPH_BITBAND_ALIAS    (0x42000000)

void TEST_GPIOBitBanding()
{

}
#endif

#if 0
void TEST_OpenDrainOutput()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	while (1) {
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
		HAL_Delay(1000);

	}
}
#endif










