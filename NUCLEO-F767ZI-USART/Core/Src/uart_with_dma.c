/*
 * uart_with_dma.c
 *
 *  Created on: Aug 18, 2022
 *      Author: JMJ
 */

#include "uart_with_dma.h"

#include <memory.h>
#include "usart.h"


#define min(a, b)			\
({							\
	typeof(a) _a = (a);		\
	typeof(b) _b = (b);		\
	(_a < _b ? _a : _b);	\
})

