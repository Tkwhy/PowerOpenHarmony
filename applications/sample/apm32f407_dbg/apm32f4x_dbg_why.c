#include <stdio.h>
#include "ohos_init.h"
#include "ohos_types.h"
#include "los_task.h"
#include "apm32f4xx.h"
#include "apm32f4xx_gpio.h"
#include "apm32f4xx_rcm.h"
#include "Board_APM32F407_EVAL.h"

/*
    TraceMode: 0 选择异步TRACE模式
    TraceMode: 1 选择同步TRACE模式,数据长度为1
    TraceMode: 2 选择同步TRACE模式,数据长度为2
    TraceMode: 3 选择同步TRACE模式,数据长度为4
*/
#define TraceMode 1
/*
    测试说明：
    1. 本例程演示如何使用ITM输出调试信息
    2. ITM分别支持异步和同步模式，异步模式下，波特率固定为56250，同步模式下，波特率由HCL/2K决定
    3. 同步模式下，需要配置TRACE IO的引脚，TRACE IO的引脚由TRACECLK和TRACED0~3组成


*/
static void SystemClockConfig(void); // 系统时钟配置
// 初始化ITM
#if TraceMode == 0
static void TraceInit(void)
{
    DBGMCU->CFG_B.TRACE_IOEN = 1; // 使能TRACE功能
    DBGMCU->CFG_B.TRACE_MODE = 0; // 选择异步TRACE模式
    // 168MHz时钟下，设置TPIU波特率为56250
    TPI->ACPR = 336;                               // 时钟预分频
    TPI->SPPR = 2;                                 // 时钟分频
    CoreDebug->DEMCR = CoreDebug_DEMCR_TRCENA_Msk; // 使能调试模块
    ITM->LAR = 0xC5ACCE55;                         // 解锁ITM
    ITM->TCR = 0x00010005;                         // 启动ITM
    ITM->TER = 0x00000001;                         // 使能ITM的第一个通道 端口0
    ITM->TPR = 0x1;                                // 取消屏蔽激励端口7：0,使能ITM的第一个通道 端口0
    // 配置PB3为TRACE功能
    RCM_EnableAHB1PeriphClock(RCM_AHB1_PERIPH_GPIOB);
    GPIO_ConfigPinAF(GPIOB, GPIO_PIN_SOURCE_3, GPIO_AF_TRACE);
    GPIO_Config_T gpioConfig;
    gpioConfig.pin = GPIO_PIN_3;
    gpioConfig.mode = GPIO_MODE_AF;
    gpioConfig.speed = GPIO_SPEED_50MHz;
    gpioConfig.otype = GPIO_OTYPE_PP;
    gpioConfig.pupd = GPIO_PUPD_NOPULL;
    GPIO_Config(GPIOB, &gpioConfig);
    printf("ITM_Init success!\r\n");
}
#else // 同步模式
static void TraceInit(void)
{
    // 同步模式时，TRACE IO（包括TRACECK）由TRACECLK的上升沿驱动，因此TRACECLK的频率应该是TRACE IO的2倍
    // 重新配置时钟，将HCLK设置为84Mhz TRACE IO = 42/2 = 21Mhz
    SystemClockConfig();                  // 配置HCLK = 42Mhz
    DBGMCU->CFG_B.TRACE_IOEN = 1;         // 使能TRACE功能
    DBGMCU->CFG_B.TRACE_MODE = TraceMode; // 选择同步TRACE模式
    // 168MHz时钟下，设置TPIU波特率为115200
    TPI->SPPR = 0;                                 // 选择跟踪端口协议为同步模式
    TPI->FFCR = 0x102;                             // 选择同步模式，数据长度为4
    CoreDebug->DEMCR = CoreDebug_DEMCR_TRCENA_Msk; // 使能调试模块
    ITM->LAR = 0xC5ACCE55;                         // 解锁ITM
    ITM->TCR = 0x00010005;                         // 启动ITM
    printf("ITM->TCR");
    ITM->TER = 0x00000001; // 使能ITM的第一个通道 端口0
    ITM->TPR = 0x1;        // 取消屏蔽激励端口7：0,使能ITM的第一个通道 端口0
    // 配置PB3为TRACE功能
    RCM_EnableAHB1PeriphClock(RCM_AHB1_PERIPH_GPIOE);
    GPIO_ConfigPinAF(GPIOE, GPIO_PIN_SOURCE_2, GPIO_AF_TRACE); // TRACECLK
    GPIO_ConfigPinAF(GPIOE, GPIO_PIN_SOURCE_3, GPIO_AF_TRACE); // TRACED0
    GPIO_ConfigPinAF(GPIOE, GPIO_PIN_SOURCE_4, GPIO_AF_TRACE); // TRACED1
    GPIO_ConfigPinAF(GPIOE, GPIO_PIN_SOURCE_5, GPIO_AF_TRACE); // TRACED2
    GPIO_ConfigPinAF(GPIOE, GPIO_PIN_SOURCE_6, GPIO_AF_TRACE); // TRACED3
    GPIO_Config_T gpioConfig;
    gpioConfig.pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6; // 配置PE2~PE6为TRACE功能
    gpioConfig.mode = GPIO_MODE_AF;
    gpioConfig.speed = GPIO_SPEED_50MHz;
    gpioConfig.otype = GPIO_OTYPE_PP;
    gpioConfig.pupd = GPIO_PUPD_NOPULL;
    GPIO_Config(GPIOE, &gpioConfig);
    printf("SyncTraceInit success!\r\n");
}

