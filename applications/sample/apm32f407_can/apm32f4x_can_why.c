#include <stdio.h>
#include "ohos_init.h"
#include "ohos_types.h"
#include "los_task.h"
#include "los_sem.h"
#include "Board_APM32F407_EVAL.h"
#include "apm32f4xx_can.h"
#include "apm32f4xx.h"

// 请在BUILD.gn中添加以下内容
// source_set("$(CHIP_LIB)/apm32f4xx_can.c")

// CAN_MODE  = 0  正常收发模式
// CAN_MODE  = 1  回环模式
#define CAN_MODE 1 /*!< normal mode */

#define CAN_BAUD_125K 0 /*!< CAN baud rate config to 125K */
#define CAN_BAUD_250K 1 /*!< CAN baud rate config to 250K */
#define CAN_BAUD_500K 2 /*!< CAN baud rate config to 500K */
#define CAN_BAUD_1M 3   /*!< CAN baud rate config to 1M */
#define TIME1 CAN_TIME_SEGMENT1_2
#define TIME2 CAN_TIME_SEGMENT2_3
#define PRES 14

static void CAN_demo_Init(uint8_t baud);
static void CAN_GPIOInit(void);
void CAN_TxMessageInit(CAN_TxMessage_T *txMessage);
void CAN_RxMessageInit(CAN_RxMessage_T *rxMessage);
void CAN2_RX0_IRQHandler(void);

/* 定义二值信号量的 ID 变量 */
UINT32 CanRx_BinarySem_Handle;
CAN_RxMessage_T RxMessage;
CAN_TxMessage_T TxMessage;
static void can_task(void)
{
    UINT32 ret, i;

    CAN_TX_MAILBIX_T TransmitMailbox = CAN_TX_MAILBIX_1;
    CAN_demo_Init(CAN_BAUD_500K);
    /* Reset Message */
    CAN_TxMessageInit(&TxMessage);
    CAN_RxMessageInit(&RxMessage);
    CAN_EnableInterrupt(CAN2, CAN_INT_F0MP);
    LOS_HwiCreate(CAN2_RX0_IRQn, 0, 0, CAN2_RX0_IRQHandler, 0); // 配置中断

#if CAN_MODE
    printf("CAN1 Loopback Mode\r\n");

#else
    printf("CAN2 Normal Mode\r\n");
#endif
    while (1)
    {
#if CAN_MODE
        TxMessage.stdID = 0x11;
        TxMessage.remoteTxReq = CAN_RTXR_DATA;
        TxMessage.typeID = CAN_TYPEID_STD;
        TxMessage.dataLengthCode = 2;
        TxMessage.data[0] = 0xCA;
        TxMessage.data[1] = 0xFE;
        TransmitMailbox = (CAN_TX_MAILBIX_T)CAN_TxMessage(CAN1, &TxMessage);
        LOS_TaskDelay(10);
        /* receive */
        RxMessage.stdID = 0x00;
        RxMessage.typeID = CAN_TYPEID_STD;
        RxMessage.dataLengthCode = 0;
        RxMessage.data[0] = 0x00;
        RxMessage.data[1] = 0x00;
        CAN_RxMessage(CAN1, CAN_RX_FIFO_0, &RxMessage);
        if ((RxMessage.stdID == 0x11) && (RxMessage.typeID == CAN_TYPEID_STD) && (RxMessage.dataLengthCode == 2) && (RxMessage.data[0] == 0xCA && RxMessage.data[1] == 0xFE))
        {
            printf("CAN1 Loopback Mode: receive success\r\n");
        }
        else
        {
            printf("CAN1 Loopback Mode: receive failed\r\n");
        }
        LOS_TaskDelay(1000);
#else
        printf("CAN2 Normal Mode\r\n");

        ret = LOS_SemPend(CanRx_BinarySem_Handle, LOS_WAIT_FOREVER); // 等待二值信号量
        if (ret == LOS_OK)
        {
            printf("stdID: %x", RxMessage.stdID);
            printf("typeID: %x", RxMessage.typeID);
            printf("dataLengthCode: %x", RxMessage.dataLengthCode);
            printf("data[0]: %x", RxMessage.data[0]);
            printf("data[1]: %x", RxMessage.data[1]);
            TxMessage.stdID = RxMessage.stdID;
            TxMessage.remoteTxReq = RxMessage.typeID;
            TxMessage.typeID = CAN_TYPEID_STD;
            TxMessage.dataLengthCode = RxMessage.dataLengthCode;
            for (i = 0; i < RxMessage.dataLengthCode; i++)
            {
                printf("data[%d]: %x", i, RxMessage.data[i]);
                TxMessage.data[i] = RxMessage.data[i];
            }
            TransmitMailbox = (CAN_TX_MAILBIX_T)CAN_TxMessage(CAN2, &TxMessage);
        }
        else
        {
            printf("LOS_SemPend failed\n");
        }
#endif
    }
}
VOID CanTaskSample(VOID)
{
    UINT32 ret;
    UINT32 taskID1;
    TSK_INIT_PARAM_S stTask = {0};
    printf("apm32f407_can Sample: Task1 create start...\n");
    ret = LOS_BinarySemCreate(0, &CanRx_BinarySem_Handle); // 创建二值信号量
    if (ret != LOS_OK)
    {
        printf("Create BinarySem failed\n");
    }
    stTask.pfnTaskEntry = (TSK_ENTRY_FUNC)can_task;
    stTask.uwStackSize = 4096;
    stTask.pcName = "HelloWorld";
    stTask.usTaskPrio = 6;
    ret = LOS_TaskCreate(&taskID1, &stTask);
    if (ret != LOS_OK)
    {
        printf("Task1 create failed\n");
    }
}

