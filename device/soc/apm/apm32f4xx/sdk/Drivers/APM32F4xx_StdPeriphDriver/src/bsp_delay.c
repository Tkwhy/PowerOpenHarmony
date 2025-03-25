/*!
 * @file        bsp_delay.c
 *
 * @brief       Delay board support package body
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

/* Includes */
#include "bsp_delay.h"

/** @addtogroup Board
  @{
*/

/** @addtogroup Board_APM32F411_TINY
  @{
*/

/** @defgroup APM32F411_TINY_Variables
  @{
  */

/* public variables*/
/* The count of microseconds*/
volatile uint32_t cntUs = 0;
/* The count of milliseconds*/
volatile uint32_t cntMs = 0;

__IO uint32_t delayTick;

/**
  * @}
  */

/** @defgroup APM32F411_TINY_Fuctions
  @{
  */

/*!
 * @brief       Update SystemCoreClock variable according to Clock Register Values
 *              The SystemCoreClock variable contains the core clock (HCLK)
 *
 * @param       None
 *
 * @retval      None
 *
 */
void Delay_Init(void)
{
    SystemCoreClockUpdate();
}


/*!
 * @brief       Configures Delay.
 *
 * @param       None
 *
 * @retval      None
 */
void APM_DelayInit(void)
{
    if (SysTick_Config(SystemCoreClock / 1000 / SYSTICK_FRQ))
    {
        while (1);
    }

    NVIC_SetPriority(SysTick_IRQn, 0U);
}

/*!
 * @brief       Decrement tick
 *
 * @param       None
 *
 * @retval      None
 */
void APM_DelayTickDec(void)
{
    if (delayTick != 0x00)
    {
        delayTick--;
    }
}

/*!
 * @brief       Configures Delay ms.
 *
 * @param       nms: Specifies the delay to be configured.
 *              This parameter can be one of following parameters:
 *              @arg nms
 *
 *
 * @retval      None
 */
void Delay_ms(__IO u32 nms)
{
    SysTick_Config(SystemCoreClock / 1000);

    cntMs = nms;
    while(cntMs != 0);
}

/*!
 * @brief       Configures Delay us.
 *
 * @param       nus: Specifies the delay to be configured.
 *              This parameter can be one of following parameters:
 *              @arg nus
 *
 *
 * @retval      None
 */
void Delay_us(__IO u32 nus)
{
    SysTick_Config(SystemCoreClock / 1000000);

    cntUs = nus;
    while(cntUs != 0);
}

/*!
 * @brief       Configures Delay us.
 *
 * @param       nus: Specifies the delay to be configured.
 *              This parameter can be one of following parameters:
 *              @arg nus
 *
 * @retval      None
 */
void APM_DelayUs(__IO uint32_t nus)
{
    delayTick = nus;

    while (delayTick != 0);
}

/*!
 * @brief       Configures Delay ms.
 *
 * @param       nms: Specifies the delay to be configured.
 *              This parameter can be one of following parameters:
 *              @arg nms
 *
 * @retval      None
 */
void APM_DelayMs(__IO uint32_t nms)
{
    delayTick = nms * 1000;

    while (delayTick != 0);
}

/*!
 * @brief       Read tick value
 *
 * @param       nus: Specifies the delay to be configured.
 *              This parameter can be one of following parameters:
 *              @arg nus
 *
 * @retval      None
 */
uint32_t APM_ReadTick(void)
{
    return delayTick;
}

/**@} end of group APM32F411_TINY_Functions */
/**@} end of group Board_APM32F411_TINY */
/**@} end of group Board */
