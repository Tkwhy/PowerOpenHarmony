/*!
 * @file       BOARD_LAN8720A.h
 *
 * @brief      This file provides all the config DP83848 functions
 *
 * @version    V1.0.0
 *
 * @date        2022-02-25
 *
 * @attention
 *
 *  Copyright (C) 2021-2022 Geehy Semiconductor
 *
 *  You may not use this file except in compliance with the
 *  GEEHY COPYRIGHT NOTICE (GEEHY SOFTWARE PACKAGE LICENSE).
 *
 *  The program is only for reference, which is distributed in the hope
 *  that it will be usefull and instructional for customers to develop
 *  their software. Unless required by applicable law or agreed to in
 *  writing, the program is distributed on an "AS IS" BASIS, WITHOUT
 *  ANY WARRANTY OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the GEEHY SOFTWARE PACKAGE LICENSE for the governing permissions
 *  and limitations under the License.
 */

#ifndef __BOARD_LAN8720A_H
#define __BOARD_LAN8720A_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "apm32f4xx_eth.h"
#include "lwip/netif.h"

/** LAN8720A_PHY_ADDRESS */
#define LAN8720A_PHY_ADDRESS 0x00

/** MII and RMII mode selection */
#if defined(APM32F407_MINI)
#define RMII_MODE
#elif defined(APM32F407_ELE_HUETB)
#define MII_MODE
#elif defined(APM32F407_EVAL)
#define RMII_MODE
#else
// #error "Please select first the APM32  board to be used (in board.h)"
#endif

/** Reset delay */
#define LAN8720A_RESET_DELAY PHY_RESET_DELAY

/** define LAN8720A GPIO */
#ifdef MII_MODE

#elif defined RMII_MODE
/**
 *  ETH_RMII_REF_CLK    PA1
 *  ETH_RMII_CRS_DV     PA7
 *  ETH_RMII_RXD0       PC4
 *  ETH_RMII_RXD1       PC5
 *  ETH_RMII_TX_EN      PG11
 *  ETH_RMII_TXD0       PG13
 *  ETH_RMII_TXD1       PG14
 */
/** ETH_RMII_REF_CLK PA1 */
#define ETH_RMII_REF_CLK_GPIO_CLK RCM_AHB1_PERIPH_GPIOA
#define ETH_RMII_REF_CLK_PORT GPIOA
#define ETH_RMII_REF_CLK_PIN GPIO_PIN_1
#define ETH_RMII_REF_CLK_AF GPIO_AF_ETH
#define ETH_RMII_REF_CLK_SOURCE GPIO_PIN_SOURCE_1
/** ETH_RMII_CRS_DV     PA7 */
#define ETH_RMII_CRS_DV_GPIO_CLK RCM_AHB1_PERIPH_GPIOA
#define ETH_RMII_CRS_DV_PORT GPIOA
#define ETH_RMII_CRS_DV_PIN GPIO_PIN_7
#define ETH_RMII_CRS_DV_AF GPIO_AF_ETH
#define ETH_RMII_CRS_DV_SOURCE GPIO_PIN_SOURCE_7
/** ETH_RMII_RXD0 PC4 */
#define ETH_RMII_RXD0_GPIO_CLK RCM_AHB1_PERIPH_GPIOC
#define ETH_RMII_RXD0_PORT GPIOC
#define ETH_RMII_RXD0_PIN GPIO_PIN_4
#define ETH_RMII_RXD0_AF GPIO_AF_ETH
#define ETH_RMII_RXD0_SOURCE GPIO_PIN_SOURCE_4
/** ETH_RMII_RXD1 PC5 */
#define ETH_RMII_RXD1_GPIO_CLK RCM_AHB1_PERIPH_GPIOC
#define ETH_RMII_RXD1_PORT GPIOC
#define ETH_RMII_RXD1_PIN GPIO_PIN_5
#define ETH_RMII_RXD1_AF GPIO_AF_ETH
#define ETH_RMII_RXD1_SOURCE GPIO_PIN_SOURCE_5
/** ETH_RMII_TX_EN PG11 */
#define ETH_RMII_TX_EN_GPIO_CLK RCM_AHB1_PERIPH_GPIOG
#define ETH_RMII_TX_EN_PORT GPIOG
#define ETH_RMII_TX_EN_PIN GPIO_PIN_11
#define ETH_RMII_TX_EN_AF GPIO_AF_ETH
#define ETH_RMII_TX_EN_SOURCE GPIO_PIN_SOURCE_11
/** ETH_RMII_TXD0 PG13 */
#define ETH_RMII_TXD0_GPIO_CLK RCM_AHB1_PERIPH_GPIOG
#define ETH_RMII_TXD0_PORT GPIOG
#define ETH_RMII_TXD0_PIN GPIO_PIN_13
#define ETH_RMII_TXD0_AF GPIO_AF_ETH
#define ETH_RMII_TXD0_SOURCE GPIO_PIN_SOURCE_13
/** ETH_RMII_TXD1 PG14 */
#define ETH_RMII_TXD1_GPIO_CLK RCM_AHB1_PERIPH_GPIOG
#define ETH_RMII_TXD1_PORT GPIOG
#define ETH_RMII_TXD1_PIN GPIO_PIN_14
#define ETH_RMII_TXD1_AF GPIO_AF_ETH
#define ETH_RMII_TXD1_SOURCE GPIO_PIN_SOURCE_14
#endif

/**
 *  ETH_RESET#                          PD11
 *  ETH_MDIO                            PA2
 *  ETH_MDC                             PC1
 */

/** ETH_RESET# PD3 */
#define ETH_RESET_GPIO_CLK RCM_AHB1_PERIPH_GPIOD
#define ETH_RESET_PORT GPIOD
#define ETH_RESET_PIN GPIO_PIN_11
#define ETH_RESET_PIN_HIGH() GPIO_SetBit(ETH_RESET_PORT, ETH_RESET_PIN)
#define ETH_RESET_PIN_LOW() GPIO_ResetBit(ETH_RESET_PORT, ETH_RESET_PIN)
  /** ETH_MII_MDC PC1 */
#define ETH_MDC_GPIO_CLK RCM_AHB1_PERIPH_GPIOC
#define ETH_MDC_PORT GPIOC
#define ETH_MDC_PIN GPIO_PIN_1
#define ETH_MDC_AF GPIO_AF_ETH
#define ETH_MDC_SOURCE GPIO_PIN_SOURCE_1
/** ETH_MII_MDIO PA2 */
#define ETH_MDIO_GPIO_CLK RCM_AHB1_PERIPH_GPIOA
#define ETH_MDIO_PORT GPIOA
#define ETH_MDIO_PIN GPIO_PIN_2
#define ETH_MDIO_AF GPIO_AF_ETH
#define ETH_MDIO_SOURCE GPIO_PIN_SOURCE_2

  void ConfigEthernet(void);

#ifdef __cplusplus
}
#endif

#endif /** __BOARD_LAN8720A_H */
