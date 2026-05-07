#ifndef _KEY_H_
#define _KEY_H_

#include "main.h"

#define KEY_Pin GPIO_PIN_1
#define KEY_GPIO_Port GPIOA

void MX_KEY_Init(void); // 按键初始化函数声明
uint8_t Check_Key_Debounce(void);

#endif // !_KEY_H_
