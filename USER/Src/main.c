/**
 ******************************************************************************
 * @file    main.c
 * @author  MCU Application Team
 * @brief   Main program body
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

#include "low_power.h"

/* Private user code ---------------------------------------------------------*/

#define LED_ON() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET)
#define LED_OFF() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET)

void SystemClock_Config(void);
void PA2_LED_Init(void);
void Collect_and_Display(void);

/* Private macro -------------------------------------------------------------*/

float temperature = 0.0;
float humidity = 0.0;
float voltage = 3.0;
char buffer[64];
uint8_t LED_State = 0; // 0: 关, 1: 开

SHT40_DataTypeDef sht40_data = {0};
uint8_t low_power_mode = 0; // 0:正常模式, 1:低功耗模式
uint32_t last_activity_time = 0;

/**
 * @brief  Main program.
 * @retval int
 */
int main(void)
{
    /* Reset of all peripherals, Initializes the Systick */
    HAL_Init();
    SystemClock_Config(); // 配置系统时钟

    MX_I2C_Init();   // 初始化 I2C1
    MX_ADC_Init();   // 初始化 ADC1
    MX_KEY_Init();   // 初始化按键 PA1
    MX_LPTIM_Init(); // 初始化 LPTIM1
    __HAL_RCC_PWR_CLK_ENABLE();

#if (DBG == 1)
    MX_USART1_UART_Init(); // 初始化 UART1
#endif

    HT1621_Init();
    SHT40_Init();
    PA2_LED_Init();

    HAL_Delay(100); // 等待 HT1621 稳定

    Collect_and_Display();
    Enter_Stop_Mode();

    while (1)
    {
        // 检查唤醒源
        if (wakeup_source == WAKEUP_KEY)
        {
            wakeup_source = WAKEUP_NONE;
            // 先禁用LPTIM
            __HAL_LPTIM_DISABLE(&LPTIMConf);
            // 按键唤醒
            if (Check_Key_Debounce())
            {
                Recover_From_Stop();
#if (DBG == 1)
                UART1_SendString("Key wakeup -");
#endif
                Collect_and_Display();
                LED_State = 1;
                LED_ON();
                Stop_Time = 5;
            }

            else
            {
#if (DBG == 1)
                UART1_SendString("Key bounce, ignore\r\n");
#endif
                __HAL_LPTIM_ENABLE(&LPTIMConf);
                HAL_SuspendTick();
                HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
                HAL_ResumeTick();
                continue;
            }
        }
        // 定时器唤醒
        else if (wakeup_source == WAKEUP_LPTIM)
        {
            if (LED_State == 1)
            {
                LED_State = 0;
                LED_OFF();
                Stop_Time = 60;
            }
            else
            {
                wakeup_source = WAKEUP_NONE;
                Recover_From_Stop();
#if (DBG == 1)
                UART1_SendString("Timer wakeup - ");
#endif
                Collect_and_Display();
            }
        }

        else
        {
            // 未知唤醒，直接采集
            Recover_From_Stop();
#if (DBG == 1)
            UART1_SendString("Unknown wakeup - ");
#endif
            Collect_and_Display();
        }
        Enter_Stop_Mode();
    }
}

void PA2_LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 1. 使能GPIOA的时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // 2. 配置PA3引脚
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  // 推挽输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;          // 无上下拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; // 低速
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 3. 默认先关灯（根据你的实际电路，高电平可能是关，也可能是开）
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // 时钟源配置：启用HSE、HSI和LSI
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;

    // 启用内部高速晶振
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_8MHz;

    // 启用外部高速晶振
    // RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    // RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;

    RCC_OscInitStruct.LSIState = RCC_LSI_ON;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        APP_ErrorHandler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    // 设置时钟源为外部高速晶振
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    // AHB 和 APB 都不分频
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;

    // 应用设置
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        APP_ErrorHandler();
    }
}

void Collect_and_Display(void)
{
    SHT40_ReadData(&sht40_data);
    voltage = Get_Bat_Voltage();
    // voltage += 0.1;
    LCD_Display(&sht40_data, voltage);
#if (DBG == 1)
    snprintf(buffer, sizeof(buffer), "Temp: %.2f C, Hum: %.2f %%, Voltage: %.2f V\r\n",
             sht40_data.temperature, sht40_data.humidity, voltage);
    UART1_SendString(buffer);
#endif
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
void APP_ErrorHandler(void)
{
    /* infinite loop */
    while (1)
    {
#if (DBG == 1)
        UART1_SendString("Error occurred!\r\n");
        HAL_Delay(1000);
#endif
    }
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       for example: printf("Wrong parameters value: file %s on line %d\r\n", file, line)  */
    /* infinite loop */
    while (1)
    {
    }
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
