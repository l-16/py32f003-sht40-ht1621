#ifndef _LOW_POWER_H_
#define _LOW_POWER_H_

#include "main.h"
#include "py32f0xx_hal_pwr.h"
#include "KEY.h"
#include "LCD.h"

#include "SHT40.h"
#include "ADC.h"
#include "lptim.h"

#if (DBG == 1)
#include "UART.h"
#endif

// 唤醒源定义
#define WAKEUP_NONE 0
#define WAKEUP_KEY 1
#define WAKEUP_LPTIM 2

extern volatile uint8_t wakeup_source;

void Enter_Stop_Mode(void);

static void Prepare_Enter_Stop(void);
void Recover_From_Stop(void);
void SystemClock_Switch_To_HSE(void);
static void SystemClock_Switch_To_HSI(void);

#endif // !_LOW_POWER_H_
