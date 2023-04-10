/*
 * test.c
 *
 *  Created on: Aug 17, 2022
 *      Author: JMJ
 *
 */


#include "main.h"

#if 0 /* DMA init template */

static void HAL_UART3_DMAInit()
{

    hdma_usart3_tx.Instance = DMA1_Stream3;
    hdma_usart3_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart3_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_usart3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart3_tx.Init.Mode = DMA_NORMAL;
    hdma_usart3_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart3_tx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_usart3_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_1QUARTERFULL;
    hdma_usart3_tx.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_usart3_tx.Init.PeriphBurst = DMA_PBURST_SINGLE;
    if (HAL_DMA_Init(&hdma_usart3_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart3_tx);
}

#endif


DMA_HandleTypeDef hdma_usart3_tx;


#if 0 /* polling mode */

#include "usart.h"
#include "string.h"

static const char* MSG = "It's a beautiful night. We're looking for something dumb to do.\r\n";

static void HAL_UART3_DMAInit()
{

    hdma_usart3_tx.Instance = DMA1_Stream3;
    hdma_usart3_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart3_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart3_tx.Init.Mode = DMA_NORMAL;
    hdma_usart3_tx.Init.Priority = DMA_PRIORITY_LOW;

// packing/unpacking 테스트
/* test 1 */
// 1글자씩 건너뛰어서 나옴
    hdma_usart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_usart3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
/* test 2 */
// 앞쪽 반만 나옴
//    hdma_usart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
//    hdma_usart3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;

    hdma_usart3_tx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_usart3_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_1QUARTERFULL;
    hdma_usart3_tx.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_usart3_tx.Init.PeriphBurst = DMA_PBURST_SINGLE;

    if (HAL_DMA_Init(&hdma_usart3_tx) != HAL_OK)
    {
      Error_Handler();
    }
}

void TEST_DMA()
{
	HAL_UART3_DMAInit();

/*
stlen(MSG)를 반으로 나눈 이유는 전송 개수는
"DMA request가 제공되면 전송 크기 값은 전송된 데이터의 양만큼 감소"
하기 때문에 만약 반으로 나누지 않는다면
test1에서는 경계값 너머를 접근하게 된다
(half word(2byte)씩 전송하기 때문에 전송양이 반으로 줄어듦).
하지만 반으로 나눔으로써 test2에서는 앞쪽 반만 나오게 되는 것이다.

참고로 위 따옴표 안에 들어 있는 문장은 'flow controller'가
DMA일때에 해대서 말하는 것이다.
*/
	HAL_DMA_Start(&hdma_usart3_tx, (uint32_t)MSG, (uint32_t)&huart3.Instance->TDR, strlen(MSG) / 2);

	ATOMIC_SET_BIT(huart3.Instance->CR3, USART_CR3_DMAT);
	HAL_DMA_PollForTransfer(&hdma_usart3_tx, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
	ATOMIC_CLEAR_BIT(huart3.Instance->CR3, USART_CR3_DMAT);

	HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);

	while (1);
}

#endif /* polling mode */


#if 0 /* interrupt mode */

#include "usart.h"
#include "string.h"

extern DMA_HandleTypeDef hdma_usart3_tx;

static const char* MSG = "It's a beautiful night. We're looking for something dumb to do.\r\n";

static void dma_tx_complete(DMA_HandleTypeDef* hdma)
{
	if (hdma->Instance == DMA1_Stream3) {
		huart3.Instance->CR3 &=  ~USART_CR3_DMAT;
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
	}

}

void TEST_DMA()
{
	hdma_usart3_tx.XferCpltCallback = dma_tx_complete;

	HAL_DMA_Start_IT(&hdma_usart3_tx, (uint32_t)MSG, (uint32_t)&huart3.Instance->TDR, strlen(MSG));
	huart3.Instance->CR3 |= USART_CR3_DMAT;

	while (1);
}

#endif /* interrupt mode */

#if 0 /* multi buffer */

void TEST_DMA()
{
	HAL_DMAEx_ChangeMemory
}

#endif /* multi buffer */

#if 0 /* DMA ping-pong buffer */

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




#endif /* DMA 방식 */


#if 1 /* polling mode */

#include "usart.h"
#include "string.h"

DMA_HandleTypeDef hdma_usart3_rx;

static void HAL_UART3_DMAInit()
{
//    HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
//    HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);

    hdma_usart3_rx.Instance = DMA1_Stream1;
    hdma_usart3_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart3_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart3_rx.Init.Mode = DMA_NORMAL;
    hdma_usart3_rx.Init.Priority = DMA_PRIORITY_LOW;


    hdma_usart3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;

//    hdma_usart3_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    hdma_usart3_rx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_usart3_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_1QUARTERFULL;
    hdma_usart3_rx.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_usart3_rx.Init.PeriphBurst = DMA_PBURST_SINGLE;

    if (HAL_DMA_Init(&hdma_usart3_rx) != HAL_OK)
    {
      Error_Handler();
    }

//    __HAL_LINKDMA(huart3, hdmatrx)
}

static char buf[23] = "!!!!!!!!!!@@@@@@@@@@@\r\n";

void TEST_DMA()
{
	HAL_UART3_DMAInit();

	HAL_DMA_Start(&hdma_usart3_rx, (uint32_t)&huart3.Instance->RDR, (uint32_t)buf, 5);

	ATOMIC_SET_BIT(huart3.Instance->CR3, USART_CR3_DMAR);
	HAL_DMA_PollForTransfer(&hdma_usart3_rx, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
	ATOMIC_CLEAR_BIT(huart3.Instance->CR3, USART_CR3_DMAR);

	HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);

	HAL_UART_Transmit(&huart3, (const uint8_t*)buf, 23, HAL_MAX_DELAY);

	while (1);
}

#endif /* polling mode */









