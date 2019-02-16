#ifndef DELAY_H
#define DELAY_H

#include "stm32f10x.h"

void delay_us(uint32_t time);
void osDelay(uint32_t time);

#endif	/* DELAY_H */
