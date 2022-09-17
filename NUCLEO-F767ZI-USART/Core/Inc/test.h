/*
 * test.h
 *
 *  Created on: Aug 10, 2022
 *      Author: user
 */

#ifndef INC_TEST_H_
#define INC_TEST_H_

void TEST_UART_polling();
void TEST_UART_interrupt();
void TEST_UART_DMA();
void TEST_UART_DMA2();


#define TestFunction() TEST_UART_DMA2()

#endif /* INC_TEST_H_ */
