/*
 * uart_with_dma.h
 *
 *  Created on: Aug 18, 2022
 *      Author: JMJ
 */

#ifndef INC_UART_WITH_DMA_H_
#define INC_UART_WITH_DMA_H_


//#define UART1_ENABLE
//#define UART2_ENABLE
#define UART3_ENABLE

#define UART_BUFFER_SIZE 512


void UART_StartDMA();

void UART_putc(char c);
void UART_puts(const char* s);

void UART_flush();


#endif /* INC_UART_WITH_DMA_H_ */