void CAN2_RX0_IRQHandler(void)
{
    CAN_RxMessage(CAN2, CAN_RX_FIFO_0, &RxMessage); // 读取接收到的数据
    LOS_SemPost(CanRx_BinarySem_Handle);            // 释放二值信号量
}
/*!
 * @brief     CAN initialization
 *
 * @param     baud: baud rate for CAN
 *                  This parameter can be one of the following values:
 *                  @arg CAN_BAUD_125K : CAN baud rate config to 125K
 *                  @arg CAN_BAUD_250K : CAN baud rate config to 250K
 *                  @arg CAN_BAUD_500K : CAN baud rate config to 500K
 *                  @arg CAN_BAUD_1M   : CAN baud rate config to 1M
 *
 * @retval    None
 */
static void CAN_demo_Init(uint8_t baud)
{
    CAN_Config_T CAN_ConfigStructure;
    CAN_FilterConfig_T CAN_FilterStruct;
    uint32_t apb1Clock;
    uint16_t prescaler;
    uint16_t timeSegment1;
    uint16_t timeSegment2;

    RCM_EnableAPB1PeriphClock(RCM_APB1_PERIPH_CAN1 | RCM_APB1_PERIPH_CAN2);

    RCM_ReadPCLKFreq(&apb1Clock, NULL);
    apb1Clock /= 1000000;

    /* Config GPIO for CAN */
    CAN_GPIOInit();

    /* CAN register init */
    CAN_Reset(CAN1);
    CAN_Reset(CAN2);

    CAN_ConfigStructInit(&CAN_ConfigStructure);

    /* CAN cell init */
    CAN_ConfigStructure.autoBusOffManage = DISABLE;
    CAN_ConfigStructure.autoWakeUpMode = DISABLE;
    CAN_ConfigStructure.nonAutoRetran = DISABLE;
    CAN_ConfigStructure.rxFIFOLockMode = DISABLE;
    CAN_ConfigStructure.txFIFOPriority = DISABLE;
#if CAN_MODE
    CAN_ConfigStructure.mode = CAN_MODE_LOOPBACK;
#else
    CAN_ConfigStructure.mode = CAN_MODE_NORMAL;
#endif
    /* Baudrate = PCLK1 / (prescaler * (timeSegment1 + timeSegment2 + 1)) */
    CAN_ConfigStructure.syncJumpWidth = CAN_SJW_1;
    /* When APB1 Clock is not 42M, you should calculate by you clock!! */
    /* APM32F407 APB1 Clock is 42M */
    /* APM32F411 APB1 Clock is 50M */
    if (baud == CAN_BAUD_500K)
    {
        CAN_ConfigStructure.timeSegment1 = TIME1;
        CAN_ConfigStructure.timeSegment2 = TIME2;
        CAN_ConfigStructure.prescaler = PRES;
    }
    else if (baud == CAN_BAUD_1M)
    {
        CAN_ConfigStructure.timeSegment1 = TIME1;
        CAN_ConfigStructure.timeSegment2 = TIME2;
        CAN_ConfigStructure.prescaler = PRES / 2;
    }
    else //!< 125K and 250K
    {
        CAN_ConfigStructure.timeSegment1 = CAN_TIME_SEGMENT1_3;
        CAN_ConfigStructure.timeSegment2 = CAN_TIME_SEGMENT2_4;
        CAN_ConfigStructure.prescaler = apb1Clock >> baud;
    }
    CAN_Config(CAN1, &CAN_ConfigStructure);
    CAN_Config(CAN2, &CAN_ConfigStructure);

    /* CAN filter init */
    CAN_FilterStruct.filterMode = CAN_FILTER_MODE_IDMASK;
    CAN_FilterStruct.filterScale = CAN_FILTER_SCALE_32BIT;
    CAN_FilterStruct.filterIdHigh = 0x0000;
    CAN_FilterStruct.filterIdLow = 0x0000;
    CAN_FilterStruct.filterMaskIdHigh = 0x0000;
    CAN_FilterStruct.filterMaskIdLow = 0x0000;
    CAN_FilterStruct.filterFIFO = CAN_FILTER_FIFO_0;
    CAN_FilterStruct.filterActivation = ENABLE;

    /* CAN1 filter can be from 0 to 13 */
    CAN_FilterStruct.filterNumber = 0;
    CAN_ConfigFilter(&CAN_FilterStruct);

    /* CAN2 filter can be from 14 to 27(using CAN1 register) */
    CAN_FilterStruct.filterNumber = 14;
    CAN_ConfigFilter(&CAN_FilterStruct);

    prescaler = CAN_ConfigStructure.prescaler;
    timeSegment1 = CAN_ConfigStructure.timeSegment1;
    timeSegment2 = CAN_ConfigStructure.timeSegment2;

    printf("   Configuration :\r\n");
    printf("   APB1 Clock : %d MHz\r\n", (uint16_t)apb1Clock);
    printf("   CAN Baud   : %d K\r\n",
           (uint16_t)(apb1Clock * 1000) / (prescaler * (timeSegment1 + timeSegment2 + 3)));
}

