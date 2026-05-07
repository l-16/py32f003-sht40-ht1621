#ifndef _LCD_H_
#define _LCD_H_

#include "HT1621.h"
#include "SHT40.h"

extern const uint8_t BCD_decode_tab1[10];
extern const uint8_t BCD_decode_tab2[10];
extern uint8_t GRAM[8];

static void Get_GRAM(SHT40_DataTypeDef *data,float voltage);
void LCD_Display(SHT40_DataTypeDef *data,float voltage);
void LCD_Clear(void);

#endif // !_LCD_H_
