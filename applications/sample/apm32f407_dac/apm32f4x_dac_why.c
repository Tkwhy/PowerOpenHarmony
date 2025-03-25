#include <stdio.h>
#include "ohos_init.h"
#include "ohos_types.h"
#include "los_task.h"
#include "apm32f4xx_gpio.h"
#include "apm32f4xx_dac.h"
#include "apm32f4xx_adc.h"
#include "apm32f4xx_misc.h"
#include "apm32f4xx_rcm.h"
#include "los_sem.h"
static void DAC_Init();
static void ADC_Init(void); // ADC初始化
void ADC_IRQHandler(void);  // ADC中断处理函数
/*
    请在BUILD.gn中添加以下内容
    source_set("apm32f4x_dac_why.c")
    please connect PA4 to PA0
*/

/* 定义二值信号量的 ID 变量 */
UINT32 ADC_BinarySem_Handle;

void DacTask(void)
{
    UINT32 ret;
    UINT32 dacData = 0;
    DAC_Init();
    ADC_Init();
    LOS_HwiCreate(ADC_IRQn, 0, 0, ADC_IRQHandler, 0); // 配置中断
    while (1)
    {
        ADC_SoftwareStartConv(ADC1);
        ret = LOS_SemPend(ADC_BinarySem_Handle, LOS_WAIT_FOREVER); // 等待二值信号量
        if (ret == LOS_OK)
        {
            DAC_ConfigChannel1Data(DAC_ALIGN_12BIT_R, dacData);
            if (dacData >= 0x0FFF)
            {
                dacData = 0;
            }
            dacData += 0x7F;
        }
        else
        {
            printf("LOS_SemPend failed\n");
        }
        LOS_TaskDelay(1000); // 延时1s
    }
}
VOID DacTaskSample(VOID)
{
    UINT32 ret;
    UINT32 dactaskID1;
    TSK_INIT_PARAM_S stTask = {0};
    printf("DacTaskSample: DacTask create start...\n");
    ret = LOS_BinarySemCreate(0, &ADC_BinarySem_Handle); // 创建二值信号量
    if (ret != LOS_OK)
    {
        printf("Create BinarySem failed\n");
    }
    stTask.pfnTaskEntry = (TSK_ENTRY_FUNC)DacTask;
    stTask.uwStackSize = 4096;
    stTask.pcName = "DacTask";
    stTask.usTaskPrio = 6;
    ret = LOS_TaskCreate(&dactaskID1, &stTask);
    if (ret != LOS_OK)
    {
        printf("Task1 create failed\n");
    }
}
void ADC_IRQHandler(void)
{
    if (ADC_ReadStatusFlag(ADC1, ADC_FLAG_EOC))
    {
        printf("adcdata: 0x%03x\r\n", ADC_ReadConversionValue(ADC1));
        LOS_SemPost(ADC_BinarySem_Handle); // 释放二值信号量
        ADC_ClearStatusFlag(ADC1, ADC_FLAG_EOC);
    }
}
static void DAC_Init()
{
    GPIO_Config_T gpioConfig;
    DAC_Config_T dacConfig;

    /* Enable GPIOA clock */
    RCM_EnableAHB1PeriphClock(RCM_AHB1_PERIPH_GPIOA);

    /* DAC out PA4 pin configuration */
    GPIO_ConfigStructInit(&gpioConfig);
    gpioConfig.mode = GPIO_MODE_AN;
    gpioConfig.pupd = GPIO_PUPD_NOPULL;
    gpioConfig.pin = GPIO_PIN_4;
    GPIO_Config(GPIOA, &gpioConfig);

    /* Enable DAC clock */
    RCM_EnableAPB1PeriphClock(RCM_APB1_PERIPH_DAC);

    /* DAC channel 1 configuration */
    DAC_ConfigStructInit(&dacConfig);
    dacConfig.trigger = DAC_TRIGGER_NONE;
    dacConfig.waveGeneration = DAC_WAVE_GENERATION_NONE;
    dacConfig.maskAmplitudeSelect = DAC_LFSR_MASK_BIT11_1;
    dacConfig.outputBuffer = DAC_OUTPUT_BUFFER_ENABLE;
    DAC_Config(DAC_CHANNEL_1, &dacConfig);

    /* Enable DAC channel 1 */
    DAC_Enable(DAC_CHANNEL_1);
}
static void ADC_Init(void)
{
    GPIO_Config_T gpioConfig;
    ADC_Config_T adcConfig;
    ADC_CommonConfig_T adcCommonConfig;

    /* Enable GPIOA clock */
    RCM_EnableAHB1PeriphClock(RCM_AHB1_PERIPH_GPIOA);

    /* ADC channel 0 configuration */
    GPIO_ConfigStructInit(&gpioConfig);
    gpioConfig.mode = GPIO_MODE_AN;
    gpioConfig.pupd = GPIO_PUPD_NOPULL;
    gpioConfig.pin = GPIO_PIN_0;
    GPIO_Config(GPIOA, &gpioConfig);

    /* Enable ADC clock */
    RCM_EnableAPB2PeriphClock(RCM_APB2_PERIPH_ADC1);

    /* ADC configuration */
    ADC_Reset();
    ADC_ConfigStructInit(&adcConfig);
    adcConfig.resolution = ADC_RESOLUTION_12BIT;
    adcConfig.continuousConvMode = DISABLE;
    adcConfig.dataAlign = ADC_DATA_ALIGN_RIGHT;
    adcConfig.extTrigEdge = ADC_EXT_TRIG_EDGE_NONE;
    adcConfig.scanConvMode = DISABLE;
    ADC_Config(ADC1, &adcConfig);

    /*Set ADC Clock Prescaler. ADC_Clock = APB2_Clock/4, 84/4=21MHz*/
    adcCommonConfig.prescaler = ADC_PRESCALER_DIV4;
    ADC_CommonConfig(&adcCommonConfig);

    /* ADC channel 10 Convert configuration */
    ADC_ConfigRegularChannel(ADC1, ADC_CHANNEL_0, 1, ADC_SAMPLETIME_112CYCLES);

    /* Enable complete conversion interupt */
    ADC_EnableInterrupt(ADC1, ADC_INT_EOC);

    /* NVIC configuration */
    NVIC_EnableIRQRequest(ADC_IRQn, 1, 1);

    /* Enable ADC */
    ADC_Enable(ADC1);

    /* ADC start conversion */
    // ADC_SoftwareStartConv(ADC1);
}
SYS_RUN(DacTaskSample);