#include "SHT40.h"

I2C_HandleTypeDef hi2c;

void MX_I2C_Init(void)
{

    hi2c.Instance = I2C1;
    hi2c.Init.ClockSpeed = 100000;         // 设置通信速率为 100kHz (标准模式)
    hi2c.Init.DutyCycle = I2C_DUTYCYCLE_2; // 时钟占空比
    hi2c.Init.OwnAddress1 = 0;             // 主机模式下自身地址设为0
    hi2c.Init.OwnAddress1 = SHT40_I2C_ADDRESS;
    hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c) != HAL_OK)
    {
        APP_ErrorHandler(); // 初始化失败处理
    }
}

// SHT40 初始化（发送软复位指令）
void SHT40_Init(void) {
    // 发送软复位指令，让传感器回到默认状态
    uint8_t cmd = SHT40_CMD_RESET;
    HAL_I2C_Master_Transmit(&hi2c, SHT40_I2C_ADDRESS, &cmd, 1, 100);
    HAL_Delay(10); // 等待复位完成
}


// 读取温湿度数据
void SHT40_ReadData(SHT40_DataTypeDef *data) {
    uint8_t cmd = SHT40_CMD_MEASURE;
    uint8_t rx_data[6] = {0};

    // 1. 发送高精度测量指令
    if (HAL_I2C_Master_Transmit(&hi2c, SHT40_I2C_ADDRESS, &cmd, 1, 100) != HAL_OK) {
        return; // 发送失败直接返回
    }

    // 2. 等待传感器测量完成（高精度模式官方建议等待约 9~10ms）
    HAL_Delay(20);

    // 3. 读取 6 字节数据
    if (HAL_I2C_Master_Receive(&hi2c, SHT40_I2C_ADDRESS, rx_data, 6, 100) != HAL_OK) {
        return; // 读取失败直接返回
    }

    // 4. 组合原始数据（大端模式：高8位 << 8 | 低8位）
    uint16_t rawTemp = (uint16_t)(rx_data[0] << 8) | rx_data[1];
    uint16_t rawHumi = (uint16_t)(rx_data[3] << 8) | rx_data[4];

    // 5. 根据官方公式转换为实际温湿度值
    data->temperature = -45.0f + 175.0f * ((float)rawTemp / 65535.0f);
    data->humidity = -6.0f + 125.0f * ((float)rawHumi / 65535.0f);
    
    // 可选：对湿度做上下限保护（防止出现 -0.01 或 100.01 的情况）
    if (data->humidity > 100.0f) data->humidity = 100.0f;
    if (data->humidity < 0.0f) data->humidity = 0.0f;
}