/*!
 * @brief     CAN GPIO initialization
 *
 * @param     None
 *
 * @retval    None
 */
static void CAN_GPIOInit(void)
{
    GPIO_Config_T GPIO_InitStructure;
    RCM_EnableAHB1PeriphClock(RCM_AHB1_PERIPH_GPIOB);
    /* USE_CAN2 */
    GPIO_InitStructure.pin = GPIO_PIN_12 | GPIO_PIN_13;
    GPIO_InitStructure.mode = GPIO_MODE_AF;
    GPIO_InitStructure.otype = GPIO_OTYPE_PP;
    GPIO_InitStructure.speed = GPIO_SPEED_100MHz;
    GPIO_InitStructure.pupd = GPIO_PUPD_UP;
    GPIO_Config(GPIOB, &GPIO_InitStructure);

    GPIO_ConfigPinAF(GPIOB, GPIO_PIN_SOURCE_12, GPIO_AF_CAN2);
    GPIO_ConfigPinAF(GPIOB, GPIO_PIN_SOURCE_13, GPIO_AF_CAN2);
}

/*!
 * @brief     Configures CAN_TxMessage_T structure
 *
 * @param     txMessage : CAN_TxMessage_T structure
 *
 * @retval    None
 */
void CAN_TxMessageInit(CAN_TxMessage_T *txMessage)
{
    txMessage->stdID = 0x00;
    txMessage->extID = 0x00;
    txMessage->typeID = CAN_TYPEID_STD;
    txMessage->dataLengthCode = 0;
    for (uint8_t i = 0; i < 8; i++)
    {
        txMessage->data[i] = 0x00;
    }
}

/*!
 * @brief     Configures CAN_RxMessage_T structure
 *
 * @param     rxMessage : CAN_RxMessage_T structure
 *
 * @retval    None
 */
void CAN_RxMessageInit(CAN_RxMessage_T *rxMessage)
{
    rxMessage->stdID = 0x00;
    rxMessage->extID = 0x00;
    rxMessage->typeID = 0;
    rxMessage->dataLengthCode = 0;
    rxMessage->filterMatchIndex = 0;
    for (uint8_t i = 0; i < 8; i++)
    {
        rxMessage->data[i] = 0x00;
    }
}

SYS_RUN(CanTaskSample);