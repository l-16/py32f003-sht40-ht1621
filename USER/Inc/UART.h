#ifndef _UART_H_
#define _UART_H_

#include "main.h"

extern UART_HandleTypeDef huart1;

void MX_USART1_UART_Init(void);
void UART1_SendString(char *str);

#endif // !_UART_H_
