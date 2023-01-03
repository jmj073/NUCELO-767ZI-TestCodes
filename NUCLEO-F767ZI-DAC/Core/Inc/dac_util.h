/*
 * dac_util.h
 *
 *  Created on: 2023. 1. 1.
 *      Author: user
 */

#ifndef INC_DAC_UTIL_H_
#define INC_DAC_UTIL_H_

#include "main.h"

static inline
void DAC_WAKEUP() {
	volatile uint32_t counter;
	// DAC channel wakeup time
	counter = (10 * (SystemCoreClock / (uint32_t)1e6)); // 10us
	while (counter) counter--;
}

#endif /* INC_DAC_UTIL_H_ */
