#include "ohos_init.h"
#include "ohos_types.h"
#include "los_task.h"
#include "los_config.h"
#include "Board_APM32F407_EVAL.h"
#include "apm32f4xx_dma.h"
#include "apm32f4xx_adc.h"

// 请在BUILD.gn中添加以下内容
// source_set("$(CHIP_LIB)/apm32f4xx_adc")

#define ADC_CH_SIZE 3
#define ADC_DR_ADDR ((uint32_t)ADC1_BASE + 0x4C)
/*
 * ADC_Mode: 0: Single Channel ADC
 *           1: Multi Channel ADC
 */
#define ADC_Mode 0

uint16_t adcData[ADC_CH_SIZE];
// 单通道ADC初始化
void DMA_Singel_Init(uint32_t *Buf);
void ADC_Singel_Init(void);
// 多通道ADC初始化
void DMA_Init(void);
void ADC_Init(void);
// 单通道ADC DMA 任务
#if !ADC_Mode
static void ADC_DMA_Task(void)
{
    uint32_t DMA_ConvertedValue = 0;
    float ADC_ConvertedValue = 0;
    ADC_Singel_Init();
    DMA_Singel_Init(&DMA_ConvertedValue);

    while (1)
    {
        // printf("ADC Value: %d\r\n", ADC_Value);
        // printf("test\r\n");
        APM_EVAL_LEDToggle(LED2);
        if (DMA_ReadStatusFlag(DMA2_Stream0, DMA_FLAG_TCIFLG0))
        {
            ADC_ConvertedValue = ((float)DMA_ConvertedValue / 4095) * 3.3f;
            // printf("\r\n");
            printf("ADC REGDATA = 0x%04X \r\n", (uint16_t)DMA_ConvertedValue);
            printf("Voltage    = %3f V \r\n", ADC_ConvertedValue);
            // printf("Voltage    = %3f V \r\n", ADC_ConvertedValue);

            LOS_TaskDelay(1000);
            DMA_ClearStatusFlag(DMA2_Stream0, DMA_FLAG_TCIFLG0);
        }
    }
}
#endif // !ADC_Mode
// 多通道ADC任务
#if ADC_Mode
static void ADC_MultiChannel_Task(void)
{
    uint8_t index;
    ADC_Init();
    // DMA_Init();
    while (1)
    {
        for (index = 0; index < ADC_CH_SIZE; index++)
        {
            printf("ADC CH[%d] data =0x%03x \r\n", index, adcData[index]);
        }
        LOS_TaskDelay(1000);
    }
}
#endif // ADC_Mode
// 创建单通道ADC任务
#if !ADC_Mode
void CreateAdcDmATask(VOID)
{
    uint32 ret;
    uint32 taskID2;
    TSK_INIT_PARAM_S stTask = {0};
    printf("CreateAdcTask: ADC_DMA_Task create start...\n");
    stTask.pfnTaskEntry = (TSK_ENTRY_FUNC)ADC_DMA_Task;
    stTask.uwStackSize = 4096;
    stTask.pcName = "ADC_DMA_Task";
    stTask.usTaskPrio = 6;
    ret = LOS_TaskCreate(&taskID2, &stTask);
    if (ret != LOS_OK)
    {
        printf("ADC_DMA_Task create failed\n");
    }
}
#endif // !ADC_Mode
// 创建多通道任务
#if ADC_Mode
void CreateAdcMultiChannelTask(VOID)
{
    uint32 ret;
    uint32 taskID1;
    TSK_INIT_PARAM_S stTask = {0};
    printf("CreateAdcTask: ADC_MultiChannel_Task create start...\n");
    stTask.pfnTaskEntry = (TSK_ENTRY_FUNC)ADC_MultiChannel_Task;
    stTask.uwStackSize = 4096;
    stTask.pcName = "ADC_MultiChannel_Task";
    stTask.usTaskPrio = 6;
    ret = LOS_TaskCreate(&taskID1, &stTask);
    if (ret != LOS_OK)
    {
        printf("ADC_MultiChannel_Task create failed\n");
    }
}
#endif // ADC_Mode
/*!
 * @brief     ADC Init
 *
 * @param     None
 *
 * @retval    None
 */
