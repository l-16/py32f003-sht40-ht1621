#include "LCD.h"
//湿度部分
const uint8_t BCD_decode_tab1[10] =
    {
        0xAF, /*0*/
        0x06, /*1*/
        0x6D, /*2*/
        0x4F, /*3*/
        0xC6, /*4*/
        0xCB, /*5*/
        0xEB, /*6*/
        0x0E, /*7*/
        0xEF, /*8*/
        0xCF, /*9*/
}; // BCD码字映射
// 温度部分
const uint8_t BCD_decode_tab2[10] =
    {
        0x5F, /*0*/
        0x06, /*1*/
        0x6B, /*2*/
        0x2F, /*3*/
        0x36, /*4*/
        0x3D, /*5*/
        0x7D, /*6*/
        0x07, /*7*/
        0x7F, /*8*/
        0x3F, /*9*/
};
const uint8_t LCD_BATTERY_ICONS[4] = {
    0xc2, // 0格电量
    0xca, // 1格电量
    0xce, // 2格电量
    0xcf  // 3格电量（满格）
};

uint8_t GRAM[8] = {0}; // LCD 映射的 GRAM 数组，存储要显示的内容

void LCD_Display(SHT40_DataTypeDef *data,float voltage)
{
    Get_GRAM(data,voltage);
    HT1621_WriteAll((uint8_t *)GRAM, 16); // 将 GRAM 中的内容刷新到屏幕上
}

void LCD_Clear(void)
{
    // memset(GRAM, 0, sizeof(GRAM)); // 将 GRAM 中的内容清零
    HT1621_WriteAll((uint8_t *)GRAM, 16); // 刷新到屏幕上
}
void Get_GRAM(SHT40_DataTypeDef *data,float voltage)
{
    // 这里需要根据你的 LCD 映射关系，将温湿度数据转换成对应的段码，存入 GRAM 数组
    // 例如，如果你想在第一个数字位置显示温度的整数部分，可以这样做：
    int temp_int = (int)(data->temperature);                    // 获取温度的整数部分
    int temp_dec = (int)((data->temperature - temp_int) * 100); // 获取温度的小数部分（两位）

    int hum_int = (int)(data->humidity);                   // 获取湿度的整数部分
    int hum_dec = (int)((data->humidity - hum_int) * 100); // 获取湿度的小数部分（两位）
    // 根据你的 LCD 映射关系，将这些数值转换成对应的段码，存入 GRAM 数组
    // 这里假设 GRAM[0] 存储温度整数部分，GRAM[1] 存储温度小数部分，GRAM[2] 存储湿度整数部分，GRAM[3] 存储湿度小数部分
    if (data->temperature < 0)
    {
        temp_int = -temp_int;                             // 取绝对值
        GRAM[3] = 0xa0;                                   // 负号
        GRAM[4] = BCD_decode_tab2[temp_int / 10] | 0x80;  // 温度十位
        GRAM[5] = BCD_decode_tab2[temp_int % 10];         // 温度个位
        GRAM[6] = BCD_decode_tab2[-temp_dec / 10] | 0x80; // 温度小数十位（小数点位置）
    }
    else
    {
        GRAM[3] = BCD_decode_tab2[temp_int / 10] | 0x80; // 温度十位
        GRAM[4] = BCD_decode_tab2[temp_int % 10] | 0x80; // 温度个位
        GRAM[5] = BCD_decode_tab2[temp_dec / 10] | 0x80; // 温度小数十位（小数点位置）
        GRAM[6] = BCD_decode_tab2[temp_dec % 10];        // 温度小数个位
    }

    if (data->humidity >= 100.0f)
    {
        GRAM[0] = BCD_decode_tab1[0] | 0x10; // 湿度十位
        GRAM[1] = BCD_decode_tab1[0];        // 湿度个位
        GRAM[2] = BCD_decode_tab1[0] | 0x10; // 湿度小数十位
    }
    else
    {
        GRAM[0] = BCD_decode_tab1[hum_int / 10];        // 湿度十位
        GRAM[1] = BCD_decode_tab1[hum_int % 10];        // 湿度个位
        GRAM[2] = BCD_decode_tab1[hum_dec / 10] | 0x10; // 湿度小数十位
    }
        if (voltage >= 4.00) {
        GRAM[7] = LCD_BATTERY_ICONS[3]; // 满格电量图标
    } else if (voltage >= 3.70) {
        GRAM[7] = LCD_BATTERY_ICONS[2]; // 2格电量图标
    } else if (voltage >= 3.40) {
        GRAM[7] = LCD_BATTERY_ICONS[1]; // 1格电量图标  
    } else {
        GRAM[7] = LCD_BATTERY_ICONS[0]; // 0格电量图标
    }
}
