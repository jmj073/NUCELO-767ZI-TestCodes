/*
 * [H]TEST_I2C_MASTER.c
 *
 *  Created on: Jan 9, 2023
 *      Author: JMJ
 */

/*
 * 통신을 위해 slave로 mpu6050을 사용한다.
 */



#include "main.h"

static I2C_HandleTypeDef hi2c1;

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle) {

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

	if (i2cHandle->Instance==I2C1) {

		/* Initializes the peripherals clock */
		PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
		PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
		HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

		__HAL_RCC_GPIOB_CLK_ENABLE();
		/**I2C1 GPIO Configuration
		PB8     ------> I2C1_SCL
		PB9     ------> I2C1_SDA
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		/* I2C1 clock enable */
		__HAL_RCC_I2C1_CLK_ENABLE();

	}
}

static
void I2C_Init() {
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x20303E5D;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	HAL_I2C_Init(&hi2c1);

//	/* Configure Analog filter */
//	HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE);
//
//	/* Configure Digital filter */
//	HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0);
}

void TestFunction() {
	I2C_Init();

}