void ADC_Singel_Init(void)
{
    GPIO_Config_T gpioConfig;
    ADC_Config_T adcConfig;
    ADC_CommonConfig_T adcCommonConfig;

    /* RCM Enable*/
    RCM_EnableAHB1PeriphClock(RCM_AHB1_PERIPH_GPIOA);
    /* GPIO Configuration */
    GPIO_ConfigStructInit(&gpioConfig);
    gpioConfig.pin = GPIO_PIN_0;
    gpioConfig.mode = GPIO_MODE_AN;
    gpioConfig.pupd = GPIO_PUPD_NOPULL;
    GPIO_Config(GPIOA, &gpioConfig);

    RCM_EnableAPB2PeriphClock(RCM_APB2_PERIPH_ADC1);

    /* ADC Configuration */
    ADC_Reset();
    ADC_ConfigStructInit(&adcConfig);
    /* Set resolution*/
    adcConfig.resolution = ADC_RESOLUTION_12BIT;
    /* Set dataAlign*/
    adcConfig.dataAlign = ADC_DATA_ALIGN_RIGHT;
    /* Set scanDir*/
    adcConfig.scanConvMode = DISABLE;
    /* Set convMode continous*/
    adcConfig.continuousConvMode = ENABLE;
    /* Set extTrigEdge*/
    adcConfig.extTrigEdge = ADC_EXT_TRIG_EDGE_NONE;
    /* Set nbrOfConversion*/

    /*Set ADC Clock Prescaler. ADC_Clock = APB2_Clock/4, 84/4=21MHz*/
    adcCommonConfig.prescaler = ADC_PRESCALER_DIV4;
    ADC_CommonConfig(&adcCommonConfig);

    ADC_Config(ADC1, &adcConfig);
    ADC_ConfigRegularChannel(ADC1, ADC_CHANNEL_0, 1, ADC_SAMPLETIME_112CYCLES);

    ADC_EnableDMA(ADC1);
    ADC_EnableDMARequest(ADC1);

    /* Enable ADC*/
    ADC_Enable(ADC1);
    ADC_SoftwareStartConv(ADC1);
}
/*!
 * @brief     DMA Init
 *
 * @param     None
 *
 * @retval    None
 */
void DMA_Singel_Init(uint32_t *Buf)
{
    /* DMA Configure */
    DMA_Config_T dmaConfig;

    /* Enable DMA clock */
    RCM_EnableAHB1PeriphClock(RCM_AHB1_PERIPH_DMA2);

    /* size of buffer*/
    dmaConfig.bufferSize = 1;
    /* set memory Data Size*/
    dmaConfig.memoryDataSize = DMA_MEMORY_DATA_SIZE_HALFWORD;
    /* Set peripheral Data Size*/
    dmaConfig.peripheralDataSize = DMA_PERIPHERAL_DATA_SIZE_HALFWORD;
    /* Enable Memory Address increase*/
    dmaConfig.memoryInc = DMA_MEMORY_INC_DISABLE;
    /* Disable Peripheral Address increase*/
    dmaConfig.peripheralInc = DMA_PERIPHERAL_INC_DISABLE;
    /* Reset Circular Mode*/
    dmaConfig.loopMode = DMA_MODE_CIRCULAR;
    /* set priority*/
    dmaConfig.priority = DMA_PRIORITY_HIGH;
    /* read from peripheral*/
    dmaConfig.dir = DMA_DIR_PERIPHERALTOMEMORY;
    /* Set memory Address*/
    dmaConfig.memoryBaseAddr = (uint32_t)Buf;
    /* Set Peripheral Address*/
    dmaConfig.peripheralBaseAddr = (uint32_t)&ADC1->REGDATA;

    dmaConfig.channel = DMA_CHANNEL_0;
    dmaConfig.fifoMode = DMA_FIFOMODE_DISABLE;
    dmaConfig.fifoThreshold = DMA_FIFOTHRESHOLD_FULL;
    dmaConfig.peripheralBurst = DMA_PERIPHERALBURST_SINGLE;
    dmaConfig.memoryBurst = DMA_MEMORYBURST_SINGLE;

    DMA_Config(DMA2_Stream0, &dmaConfig);
    /* Clear DMA TF flag*/
    DMA_ClearIntFlag(DMA2_Stream0, DMA_INT_TCIFLG0);
    /* Enable DMA Interrupt*/
    DMA_EnableInterrupt(DMA2_Stream0, DMA_INT_TCIFLG);
    DMA_Enable(DMA2_Stream0);
}

