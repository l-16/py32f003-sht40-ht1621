#include "KEY.h"
static GPIO_InitTypeDef GPIO_InitStruct = {0};
// 按键 GPIO 与外部中断初始化
void MX_KEY_Init(void)
{
    // 1. 开启 GPIOA 的时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // 2. 配置 PA1 引脚
    GPIO_InitStruct.Pin = KEY_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING; // 下降沿触发唤醒
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(KEY_GPIO_Port, &GPIO_InitStruct);

    // 3. 使能 EXTI1 的中断通道并设置优先级
    // PA1 对应的是 EXTI1 中断线
    HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0); // 抢占优先级2，子优先级0（可根据你的项目需求调整）
    HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
}

static void MX_KEY_Read_Init(void)
{
    __HAL_RCC_GPIOA_CLK_DISABLE();

    GPIO_InitStruct.Pin = KEY_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT; 
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(KEY_GPIO_Port, &GPIO_InitStruct);

    __HAL_RCC_GPIOA_CLK_ENABLE();
}

// 检查按键消抖
uint8_t Check_Key_Debounce(void)
{
    MX_KEY_Read_Init();
    uint8_t valid = 0;
    uint8_t stable_count = 0;

    // 20ms内采样10次
    for (int i = 0; i < 10; i++)
    {
        if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET)
        {
            stable_count++;
        }
        HAL_Delay(2); // 2ms间隔
    }

    // 10次采样中8次为低电平认为有效
    if (stable_count >= 8)
    {
        valid = 1;
    }
    GPIO_InitStruct.Pin = KEY_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING; // 下降沿触发唤醒
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(KEY_GPIO_Port, &GPIO_InitStruct);

    return valid;
}

// HAL 库标准的外部中断回调函数
// 注意：这个函数在py32f0xx_it.c中已经实现
// 这里保留空实现或删除，避免重复定义
