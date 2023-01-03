/*
 * [L]TEST_DAC_DMA.c
 *
 *  Created on: 2023. 1. 1.
 *      Author: JMJ
 */

#if 0 /* TIM6 */

#include "dac_util.h"

#define stream DMA1_Stream5

static uint16_t DMA_DATA[8];

static
void DAC_Msp_Init() {
	/* DMA */
	__HAL_RCC_DMA1_CLK_ENABLE();
	stream->CR |= (
		7UL << DMA_SxCR_CHSEL_Pos 	| // select channel 7
		DMA_SxCR_CIRC				| // circular mode
		0b01 << DMA_SxCR_DIR_Pos	| // memory-to-peripheral
		0b01 << DMA_SxCR_MSIZE_Pos	| // msize=half-word
		0b01 << DMA_SxCR_PSIZE_Pos	| // psize=half-word
		DMA_SxCR_MINC				| // mem increase
	0);

	stream->NDTR = sizeof(DMA_DATA) / 2; 	// num of data
	stream->PAR = (uint32_t)&DAC->DHR12R1; 	// periph addr
	stream->M0AR = (uint32_t)DMA_DATA; 		// mem addr


	/* GPIO */
	GPIO_InitTypeDef gpio_init = { 0 };

	__HAL_RCC_GPIOA_CLK_ENABLE();
	/* PA4 ------> DAC_OUT1 */
	gpio_init.Pin = GPIO_PIN_4;
	gpio_init.Mode = GPIO_MODE_ANALOG;
	gpio_init.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpio_init);
}

static
void TIM6_Init() { // APB1 tim clk: 96MHz
	__HAL_RCC_TIM6_CLK_ENABLE();

	TIM6->PSC = 48000 - 1;
	TIM6->ARR = 500 - 1;

	TIM6->CR1 |= TIM_CR1_ARPE;
	TIM6->CR2 |= 0b010 << TIM_CR2_MMS_Pos; // update event를 TRGO로 설정
}

static
void DAC_Init() {
	__HAL_RCC_DAC_CLK_ENABLE();
	DAC_Msp_Init();

	DAC->CR |= (
		0b000 << DAC_CR_TSEL1_Pos 	| // Timer 6 TRGO event
		DAC_CR_TEN1					| // trigger enable
		DAC_CR_DMAEN1				| // DMA mode enable
	0);
}

void TestFunction() {
	for (int i = 0; i < 8; ++i) {
		DMA_DATA[i] = 250 * i + 2250;
	}

	TIM6_Init();
	DAC_Init();

	stream->CR |= DMA_SxCR_EN; // stream enable
	DAC->CR |= DAC_CR_EN1;
	DAC_WAKEUP();
	TIM6->CR1 |= TIM_CR1_CEN;

	while (1);
}

#endif /* TIM6 */


#if 0 /* TIMER ONE PUSE MODE */

#include "dac_util.h"

#define stream DMA1_Stream5

static uint16_t DMA_DATA[1] = { 3000 };

static
void DAC_Msp_Init() {
	/* DMA */
	__HAL_RCC_DMA1_CLK_ENABLE();
	stream->CR |= (
		7UL << DMA_SxCR_CHSEL_Pos 	| // select channel 7
		DMA_SxCR_CIRC				| // circular mode
		0b01 << DMA_SxCR_DIR_Pos	| // memory-to-peripheral
		0b01 << DMA_SxCR_MSIZE_Pos	| // msize=half-word
		0b01 << DMA_SxCR_PSIZE_Pos	| // psize=half-word
		DMA_SxCR_MINC				| // mem increase
	0);

	stream->NDTR = sizeof(DMA_DATA) / 2; 	// num of data
	stream->PAR = (uint32_t)&DAC->DHR12R1; 	// periph addr
	stream->M0AR = (uint32_t)DMA_DATA; 		// mem addr


	/* GPIO */
	GPIO_InitTypeDef gpio_init = { 0 };

	__HAL_RCC_GPIOA_CLK_ENABLE();
	/* PA4 ------> DAC_OUT1 */
	gpio_init.Pin = GPIO_PIN_4;
	gpio_init.Mode = GPIO_MODE_ANALOG;
	gpio_init.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpio_init);
}

