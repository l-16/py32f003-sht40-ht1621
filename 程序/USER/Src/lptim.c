#include "lptim.h"

LPTIM_HandleTypeDef LPTIMConf = {0};
uint16_t Stop_Time = 60;

// 配置LSI时钟并启用LPTIM时钟
void APP_LPTIMClockConfig(void)
{
    RCC_OscInitTypeDef OSCINIT = {0};
    RCC_PeriphCLKInitTypeDef LPTIM_RCC = {0};

    // LSI时钟配置
    OSCINIT.OscillatorType = RCC_OSCILLATORTYPE_LSI;
    OSCINIT.LSIState = RCC_LSI_ON;
    if (HAL_RCC_OscConfig(&OSCINIT) != HAL_OK)
    {
        APP_ErrorHandler();
    }

    // LPTIM外设时钟配置
    LPTIM_RCC.PeriphClockSelection = RCC_PERIPHCLK_LPTIM;
    LPTIM_RCC.LptimClockSelection = RCC_LPTIMCLKSOURCE_LSI;
    if (HAL_RCCEx_PeriphCLKConfig(&LPTIM_RCC) != HAL_OK)
    {
        APP_ErrorHandler();
    }

    // 启用LPTIM时钟
    __HAL_RCC_LPTIM_CLK_ENABLE();
}

// LPTIM初始化
void MX_LPTIM_Init(void)
{
    // 先配置时钟
    APP_LPTIMClockConfig();

    // LPTIM配置
    LPTIMConf.Instance = LPTIM;
    LPTIMConf.Init.Prescaler = LPTIM_PRESCALER_DIV128;
    LPTIMConf.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;

    if (HAL_LPTIM_Init(&LPTIMConf) != HAL_OK)
    {
        APP_ErrorHandler();
    }

    // 配置LPTIM中断
    HAL_NVIC_SetPriority(LPTIM1_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(LPTIM1_IRQn);

#if (DBG == 1)
    UART1_SendString("LPTIM initialized\r\n");
#endif
}

// 微秒延时
static void APP_delay_us(uint32_t nus)
{
    __IO uint32_t Delay = 1 + nus * (SystemCoreClock / 24U) / 1000000U;
    do
    {
        __NOP();
    } while (Delay--);
}

// 启动LPTIM定时唤醒
// LSI约40kHz，分频128后约312.5Hz
// 10秒 = 3125计数，1分钟 = 18750计数
void LPTIM_Set_Wakeup_Time(void)
{
    // 使能自动重载匹配中断
    __HAL_LPTIM_ENABLE_IT(&LPTIMConf, LPTIM_IT_ARRM);

    // 使能LPTIM
    __HAL_LPTIM_ENABLE(&LPTIMConf);

    // 设置自动重载值
    __HAL_LPTIM_AUTORELOAD_SET(&LPTIMConf, Stop_Time * 256 - 64);   //64是矫正值

    // 延时120us等待LPTIM稳定
    APP_delay_us(120);

    // 启动单次计数模式
    __HAL_LPTIM_START_SINGLE(&LPTIMConf);

#if (DBG == 1)
    UART1_SendString("LPTIM started (2s)\r\n");
#endif
}