#endif
void ItmTask(void)
{

    TraceInit();
    while (1)
    {
        APM_EVAL_LEDToggle(LED2);
        ITM_SendChar(0x55);
        LOS_TaskDelay(1000); // 1s 延时
    }
}
VOID DbgTaskSample(VOID)
{
    UINT32 ret;
    UINT32 taskID1;
    TSK_INIT_PARAM_S stTask = {0};
    printf("TaskSample: Task1 create start...\n");
    stTask.pfnTaskEntry = (TSK_ENTRY_FUNC)ItmTask;
    stTask.uwStackSize = 4096;
    stTask.pcName = "HelloWorld";
    stTask.usTaskPrio = 6;
    ret = LOS_TaskCreate(&taskID1, &stTask);
    if (ret != LOS_OK)
    {
        printf("Task1 create failed\n");
    }
}
/* PLL_VCO = (HSE_VALUE or HSI_VALUE / PLL_B) * PLL_A */
#define PLL_B 8

/* USB OTG FS, SDIO and RNG Clock =  PLL_VCO / PLL_D */
#define PLL_D 7

#define PLL_A 336
/* SYSCLK = PLL_VCO / PLL_C */
#define PLL_C 2
static void SystemClockConfig(void)
{
    __IO uint32_t i;

    RCM->CTRL_B.HSEEN = BIT_SET;

    for (i = 0; i < HSE_STARTUP_TIMEOUT; i++)
    {
        if (RCM->CTRL_B.HSERDYFLG)
        {
            break;
        }
    }

    if (RCM->CTRL_B.HSERDYFLG)
    {
        /* Select regulator voltage output Scale 1 mode */
        RCM->APB1CLKEN_B.PMUEN = BIT_SET;
        PMU->CTRL_B.VOSSEL = BIT_SET;

        /* HCLK = SYSCLK / 256*/
        RCM->CFG_B.AHBPSC = 0x0E;

        /* PCLK2 = HCLK / 2*/
        RCM->CFG_B.APB2PSC = 0x04;

        /* PCLK1 = HCLK / 4*/
        RCM->CFG_B.APB1PSC = 0x05;

        /* Configure the main PLL */
        RCM->PLL1CFG = PLL_B | (PLL_A << 6) | (((PLL_C >> 1) - 1) << 16) | (PLL_D << 24);
        RCM->PLL1CFG_B.PLL1CLKS = 0x01;

        /* Enable the main PLL */
        RCM->CTRL_B.PLL1EN = BIT_SET;

        /* Wait till the main PLL is ready */
        while (RCM->CTRL_B.PLL1RDYFLG == 0)
        {
        }

        /* Configure Flash prefetch, Instruction cache, Data cache and wait state */
        FMC->ACCTRL = 0x05 | BIT8 | BIT9 | BIT10;

        /* Select the main PLL as system clock source */
        RCM->CFG_B.SCLKSEL = RESET;
        RCM->CFG_B.SCLKSEL = 0x02;

        /* Wait till the main PLL is used as system clock source */
        while ((RCM->CFG_B.SCLKSWSTS) != 0x02)
        {
        }
    }
    else
    {
        /* If HSE fails to start-up, the application will have wrong clock configuration. */
    }
}

SYS_RUN(DbgTaskSample);