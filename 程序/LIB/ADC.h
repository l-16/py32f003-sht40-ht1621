#ifndef _ADC_H_
#define _ADC_H_

#include "main.h"
#include "py32f0xx_hal_adc.h"

#define ADC_Pin GPIO_PIN_0
#define ADC_GPIO_Port GPIOA
#define ADC_CHANNEL ADC_CHANNEL_0

#define VOLTAGE_DIVIDER_RATIO 4.0f

extern ADC_HandleTypeDef hadc;

void MX_ADC_Init(void);

float Get_Bat_Voltage(void);

#endif // !_ADC_H_
