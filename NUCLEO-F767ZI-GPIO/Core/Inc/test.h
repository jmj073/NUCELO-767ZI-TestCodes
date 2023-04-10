/*
 * test.h
 *
 *  Created on: Aug 7, 2022
 *      Author: JMJ
 *
 */

#ifndef INC_TEST_H_
#define INC_TEST_H_

void TEST_userled123_button();
void TEST_BSRR();
//void TEST_GPIOBitBanding(); // 적어도 0+, 7에서는 지원 안함...
void TEST_OpenDrainOutput();

#define TestFunction() TEST_BSRR()

#endif /* INC_TEST_H_ */
