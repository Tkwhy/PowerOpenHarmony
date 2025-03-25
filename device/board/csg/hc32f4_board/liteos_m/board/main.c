/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (C) 2022, Xiaohua Semiconductor Co.
  * All rights reserved.
  *
  * This software component is licensed by XHSC under BSD 3-Clause license
  * (the "License"); You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                    opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "los_config.h"
#include "ev_hc32f4a0_lqfp176.h"
#include "hc32_ll.h"

#define LL_PERIPH_SEL (LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_EFM | LL_PERIPH_SRAM)

static void LED_Init(void)
{
    stc_gpio_init_t stcGpioInit;

    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinState = PIN_STAT_RST;
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    (void)GPIO_Init(GPIO_PORT_C, GPIO_PIN_10, &stcGpioInit);
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    int ret;

    LL_PERIPH_WE(LL_PERIPH_SEL);
    /* LED initialize */
    LED_Init();
    BSP_CLK_Init();
    DDL_PrintfInit(BSP_PRINTF_DEVICE, 115200, BSP_PRINTF_Preinit);
    printf("%s user main!\n", __TIME__);
    /* Register write protected for some required peripherals. */
    LL_PERIPH_WP(LL_PERIPH_GPIO);

    ret = LOS_KernelInit();
    if (ret == LOS_OK) {
        OHOS_SystemInit();
        HashTest();
        LOS_Start();
    }
    /* Infinite loop */
    while (1)
    {
    }
}

/**
  * @}
  */

/**
  * @}
  */

