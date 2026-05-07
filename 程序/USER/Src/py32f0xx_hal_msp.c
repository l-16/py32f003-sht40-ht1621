/**
 ******************************************************************************
 * @file    py32f0xx_hal_msp.c
 * @author  MCU Application Team
 * @brief   This file provides code for the MSP Initialization
 *          and de-Initialization codes.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 Puya Semiconductor Co.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by Puya under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "SHT40.h"
#include "ADC.h"
#include "KEY.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* External functions --------------------------------------------------------*/

/**
 * @brief Initialize global MSP
 */
void HAL_MspInit(void)
{
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *i2cHandle)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (i2cHandle->Instance == I2C1)
    {

        /* 1. 开启 GPIOB 和 I2C1 的外设时钟 */
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_I2C_CLK_ENABLE();

        /**I2C pin Configuration
        PB6     ------> I2C1_SCL
        PB7     ------> I2C1_SDA
        */

        /* 2. 配置 PB6 (SCL) 和 PB7 (SDA) */
        GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD; // 必须配置为复用开漏输出
        GPIO_InitStruct.Pull = GPIO_NOPULL;     // I2C总线通常需要外部上拉电阻，这里设为无上下拉
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF6_I2C;
        // PY32F003 的 PB6/PB7 上电默认就是 I2C 复用功能，一般不需要指定 Alternate 参数

        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        __HAL_RCC_I2C_FORCE_RESET();
        __HAL_RCC_I2C_RELEASE_RESET();
    }
}

// ADC 底层引脚初始化
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hadc->Instance == ADC1)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();

        GPIO_InitStruct.Pin = ADC_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(ADC_GPIO_Port, &GPIO_InitStruct);
    }
}

// 2. 串口底层硬件初始化
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        // 使能串口1和GPIOA的时钟
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        GPIO_InitTypeDef GPIO_InitStruct = {0};

        // 配置 PA7 为 USART1_TX
        GPIO_InitStruct.Pin = GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;       // 复用推挽输出
        GPIO_InitStruct.Pull = GPIO_PULLUP;           // 上拉
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // 高速
        GPIO_InitStruct.Alternate = GPIO_AF8_USART1;  // 映射到 AF1 (USART1)
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
