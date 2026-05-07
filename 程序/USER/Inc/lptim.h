#ifndef __LPTIM_H
#define __LPTIM_H

#include "main.h"
#include "py32f0xx_hal_lptim.h"
#include "low_power.h"

extern LPTIM_HandleTypeDef LPTIMConf;
extern uint16_t Stop_Time;

void MX_LPTIM_Init(void);
void LPTIM_Set_Wakeup_Time(void);

#endif /* __LPTIM_H */
