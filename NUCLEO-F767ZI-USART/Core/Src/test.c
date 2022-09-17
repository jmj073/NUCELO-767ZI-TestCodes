/*
 * test.c
 *
 *  Created on: Aug 10, 2022
 *      Author: user
 */

#include "test.h"

#include "main.h"
#include "usart.h"

#if 0 /* polling 방식 */

void TEST_UART_polling()
{
	char c;

	while (1) {
		// 연속으로 보내면 문제가 생긴다
		if (HAL_UART_Receive(&huart3, (uint8_t*)&c, 1, HAL_MAX_DELAY) != HAL_OK) {
			HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
			continue;
		}

		if (c == 'h') {
			HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
			HAL_UART_Transmit(&huart3, (const uint8_t*)"on!\r\n", 5, HAL_MAX_DELAY);
		}
		else if (c == 'l') {
			HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
			HAL_UART_Transmit(&huart3, (const uint8_t*)"off!\r\n", 6, HAL_MAX_DELAY);
		}
	}

}

#endif /* polling 방식 */

#if 0 /* interrupt 방식 */

static volatile int rx_cplt = 0;
static volatile int tx_cplt = 0;

void TEST_UART_interrupt()
{
	uint8_t c;
	uint32_t prev_ms = HAL_GetTick();


	while (1) {
		HAL_UART_Receive_IT(&huart3, &c, 1);

		do {
			uint32_t curr_ms = HAL_GetTick();
			if (curr_ms - prev_ms >= 500) {
				prev_ms += 500;
				HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
			}
		} while (!rx_cplt);
		rx_cplt = 0; // RxCallback과 배타적이기 때문에 상관 없다

		if (c == 'h') {
			HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
			HAL_UART_Transmit_IT(&huart3, (const uint8_t*)"on!\r\n", 5);
		}
		else if (c == 'l') {
			HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
			HAL_UART_Transmit_IT(&huart3, (const uint8_t*)"off!\r\n", 6);
		}
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3) {

	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3) {
		rx_cplt = 1;
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3) {
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
	}
}

#endif /* interrupt 방식 */

#if 0 /* DMA 방식 */
// normal O, circular X

#include "string.h"

#define TX_MSG "hello, world!\r\n"

void TEST_UART_DMA()
{
	HAL_UART_Transmit_DMA(&huart3, (const uint8_t*)TX_MSG, strlen(TX_MSG));

	while (1);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3) {
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
	}
}

void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3) {
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3) {
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
	}
}

#endif /* DMA 방식 */


#if 0 /* DMA 방식 (ping-pong buffer) */

#include "string.h"

// [36][46]
static const char* TX_MSG =
// 'The Fat Rat'의 we'll meet again
"We'll meet again                            \r\n"
"Don't know where, don't know when           \r\n"
"But I know we'll meet again                 \r\n"
"Some sunny day                              \r\n"
"Keep smiling through                        \r\n"
"Just like you always do                     \r\n"
"'Til the blue skies drive the dark clouds   \r\n"
"Far away                                    \r\n"
"So will you, please, say hello              \r\n"
"To the folks that I know?                   \r\n"
"Tell them I won't be long                   \r\n"
"They'll be happy to know                    \r\n"
"That as you saw me go                       \r\n"
"I was singing this song                     \r\n"
"We'll meet again                            \r\n"
"Don't know where, don't know when           \r\n"
"But I know we'll meet again                 \r\n"
"Some sunny day                              \r\n"
"We'll meet again                            \r\n"
"Don't know where, don't know when           \r\n"
"But I know we'll meet again                 \r\n"
"Some sunny day                              \r\n"
"Keep smiling through                        \r\n"
"Just like you always do                     \r\n"
"'Til the blue skies drive the dark clouds   \r\n"
"Far away                                    \r\n"
"So will you, please, say hello              \r\n"
"To the folks that I know?                   \r\n"
"Tell them I won't be long                   \r\n"
"They'll be happy to know                    \r\n"
"That as you saw me go                       \r\n"
"I was singing this song                     \r\n"
"We'll meet again                            \r\n"
"Don't know where, don't know when           \r\n"
"But I know we'll meet again                 \r\n"
"Some sunny day                              \r\n"
;

#define BUF_SIZE 46

#define BUF_EMPTY	0
#define BUF_FULL	1

static volatile uint8_t filled_flag[2];
static uint8_t curr_buf;

static uint8_t BUF[2][BUF_SIZE];

static void UART_TX(const uint8_t data[BUF_SIZE])
{
	while (filled_flag[curr_buf] == BUF_FULL); // dma 에 의해서 빌 때까지

	memcpy(BUF[curr_buf], data, BUF_SIZE); // 데이터 복사

	filled_flag[curr_buf] = BUF_FULL;
	curr_buf ^= 1; // swap buffer

	HAL_UART_DMAResume(&huart3);
}


void TEST_UART_DMA()
{
	memcpy(BUF[0], TX_MSG, BUF_SIZE); // 데이터 복사
	curr_buf = 1;
	filled_flag[0] = 1;

	HAL_UART_Transmit_DMA(&huart3, (const uint8_t*)BUF, sizeof(BUF));

	for (size_t line = 1; line < 36; line++) {
		UART_TX((const uint8_t*)TX_MSG + line * BUF_SIZE);
	}

	while (1);
}


void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3) {
		filled_flag[0] = BUF_EMPTY;
		if (filled_flag[1] == BUF_EMPTY) {
			HAL_UART_DMAPause(huart);
		}
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3) {
		filled_flag[1] = BUF_EMPTY;
		if (filled_flag[0] == BUF_EMPTY) {
			HAL_UART_DMAPause(huart);
		}
	}
}



void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3) {
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
	}
}

#endif /* DMA 방식 */

#if 1 /* DMA 방식 2 */

#include "uart_with_dma.h"

static const char* TX_MSG =
// 'The Fat Rat'의 we'll meet again
"We'll meet again                            \r\n"
"Don't know where, don't know when           \r\n"
"But I know we'll meet again                 \r\n"
"Some sunny day                              \r\n"
"Keep smiling through                        \r\n"
"Just like you always do                     \r\n"
"'Til the blue skies drive the dark clouds   \r\n"
"Far away                                    \r\n"
"So will you, please, say hello              \r\n"
"To the folks that I know?                   \r\n"
"Tell them I won't be long                   \r\n"
"They'll be happy to know                    \r\n"
"That as you saw me go                       \r\n"
"I was singing this song                     \r\n"
"We'll meet again                            \r\n"
"Don't know where, don't know when           \r\n"
"But I know we'll meet again                 \r\n"
"Some sunny day                              \r\n"
"We'll meet again                            \r\n"
"Don't know where, don't know when           \r\n"
"But I know we'll meet again                 \r\n"
"Some sunny day                              \r\n"
"Keep smiling through                        \r\n"
"Just like you always do                     \r\n"
"'Til the blue skies drive the dark clouds   \r\n"
"Far away                                    \r\n"
"So will you, please, say hello              \r\n"
"To the folks that I know?                   \r\n"
"Tell them I won't be long                   \r\n"
"They'll be happy to know                    \r\n"
"That as you saw me go                       \r\n"
"I was singing this song                     \r\n"
"We'll meet again                            \r\n"
"Don't know where, don't know when           \r\n"
"But I know we'll meet again                 \r\n"
"Some sunny day                              \r\n"
;

void TEST_UART_DMA2()
{
	UART_StartDMA();

	UART_puts(TX_MSG);
	while (1);
}

#endif /* DMA 방식 2 */





















