/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "apm32f4xx.h"
#include "stdio.h"



#define APM_BOARD_COMInit    APM_EVAL_COMInit
#define APM_BOARD_LEDInit    APM_EVAL_LEDInit
#define APM_BOARD_LEDOn      APM_EVAL_LEDOn
#define APM_BOARD_LEDOff     APM_EVAL_LEDOff
#define APM_BOARD_LEDToggle  APM_EVAL_LEDToggle
#define APM_BOARD_PBInit     APM_EVAL_PBInit
#define APM_BOARD_PBGetState APM_EVAL_PBGetState


// extend the a privat flag append enum I2cFlag in i2c_if.h
// this mode doesn't care about the MemAddress
#define I2C_FLAG_RXTX_RAW (0x2)
/* Definition for USARTx - debug */
#define USARTx                           USART1

#define OHOS_APP_RUN(func) \
void ohos_app_main(void) { \
    LOS_TaskDelay(100); \
    printf("\n\033[1;32m<--------------- OHOS Application Start Here --------------->\033[0m\n"); \
    func(); \
}

#endif /* __MAIN_H */