static
void TIM6_Init() { // APB1 tim clk: 96MHz
	__HAL_RCC_TIM6_CLK_ENABLE();

	TIM6->PSC = 48000 - 1;
	TIM6->ARR = 500 - 1;

	TIM3->CR1 |= (
		TIM_CR1_ARPE	| // arr preload enable
		TIM_CR1_OPM		| // one-pulse mode enable
	0);
	TIM6->CR2 |= 0b010 << TIM_CR2_MMS_Pos; // update event를 TRGO로 설정
}

static
void DAC_Init() {
	__HAL_RCC_DAC_CLK_ENABLE();
	DAC_Msp_Init();

	DAC->CR |= (
		0b000 << DAC_CR_TSEL1_Pos 	| // Timer 6 TRGO event
		DAC_CR_TEN1					| // trigger enable
		DAC_CR_DMAEN1				| // DMA mode enable
	0);
}

void TestFunction() {

	TIM6_Init();
	DAC_Init();

	stream->CR |= DMA_SxCR_EN; // stream enable
	DAC->CR |= DAC_CR_EN1;
	DAC_WAKEUP();

	DAC->DHR12R1 = 0;
	TIM6->CR1 |= TIM_CR1_CEN;

	while (1);
}

#endif /* TIMER ONE PUSE MODE */

#if 0 /* two channel */

#include "dac_util.h"

#define stream1 DMA1_Stream5
#define stream2 DMA1_Stream6

#define DATA_SIZE 8
static uint16_t DMA_DATA1[DATA_SIZE];
static uint16_t DMA_DATA2[DATA_SIZE];

static
void DAC_Msp_Init() {
	/* DMA */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA for channel1 */
	stream1->CR |= (
		7UL << DMA_SxCR_CHSEL_Pos 	| // select channel 7
		DMA_SxCR_CIRC				| // circular mode
		0b01 << DMA_SxCR_DIR_Pos	| // memory-to-peripheral
		0b01 << DMA_SxCR_MSIZE_Pos	| // msize=half-word
		0b01 << DMA_SxCR_PSIZE_Pos	| // psize=half-word
		DMA_SxCR_MINC				| // mem increase
	0);

	stream1->NDTR = DATA_SIZE; 				// num of data
	stream1->PAR = (uint32_t)&DAC->DHR12R1; // periph addr
	stream1->M0AR = (uint32_t)DMA_DATA1; 	// mem addr

	/* DMA for channel2 */
	stream2->CR |= (
		7UL << DMA_SxCR_CHSEL_Pos 	| // select channel 7
		DMA_SxCR_CIRC				| // circular mode
		0b01 << DMA_SxCR_DIR_Pos	| // memory-to-peripheral
		0b01 << DMA_SxCR_MSIZE_Pos	| // msize=half-word
		0b01 << DMA_SxCR_PSIZE_Pos	| // psize=half-word
		DMA_SxCR_MINC				| // mem increase
	0);

	stream2->NDTR = DATA_SIZE; 				// num of data
	stream2->PAR = (uint32_t)&DAC->DHR12R2; // periph addr
	stream2->M0AR = (uint32_t)DMA_DATA2; 	// mem addr

	/* GPIO */
	GPIO_InitTypeDef gpio_init = { 0 };

	__HAL_RCC_GPIOA_CLK_ENABLE();
	// PA4 ------> DAC_OUT1
	// PA5 ------> DAC_OUT2
	gpio_init.Pin = GPIO_PIN_4 | GPIO_PIN_5;
	gpio_init.Mode = GPIO_MODE_ANALOG;
	gpio_init.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpio_init);
}

static
void TIM6_Init() { // APB1 tim clk: 96MHz
	__HAL_RCC_TIM6_CLK_ENABLE();

	TIM6->PSC = 48000 - 1;
	TIM6->ARR = 500 - 1;

	TIM6->CR1 |= TIM_CR1_ARPE;
	TIM6->CR2 |= 0b010 << TIM_CR2_MMS_Pos; // update event를 TRGO로 설정
}

