#include "ADC.h"

ADC_HandleTypeDef hadc;


// ADC 初始化配置
void MX_ADC_Init(void) {
    // 官方例程的精华1：ADC复位与校准，能有效减小芯片个体差异带来的误差
    __HAL_RCC_ADC_FORCE_RESET();
    __HAL_RCC_ADC_RELEASE_RESET();
    __HAL_RCC_ADC_CLK_ENABLE();

    hadc.Instance = ADC1;
    hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc.Init.Resolution = ADC_RESOLUTION_12B;
    hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
    hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc.Init.LowPowerAutoWait = DISABLE; // 这里改为DISABLE，配合单次读取更稳定
    hadc.Init.ContinuousConvMode = DISABLE;
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc.Init.DMAContinuousRequests = DISABLE;
    hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    
    // 官方例程的精华2：设置最长的采样时间 (239.5个周期)
    // 你的分压电阻较大(300k+100k)，长采样时间能让ADC内部电容充放电更充分，读数更准
    hadc.Init.SamplingTimeCommon = ADC_SAMPLETIME_239CYCLES_5; 

    if (HAL_ADC_Init(&hadc) != HAL_OK) {
        APP_ErrorHandler();
    }

    // 官方例程的精华3：启动ADC校准
    if (HAL_ADCEx_Calibration_Start(&hadc) != HAL_OK) {
        APP_ErrorHandler();
    }
}


// 读取并计算锂电池实际电压
float Get_Bat_Voltage(void) {
    uint32_t adcValue = 0;
    float vBat = 0.0f;

    // 配置要读取的通道（PA0 对应 CHANNEL_0）
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
    // 注意：如果在 MX_ADC_Init 里用了 SamplingTimeCommon，这里可以不用重复设置 sConfig.SamplingTime
    if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK) {
        return 0.0f;
    }

    // 启动 ADC 转换
    if (HAL_ADC_Start(&hadc) != HAL_OK) {
        return 0.0f;
    }

    // 等待转换完成
    if (HAL_ADC_PollForConversion(&hadc, 100) == HAL_OK) { // 超时时间加长到100ms，因为采样时间变长了
        adcValue = HAL_ADC_GetValue(&hadc);
        // 还原真实电池电压：(ADC值 / 4095) * 3.3V * 分压系数4
        vBat = ((float)adcValue / 4095.0f) * 3.3f * VOLTAGE_DIVIDER_RATIO;
    }

    HAL_ADC_Stop(&hadc);
    return vBat;
}
