#include "ht1621.h"
#include <string.h> // 用于 memset 清屏

// ================= 底层 IO 模拟时序 =================
// 发送指定数量的位 (低位在前，HT1621 标准时序)
static void HT1621_SendBits(uint8_t data, uint8_t cnt)
{
    for (uint8_t i = 0; i < cnt; i++)
    {
        HT1621_WR_LOW(); // WR 拉低

        if (data & 0x80)
        {
            HAL_GPIO_WritePin(HT1621_DATA_PORT, HT1621_DATA_PIN, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(HT1621_DATA_PORT, HT1621_DATA_PIN, GPIO_PIN_RESET);
        }

        data <<= 1;       // 准备下一位
        HT1621_WR_HIGH(); // WR 拉高，上升沿写入

    }
    HAL_GPIO_WritePin(HT1621_DATA_PORT, HT1621_DATA_PIN, GPIO_PIN_SET); // 数据线默认拉高
}

// ================= 基础通信函数 =================
// 发送命令 (命令标识为 100)
void HT1621_WriteCmd(uint8_t cmd)
{
    HT1621_CS_LOW();          // CS 拉低
    HT1621_SendBits(0x80, 4); // 写入命令标志 100
    HT1621_SendBits(cmd, 8);  // 写入 8 位命令数据
    HT1621_CS_HIGH();         // CS 拉高
}

// 向指定地址写入 4 位数据 (数据写入标识为 101)
void HT1621_Write_4b(uint8_t addr, uint8_t data)
{
    HT1621_CS_LOW();
    HT1621_SendBits(0xA0, 3);      // 写入数据标志 101
    HT1621_SendBits(addr << 2, 6); // 写入 6 位 RAM 地址 (需左移2位)
    HT1621_SendBits(data, 4);      // 写入 4 位数据
    HT1621_CS_HIGH();
}
void HT1621_Write_8b(uint8_t addr, uint8_t data)
{
    HT1621_CS_LOW();
    HT1621_SendBits(0xA0, 3);      // 写入数据标志 101
    HT1621_SendBits(addr << 2, 6); // 写入 6 位 RAM 地址 (需左移2位)
    HT1621_SendBits(data, 8);      // 写入 8 位数据
    HT1621_CS_HIGH();
}

// 从指定地址开始，连续写入多个字节数据 (极大提高刷屏效率)
void HT1621_WriteAll(uint8_t *p, uint8_t len)
{
    HT1621_CS_LOW();
    HT1621_SendBits(0xA0, 3); // 写入数据标志 101
    HT1621_SendBits(0, 6);    // 起始地址设为 0
    for (uint8_t i = 0; i < len; i++)
    {
        HT1621_SendBits(*p, 8); // 连续发送数据 (每次发8位，即两个4位的COM)
        p++;
    }
    HT1621_CS_HIGH();
}

// ================= 应用层功能函数 =================
// 初始化 HT1621
void HT1621_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // 初始化 CS, WR, DATA 引脚为推挽输出
    GPIO_InitStruct.Pin = HT1621_CS_PIN | HT1621_WR_PIN | HT1621_DATA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // GPIO_InitStruct.Pin = HT1621_RD_PIN;
    // GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    // GPIO_InitStruct.Pull = GPIO_PULLUP;
    // HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 默认拉高 CS 和 WR
    HT1621_RD_HIGH();
    HAL_GPIO_WritePin(HT1621_CS_PORT, HT1621_CS_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HT1621_WR_PORT, HT1621_WR_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HT1621_DATA_PORT, HT1621_DATA_PIN, GPIO_PIN_SET);

    HAL_Delay(100); // 等待电源稳定
    // 发送初始化指令序列

    HT1621_WriteCmd(HT1621_CMD_SYSDIS<<1); // 先关闭系统
    HT1621_WriteCmd(HT1621_CMD_WDTDIS<<1); // 关闭看门狗
    HT1621_WriteCmd(HT1621_CMD_SYSEN<<1);  // 开启系统振荡器

    HT1621_WriteCmd(HT1621_CMD_RC256<<1); // 使用内部 RC 振荡器
    
    HT1621_WriteCmd(HT1621_CMD_BIAS<<1);  // 设置偏压和 Duty

    HT1621_WriteCmd(HT1621_CMD_LCDON<<1); // 开启 LCD 偏压，点亮屏幕

}

void HT1621_WakeUp(void){
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // 初始化 CS, WR, DATA 引脚为推挽输出
    GPIO_InitStruct.Pin = HT1621_CS_PIN | HT1621_WR_PIN | HT1621_DATA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 默认拉高 CS 和 WR
    HT1621_RD_HIGH();
    HAL_GPIO_WritePin(HT1621_CS_PORT, HT1621_CS_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HT1621_WR_PORT, HT1621_WR_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HT1621_DATA_PORT, HT1621_DATA_PIN, GPIO_PIN_SET);
}

// 清屏 (向所有 RAM 写入 0)
void HT1621_Clear(void)
{
    uint8_t buf[16] = {0}; // HT1621 有 32 个地址，每个地址 4 位，相当于 16 个字节
    HT1621_WriteAll(buf, 16);
}

// 全屏点亮 (向所有 RAM 写入 0xFF)
void HT1621_Fill(void)
{
    uint8_t buf[16];
    memset(buf, 0xff, 16);
    HT1621_WriteAll(buf, 16);
}

// 读取指定地址的 4 位数据（HT1621 一个地址对应 4 个 COM 位）
uint8_t HT1621_ReadData(uint8_t addr)
{
    uint8_t i, data = 0;
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    HT1621_CS_LOW(); // 拉低片选，开始通信

    // 1. 发送读命令标识 "110"
    // 手动发 3 位，高位先发
    HT1621_SendBits(0xC0, 3); // 110xxxxx，后面 5 位不重要

    // 2. 发送 6 位地址 (A5-A0)，高位先发
    HT1621_SendBits(addr << 2, 6); // 地址左移 2 位，末尾补 00

    // 3. 【关键】将 DATA 引脚切换为浮空输入，准备接收数据
    GPIO_InitStruct.Pin = HT1621_DATA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(HT1621_DATA_PORT, &GPIO_InitStruct);

    // 4. 在 RD 的下降沿读取 4 位数据 (D0-D3)，低位先发
    for (i = 0; i < 8; i++)
    {
        HT1621_RD_LOW(); // RD 拉低（下降沿）
        for (volatile int d = 0; d < 10; d++)
            ;       // 延时等待数据稳定
        data >>= 1; // 数据右移，准备接收低位
        if (HAL_GPIO_ReadPin(HT1621_DATA_PORT, HT1621_DATA_PIN) == GPIO_PIN_SET)
        {
            data |= 0x08; // 如果读到高电平，给最高位置 1
        }

        HT1621_RD_HIGH(); // RD 拉高
        for (volatile int d = 0; d < 10; d++)
            ; // 延时等待数据稳定
    }

    // 5. 读取完毕，将 DATA 引脚切回推挽输出（恢复默认状态）
    GPIO_InitStruct.Pin = HT1621_DATA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(HT1621_DATA_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(HT1621_DATA_PORT, HT1621_DATA_PIN, GPIO_PIN_SET);

    HT1621_CS_HIGH(); // 拉高片选，结束通信
    return data;      // 返回读到的 4 位数据
}