static
void DAC_Init() {
	__HAL_RCC_DAC_CLK_ENABLE();
	DAC_Msp_Init();

	DAC->CR |= (
		/* channel 1 */
		0b000 << DAC_CR_TSEL1_Pos 	| // Timer 6 TRGO event
		DAC_CR_TEN1					| // trigger enable
		DAC_CR_DMAEN1				| // DMA mode enable
		/* channel 2 */
		0b000 << DAC_CR_TSEL2_Pos 	| // Timer 6 TRGO event
		DAC_CR_TEN2					| // trigger enable
		DAC_CR_DMAEN2				| // DMA mode enable
	0);
}

void TestFunction() {
	for (int i = 0; i < DATA_SIZE; ++i) {
		DMA_DATA1[i] = 250 * i + 2250;
		DMA_DATA2[DATA_SIZE - i - 1] = 250 * i + 2250;
	}

	TIM6_Init();
	DAC_Init();

	stream1->CR |= DMA_SxCR_EN; // stream enable
	stream2->CR |= DMA_SxCR_EN; // stream enable
	DAC->CR |= DAC_CR_EN1;
	DAC->CR |= DAC_CR_EN2;
	DAC_WAKEUP();
	TIM6->CR1 |= TIM_CR1_CEN;

	while (1);
}

#endif /* two channel */

#if 0 /* DMA underrun */

#include "dac_util.h"

#define stream DMA1_Stream5

static uint16_t DMA_DATA[8];

static
void DAC_Msp_Init() {
	/* DMA */
	__HAL_RCC_DMA1_CLK_ENABLE();
	stream->CR |= (
		7UL << DMA_SxCR_CHSEL_Pos 	| // select channel 7
		DMA_SxCR_CIRC				| // circular mode
		0b01 << DMA_SxCR_DIR_Pos	| // memory-to-peripheral
		0b01 << DMA_SxCR_MSIZE_Pos	| // msize=half-word
		0b01 << DMA_SxCR_PSIZE_Pos	| // psize=half-word
		DMA_SxCR_MINC				| // mem increase
	0);

	stream->NDTR = sizeof(DMA_DATA) / 2; 	// num of data
	/*
	 * underrun을 위해서 주소를 일부러 channel 1의 DHR을 가리키지 않게 한다.
	 * 아래의 DHR12R2은 별 의미 없다.
	 */
	stream->PAR = (uint32_t)&DAC->DHR12R2; 	// periph addr
	stream->M0AR = (uint32_t)DMA_DATA; 		// mem addr


	/* GPIO */
	GPIO_InitTypeDef gpio_init = { 0 };

	__HAL_RCC_GPIOA_CLK_ENABLE();
	/* PA4 ------> DAC_OUT1 */
	gpio_init.Pin = GPIO_PIN_4;
	gpio_init.Mode = GPIO_MODE_ANALOG;
	gpio_init.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpio_init);
}

static
void TIM6_Init() { // APB1 tim clk: 96MHz
	__HAL_RCC_TIM6_CLK_ENABLE();

	TIM6->PSC = 48000 - 1;
	TIM6->ARR = 500 - 1;

	TIM6->CR1 |= TIM_CR1_ARPE;
	TIM6->CR2 |= 0b010 << TIM_CR2_MMS_Pos; // update event를 TRGO로 설정
}

static
void DAC_Init() {
	__HAL_RCC_DAC_CLK_ENABLE();
	DAC_Msp_Init();

	DAC->CR |= (
		0b000 << DAC_CR_TSEL1_Pos 	| // Timer 6 TRGO event
		DAC_CR_TEN1					| // trigger enable
		DAC_CR_DMAEN1				| // DMA mode enable
	0);
}

void TestFunction() {
	TIM6_Init();
	DAC_Init();

	stream->CR |= DMA_SxCR_EN;
	DAC->CR |= DAC_CR_EN1;
	DAC_WAKEUP();

	TIM6->EGR = TIM_EGR_UG;
	HAL_Delay(1);
	if (DAC->SR & DAC_SR_DMAUDR1) {
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
	}

	TIM6->EGR = TIM_EGR_UG;
	HAL_Delay(1);
	if (DAC->SR & DAC_SR_DMAUDR1) {
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
	}
}

