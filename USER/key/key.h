#ifndef _KEY_H_
#define _KEY_H_
#include <stm32f10x.h>
#include "io_bit.h"


#define   KEY1  PCin(9)
#define   KEY2  PCin(8)


void Key_Init(void);
char Scanf_Key(void);
	
	
#endif
