/*!
 * @file        bsp_delay.h
 *
 * @brief       Header for bsp_delay.c module
 *
 * @version     V1.0.0
 *
 * @date        2023-07-31
 *
 * @attention
 *
 *  Copyright (C) 2021-2023 Geehy Semiconductor
 *
 *  You may not use this file except in compliance with the
 *  GEEHY COPYRIGHT NOTICE (GEEHY SOFTWARE PACKAGE LICENSE).
 *
 *  The program is only for reference, which is distributed in the hope
 *  that it will be useful and instructional for customers to develop
 *  their software. Unless required by applicable law or agreed to in
 *  writing, the program is distributed on an "AS IS" BASIS, WITHOUT
 *  ANY WARRANTY OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the GEEHY SOFTWARE PACKAGE LICENSE for the governing permissions
 *  and limitations under the License.
 */

/* Define to prevent recursive inclusion */
#ifndef _BSP_DELAY_H
#define _BSP_DELAY_H

/* Includes */
#include "main.h"

/** @addtogroup Board
  @{
*/

/** @addtogroup Board_APM32F411_TINY
  @{
*/

/** @defgroup Board_APM32F411_TINY Macros
  @{
*/

/* 100KHz */
#define SYSTICK_FRQ         1000U

/**
  * @}
  */

/** @defgroup APM32F411_TINY_Variables
  @{
  */

/* extern variables*/
extern volatile uint32_t cntUs;
extern volatile uint32_t cntMs;

/**
  * @}
  */

/** @defgroup APM32F411_TINY_Functions
  @{
  */

/* function declaration*/
void Delay_Init(void);
void Delay_ms(__IO u32 nms);
void Delay_us(__IO u32 nus);


/* function declaration*/
void APM_DelayInit(void);
void APM_DelayTickDec(void);
uint32_t APM_ReadTick(void);

/* Delay*/
void APM_DelayMs(__IO uint32_t nms);
void APM_DelayUs(__IO uint32_t nus);
#endif

/**@} end of group APM32F411_TINY_Functions */
/**@} end of group Board_APM32F411_TINY */
/**@} end of group Board */