#endif /* DMA underrun */


// TODO 이어서 하기
#if 0 /* dual mode */

#include "dac_util.h"

#define stream1 DMA1_Stream5
#define stream2 DMA1_Stream6

#define DATA_SIZE 8
static uint16_t DMA_DATA1[DATA_SIZE];
static uint16_t DMA_DATA2[DATA_SIZE];

static
void DAC_Msp_Init() {
	/* DMA */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA for channel1 */
	stream1->CR |= (
		7UL << DMA_SxCR_CHSEL_Pos 	| // select channel 7
		DMA_SxCR_CIRC				| // circular mode
		0b01 << DMA_SxCR_DIR_Pos	| // memory-to-peripheral
		0b01 << DMA_SxCR_MSIZE_Pos	| // msize=half-word
		0b01 << DMA_SxCR_PSIZE_Pos	| // psize=half-word
		DMA_SxCR_MINC				| // mem increase
	0);

	stream1->NDTR = DATA_SIZE; 				// num of data
	stream1->PAR = (uint32_t)&DAC->DHR12R1; // periph addr
	stream1->M0AR = (uint32_t)DMA_DATA1; 	// mem addr

	/* DMA for channel2 */
	stream2->CR |= (
		7UL << DMA_SxCR_CHSEL_Pos 	| // select channel 7
		DMA_SxCR_CIRC				| // circular mode
		0b01 << DMA_SxCR_DIR_Pos	| // memory-to-peripheral
		0b01 << DMA_SxCR_MSIZE_Pos	| // msize=half-word
		0b01 << DMA_SxCR_PSIZE_Pos	| // psize=half-word
		DMA_SxCR_MINC				| // mem increase
	0);

	stream2->NDTR = DATA_SIZE; 				// num of data
	stream2->PAR = (uint32_t)&DAC->DHR12R2; // periph addr
	stream2->M0AR = (uint32_t)DMA_DATA2; 	// mem addr

	/* GPIO */
	GPIO_InitTypeDef gpio_init = { 0 };

	__HAL_RCC_GPIOA_CLK_ENABLE();
	// PA4 ------> DAC_OUT1
	// PA5 ------> DAC_OUT2
	gpio_init.Pin = GPIO_PIN_4 | GPIO_PIN_5;
	gpio_init.Mode = GPIO_MODE_ANALOG;
	gpio_init.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpio_init);
}

static
void TIM6_Init() { // APB1 tim clk: 96MHz
	__HAL_RCC_TIM6_CLK_ENABLE();

	TIM6->PSC = 48000 - 1;
	TIM6->ARR = 500 - 1;

	TIM6->CR1 |= TIM_CR1_ARPE;
	TIM6->CR2 |= 0b010 << TIM_CR2_MMS_Pos; // update event를 TRGO로 설정
}

static
void DAC_Init() {
	__HAL_RCC_DAC_CLK_ENABLE();
	DAC_Msp_Init();

	DAC->CR |= (
		/* channel 1 */
		0b000 << DAC_CR_TSEL1_Pos 	| // Timer 6 TRGO event
		DAC_CR_TEN1					| // trigger enable
		DAC_CR_DMAEN1				| // DMA mode enable
		/* channel 2 */
		0b000 << DAC_CR_TSEL2_Pos 	| // Timer 6 TRGO event
		DAC_CR_TEN2					| // trigger enable
		DAC_CR_DMAEN2				| // DMA mode enable
	0);
}

void TestFunction() {
	for (int i = 0; i < DATA_SIZE; ++i) {
		DMA_DATA1[i] = 250 * i + 2250;
		DMA_DATA2[DATA_SIZE - i - 1] = 250 * i + 2250;
	}

	TIM6_Init();
	DAC_Init();

	stream1->CR |= DMA_SxCR_EN; // stream enable
	stream2->CR |= DMA_SxCR_EN; // stream enable
	DAC->CR |= DAC_CR_EN1;
	DAC->CR |= DAC_CR_EN2;
	DAC_WAKEUP();
	TIM6->CR1 |= TIM_CR1_CEN;

	while (1);
}

#endif /* dual mode */
