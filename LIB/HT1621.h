#ifndef _HT1621_H_
#define _HT1621_H_

#include "py32f0xx_hal.h"

// ================= 硬件引脚抽象层 (根据你的板子配置) =================
#define HT1621_RD_PORT      GPIOA
#define HT1621_RD_PIN       GPIO_PIN_6

#define HT1621_WR_PORT      GPIOA
#define HT1621_WR_PIN       GPIO_PIN_5

#define HT1621_CS_PORT      GPIOA
#define HT1621_CS_PIN       GPIO_PIN_4

#define HT1621_DATA_PORT    GPIOA
#define HT1621_DATA_PIN     GPIO_PIN_3

// ================= HT1621 基础命令定义 =================
#define HT1621_CMD_SYSDIS   0x00    // 关振系统荡器和LCD偏压发生器
#define HT1621_CMD_SYSEN    0x01    // 打开系统振荡器
#define HT1621_CMD_LCDOFF   0x02    // 关LCD偏压
#define HT1621_CMD_LCDON    0x03    // 打开LCD偏压
#define HT1621_CMD_RC256    0x18    // 系统时钟源，片内RC振荡器
#define HT1621_CMD_BIAS     (0x28|0x01)    // 1/3duty 4com (根据屏幕规格调整)//选中COM为4个公共口   0x01  选择了1/3偏压
#define HT1621_CMD_WDTDIS   0x05   // 禁止看门狗
#define HT1621_CMD_WDTEN    0x07   // 使能看门狗

#define HT1621_CS_HIGH()    HAL_GPIO_WritePin(HT1621_CS_PORT, HT1621_CS_PIN, GPIO_PIN_SET)
#define HT1621_CS_LOW()     HAL_GPIO_WritePin(HT1621_CS_PORT, HT1621_CS_PIN, GPIO_PIN_RESET)
#define HT1621_WR_HIGH()    HAL_GPIO_WritePin(HT1621_WR_PORT, HT1621_WR_PIN, GPIO_PIN_SET)
#define HT1621_WR_LOW()     HAL_GPIO_WritePin(HT1621_WR_PORT, HT1621_WR_PIN, GPIO_PIN_RESET)
#define HT1621_RD_HIGH()    HAL_GPIO_WritePin(HT1621_RD_PORT, HT1621_RD_PIN, GPIO_PIN_SET)
#define HT1621_RD_LOW()     HAL_GPIO_WritePin(HT1621_RD_PORT, HT1621_RD_PIN, GPIO_PIN_RESET)

// ================= 对外 API 函数声明 =================
void HT1621_Init(void);                             // HT1621 初始化
void HT1621_WakeUp(void);                             // HT1621 从睡眠中唤醒（如果支持）

void HT1621_WriteCmd(uint8_t cmd);                  // 写命令
void HT1621_Write_4b(uint8_t addr, uint8_t data);   // 指定地址写4位数据
void HT1621_Write_8b(uint8_t addr, uint8_t data);   // 指定地址写8位数据
void HT1621_WriteAll(uint8_t *p, uint8_t len);      // 连续写数据（批量刷新）
void HT1621_Clear(void);                            // 清屏
void HT1621_Fill(void);                             // 全屏点亮
uint8_t HT1621_ReadData(uint8_t addr);

#endif // !_HT1621_H_
