#include "low_power.h"

// 全局唤醒标志
volatile uint8_t wakeup_source = WAKEUP_NONE;
static PWR_StopModeConfigTypeDef PwrStopModeConf = {0};

// 对外提供的统一进入低功耗接口
void Enter_Stop_Mode(void)
{
    // 1. 清除之前的唤醒标志，准备迎接新的唤醒
    __HAL_GPIO_EXTI_CLEAR_IT(KEY_Pin);
    wakeup_source = WAKEUP_NONE;

    // 4. 执行进入 Stop 前的硬件准备（关外设、设GPIO模拟）
    Prepare_Enter_Stop();
    
    // 2. 启动 LPTIM 定时唤醒
    LPTIM_Set_Wakeup_Time();

#if (DBG == 1)
    UART1_SendString("LPTIM started\r\n");
#endif

    // 3. 暂停 SysTick，防止它在低功耗期间捣乱
    HAL_SuspendTick();

    SystemClock_Switch_To_HSI();

    /* VCORE = 1.0V  when enter stop mode */
    PwrStopModeConf.LPVoltSelection = PWR_STOPMOD_LPR_VOLT_SCALE2;
    PwrStopModeConf.FlashDelay = PWR_WAKEUP_FLASH_DELAY_5US;
    PwrStopModeConf.WakeUpHsiEnableTime = PWR_WAKEUP_HSIEN_AFTER_MR;
    PwrStopModeConf.RegulatorSwitchDelay = PWR_WAKEUP_LPR_TO_MR_DELAY_2US;
    PwrStopModeConf.SramRetentionVolt = PWR_SRAM_RETENTION_VOLT_VOS;
    HAL_PWR_ConfigStopMode(&PwrStopModeConf);

    // 5. 真正进入 Stop 模式（程序会在这里“卡住”，直到被按键或 LPTIM 唤醒）
    // 使用低功耗稳压器，等待中断（WFI）唤醒
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    // --- 唤醒后，程序会立刻从这里继续往下执行 ---

    // 6. 恢复 SysTick
    HAL_ResumeTick();
}

// 进入 Stop 模式前的准备工作
static void Prepare_Enter_Stop(void)
{
#if (DBG == 1)
    // 1. 在关闭串口时钟前，先把提示发出去（并等待发送完成）
    UART1_SendString("Entering STOP mode...\r\n");
    HAL_Delay(10);
    // 如果你的 UART1_SendString 是非阻塞的，这里可以加一个简单的等待发送完成的逻辑
#endif

    // // 2. 关闭不需要的外设时钟（省电关键）
    __HAL_RCC_USART1_CLK_DISABLE();
    __HAL_RCC_ADC_CLK_DISABLE();
    __HAL_RCC_I2C_CLK_DISABLE();

    // // 3. 将空闲 GPIO 配置为模拟模式（防止引脚漏电）
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    // // // PA口：保留 PA1（按键）和 LPTIM 相关引脚，其他全设为模拟
    GPIO_InitStruct.Pin = GPIO_PIN_All & ~(KEY_Pin | GPIO_PIN_2); // 根据你的实际硬件调整
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // // PB口：全部设为模拟（根据你的实际硬件调整）
    GPIO_InitStruct.Pin = GPIO_PIN_All;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

// 从 Stop 模式唤醒后的恢复工作
void Recover_From_Stop(void)
{
    // 1. 重新配置系统时钟，切回 HSE (16MHz)
    SystemClock_Switch_To_HSE();

    // 2. 恢复调试串口（唤醒后才能正常打印）
#if (DBG == 1)
    HAL_UART_DeInit(&huart1);
    MX_USART1_UART_Init();
    UART1_SendString("Wakeup!\r\n");
#endif

    // 3. 重新初始化被我们关闭的外设
    HAL_I2C_DeInit(&hi2c);
    MX_I2C_Init();
    HAL_ADC_DeInit(&hadc);
    MX_ADC_Init();

    // 4. 重新初始化屏幕和传感器
    HT1621_WakeUp();
    SHT40_Init();
}

// 切换到HSE高精度时钟
void SystemClock_Switch_To_HSE(void)
{

    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        APP_ErrorHandler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        APP_ErrorHandler();
    }
}

static void SystemClock_Switch_To_HSI(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    // 设置时钟源为外部高速晶振
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    // AHB 和 APB 都不分频
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        APP_ErrorHandler();
    }

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_OFF;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        APP_ErrorHandler();
    }
}
