#ifndef _SHT40_H_
#define _SHT40_H_


#include "main.h"
#include "py32f0xx_hal_i2c.h"
#include "py32f0xx_hal_rcc.h"


extern I2C_HandleTypeDef hi2c;

typedef struct {
    float temperature;
    float humidity;
} SHT40_DataTypeDef;

#define SHT40_I2C_ADDRESS 0x44 << 1 // SHT40 I2C 地址，左移1位以适应HAL库的地址格式
#define SHT40_CMD_RESET   0x94         // 软复位指令
#define SHT40_CMD_MEASURE 0xFD         // 高精度测量指令

#define SHT40_I2C_HANDLER hi2c // 使用的 I2C 句柄，根据你的实际配置修改


// SHT40 function declarations
void MX_I2C_Init(void);

void SHT40_Init(void);

void SHT40_ReadData(SHT40_DataTypeDef *data);

#endif // !1
