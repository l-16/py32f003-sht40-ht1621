#include "UART.h"
#include "string.h"

UART_HandleTypeDef huart1;

// 1. 串口1初始化函数（配置波特率、数据位等）
void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;                   // 波特率，根据你的需求修改
    huart1.Init.WordLength = UART_WORDLENGTH_8B;     // 8位数据位
    huart1.Init.StopBits = UART_STOPBITS_1;          // 1位停止位
    huart1.Init.Parity = UART_PARITY_NONE;           // 无校验位
    huart1.Init.Mode = UART_MODE_TX_RX;              // 收发模式
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;     // 无硬件流控
    huart1.Init.OverSampling = UART_OVERSAMPLING_16; // 16倍过采样

    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        APP_ErrorHandler();
    }
}


// 3. 简单的串口发送测试函数（可选）
void UART1_SendString(char *str) {
    HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 1000);
}

#if (defined (__CC_ARM)) || (defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
/**
  * @brief  writes a character to the usart
  * @param  ch
  *         *f
  * @retval the character
  */
int fputc(int ch, FILE *f)
{
  /* Send a byte to USART */
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000);

  return (ch);
}

/**
  * @brief  get a character from the usart
  * @param  *f
  * @retval a character
  */
int fgetc(FILE *f)
{
  int ch;
  HAL_UART_Receive(&huart1, (uint8_t *)&ch, 1, 1000);
  return (ch);
}

#elif defined(__ICCARM__)
/**
  * @brief  writes a character to the usart
  * @param  ch
  *         *f
  * @retval the character
  */
int putchar(int ch)
{
  /* Send a byte to USART */
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000);

  return (ch);
}
#elif  defined(__GNUC__)
/**
  * @brief  writes a character to the usart
  * @param  ch
  * @retval the character
  */
int __io_putchar(int ch)
{
  /* Send a byte to USART */
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000);

  return ch;
}

int _write(int file, char *ptr, int len)
{
  int DataIdx;
  for (DataIdx=0;DataIdx<len;DataIdx++)
  {
    __io_putchar(*ptr++);
  }
  return len;
}
#endif