void DMA_Init(void)
{
    DMA_Config_T dmaConfig;

    RCM_EnableAHB1PeriphClock(RCM_AHB1_PERIPH_DMA2);

    dmaConfig.peripheralBaseAddr = ADC_DR_ADDR;
    dmaConfig.memoryBaseAddr = (uint32_t)&adcData;
    dmaConfig.dir = DMA_DIR_PERIPHERALTOMEMORY;
    dmaConfig.bufferSize = ADC_CH_SIZE;
    dmaConfig.peripheralInc = DMA_PERIPHERAL_INC_DISABLE;
    dmaConfig.memoryInc = DMA_MEMORY_INC_ENABLE;
    dmaConfig.peripheralDataSize = DMA_PERIPHERAL_DATA_SIZE_HALFWORD;
    dmaConfig.memoryDataSize = DMA_MEMORY_DATA_SIZE_HALFWORD;
    dmaConfig.loopMode = DMA_MODE_CIRCULAR;

    dmaConfig.priority = DMA_PRIORITY_HIGH;
    dmaConfig.fifoMode = DMA_FIFOMODE_DISABLE;
    dmaConfig.fifoThreshold = DMA_FIFOTHRESHOLD_HALFFULL;
    dmaConfig.memoryBurst = DMA_MEMORYBURST_SINGLE;
    dmaConfig.peripheralBurst = DMA_PERIPHERALBURST_SINGLE;
    dmaConfig.channel = DMA_CHANNEL_0;
    DMA_Config(DMA2_Stream0, &dmaConfig);

    DMA_Enable(DMA2_Stream0);
}

/*!
 * @brief     ADC Init
 *
 * @param     None
 *
 * @retval    None
 */
void ADC_Init(void)
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
    gpioConfig.pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
    GPIO_Config(GPIOA, &gpioConfig);

    /* Enable ADC clock */
    RCM_EnableAPB2PeriphClock(RCM_APB2_PERIPH_ADC1);

    /* ADC configuration */
    ADC_Reset();

    adcCommonConfig.mode = ADC_MODE_INDEPENDENT;
    adcCommonConfig.prescaler = ADC_PRESCALER_DIV4;
    adcCommonConfig.accessMode = ADC_ACCESS_MODE_DISABLED;
    adcCommonConfig.twoSampling = ADC_TWO_SAMPLING_20CYCLES;
    ADC_CommonConfig(&adcCommonConfig);

    ADC_ConfigStructInit(&adcConfig);
    adcConfig.resolution = ADC_RESOLUTION_12BIT;
    adcConfig.scanConvMode = ENABLE;
    adcConfig.continuousConvMode = ENABLE;
    adcConfig.dataAlign = ADC_DATA_ALIGN_RIGHT;
    adcConfig.extTrigEdge = ADC_EXT_TRIG_EDGE_NONE;
    adcConfig.extTrigConv = ADC_EXT_TRIG_CONV_TMR1_CC1;
    adcConfig.nbrOfChannel = ADC_CH_SIZE;
    ADC_Config(ADC1, &adcConfig);

    /* ADC channel Convert configuration */
    ADC_ConfigRegularChannel(ADC1, ADC_CHANNEL_0, 1, ADC_SAMPLETIME_480CYCLES);
    ADC_ConfigRegularChannel(ADC1, ADC_CHANNEL_1, 2, ADC_SAMPLETIME_480CYCLES);
    ADC_ConfigRegularChannel(ADC1, ADC_CHANNEL_2, 3, ADC_SAMPLETIME_480CYCLES);

    /* Config DMA*/
    DMA_Init();

    /* Enable ADC DMA Request*/
    ADC_EnableDMARequest(ADC1);

    /* Enable ADC DMA*/
    ADC_EnableDMA(ADC1);

    /* Enable ADC */
    ADC_Enable(ADC1);

    /* ADC start conversion */
    ADC_SoftwareStartConv(ADC1);
}
#if ADC_Mode
SYS_RUN(CreateAdcMultiChannelTask); // 多通道ADC任务
#endif
#if !ADC_Mode
SYS_RUN(CreateAdcDmATask); // 单通道ADC任务
#endif