/*!
 * @file       BOARD_LAN8720A.c
 *
 * @brief      This file provides all the config LAN8720A functions
 *
 * @version    V1.0.0
 *
 * @date       2022-05-25
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
#include "lwip/opt.h"
#include "lwip/netif.h"

// #include "main.h"
#include "Board_APM32F407_EVAL.h"

#include "apm32f4xx_eth.h"
#include "board_LAN8720A.h"

/** ETH Init Structure */
ETH_Config_T ETH_InitStructure;
/** lwip network interface structure for ethernetif */
extern struct netif UserNetif;

/** Configure the GPIO ports for ethernet pins */
static void ConfigEthernetGPIO(void);
/** Configure the Ethernet MAC/DMA */
static void ConfigEthernetMACDMA(void);
/** Ethernet Reset Delay */
static void EthResetDelay(__IO uint32_t count);

/*!
 * @brief  configurate Ethernet.
 *
 * @param  None
 *
 * @retval None
 *
 * @note
 */
void ConfigEthernet(void)
{
    /** Configure the GPIO ports for ethernet pins */
    ConfigEthernetGPIO();

    /** Configure the Ethernet MAC/DMA */
    ConfigEthernetMACDMA();
}

/*!
 * @brief  configurate Ethernet Interface.
 *
 * @param  None
 *
 * @retval None
 *
 * @note
 */
static void ConfigEthernetMACDMA(void)
{
    /** Enable ETHERNET clock  */
    RCM_EnableAHB1PeriphClock(RCM_AHB1_PERIPH_ETH_MAC | RCM_AHB1_PERIPH_ETH_MAC_Tx |
                              RCM_AHB1_PERIPH_ETH_MAC_Rx);

    /** Reset ETH on AHB Bus */
    ETH_Reset();

    /** Software reset */
    ETH_SoftwareReset();

    /** Wait for software reset */
    while (ETH_ReadSoftwareReset() == SET)
        ;
    /** ETH Config Struct */
    ETH_ConfigStructInit(&ETH_InitStructure);

    /** ETH Config MAC */
    /**
     *ETH_InitStructure.autoNegotiation = ETH_AUTONEGOTIATION_DISABLE;
     *ETH_InitStructure.speed = ETH_SPEED_100M;
     *ETH_InitStructure.mode = ETH_MODE_FULLDUPLEX;
     */
    ETH_InitStructure.autoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
    ETH_InitStructure.loopbackMode = ETH_LOOPBACKMODE_DISABLE;
    ETH_InitStructure.retryTransmission = ETH_RETRYTRANSMISSION_DISABLE;
    ETH_InitStructure.automaticPadCRCStrip = ETH_AUTOMATICPADCRCSTRIP_DISABLE;
    ETH_InitStructure.receiveAll = ETH_RECEIVEAll_DISABLE;
    ETH_InitStructure.broadcastFramesReception = ETH_BROADCASTFRAMESRECEPTION_ENABLE;
    ETH_InitStructure.promiscuousMode = ETH_PROMISCUOUS_MODE_DISABLE;
    ETH_InitStructure.multicastFramesFilter = ETH_MULTICASTFRAMESFILTER_PERFECT;
    ETH_InitStructure.unicastFramesFilter = ETH_UNICASTFRAMESFILTER_PERFECT;
#ifdef HARDWARE_CHECKSUM
    ETH_InitStructure.checksumOffload = ETH_CHECKSUMOFFLAOD_ENABLE;
#endif

    /** ETH Config DMA */
    ETH_InitStructure.dropTCPIPChecksumErrorFrame = ETH_DROPTCPIPCHECKSUMERRORFRAME_ENABLE;
    ETH_InitStructure.receiveStoreForward = ETH_RECEIVESTOREFORWARD_ENABLE;
    ETH_InitStructure.flushReceivedFrame = ETH_FLUSHRECEIVEDFRAME_DISABLE;
    ETH_InitStructure.transmitStoreForward = ETH_TRANSMITSTOREFORWARD_ENABLE;

    ETH_InitStructure.forwardErrorFrames = ETH_FORWARDERRORFRAMES_DISABLE;
    ETH_InitStructure.forwardUndersizedGoodFrames = ETH_FORWARDUNDERSIZEDGOODFRAMES_DISABLE;
    ETH_InitStructure.secondFrameOperate = ETH_SECONDFRAMEOPERARTE_ENABLE;
    ETH_InitStructure.addressAlignedBeats = ETH_ADDRESSALIGNEDBEATS_ENABLE;
    ETH_InitStructure.fixedBurst = ETH_FIXEDBURST_ENABLE;
    ETH_InitStructure.rxDMABurstLength = ETH_RXDMABURSTLENGTH_32BEAT;
    ETH_InitStructure.txDMABurstLength = ETH_TXDMABURSTLENGTH_32BEAT;
    ETH_InitStructure.DMAArbitration = ETH_DMAARBITRATION_ROUNDROBIN_RXTX_2_1;

    /** Configure Ethernet */
    ETH_Config(&ETH_InitStructure, LAN8720A_PHY_ADDRESS);
}

/*!
 * @brief  configurate Ethernet GPIO.
 *
 * @param  None
 *
 * @retval None
 *
 * @note
 */
void ConfigEthernetGPIO(void)
{
    GPIO_Config_T configStruct;
    /**
     *   ETH_RESET#                          PD11
     *   ETH_MDIO                            PA2
     *   ETH_MDC                             PC1
     */
    /** Enable GPIOs clocks */
    RCM_EnableAHB1PeriphClock(ETH_MDC_GPIO_CLK | ETH_MDIO_GPIO_CLK |
                              ETH_RESET_GPIO_CLK);

    /** Enable SYSCFG clock */
    RCM_EnableAPB2PeriphClock(RCM_APB2_PERIPH_SYSCFG);

    /** Configure ETH_RESET# */
    GPIO_ConfigStructInit(&configStruct);
    configStruct.pin = ETH_RESET_PIN;
    configStruct.speed = GPIO_SPEED_100MHz;
    configStruct.mode = GPIO_MODE_OUT;
    configStruct.otype = GPIO_OTYPE_PP;
    configStruct.pupd = GPIO_PUPD_NOPULL;
    GPIO_Config(ETH_RESET_PORT, &configStruct);

    /** Configure ETH_MDC */
    configStruct.pin = ETH_MDC_PIN;
    configStruct.mode = GPIO_MODE_AF;
    GPIO_Config(ETH_MDC_PORT, &configStruct);
    GPIO_ConfigPinAF(ETH_MDC_PORT, ETH_MDC_SOURCE, ETH_MDC_AF);

    /** Configure ETH_MDIO */
    configStruct.pin = ETH_MDIO_PIN;
    GPIO_Config(ETH_MDIO_PORT, &configStruct);
    GPIO_ConfigPinAF(ETH_MDIO_PORT, ETH_MDIO_SOURCE, ETH_MDIO_AF);

    /** RESET ETH PHY */
    ETH_RESET_PIN_LOW();
    EthResetDelay(LAN8720A_RESET_DELAY);
    ETH_RESET_PIN_HIGH();
    EthResetDelay(LAN8720A_RESET_DELAY);

/** if BOARD_LAN8720A.h define MII_MODE, this function configurate MII_MODE */
#ifdef MII_MODE /** Mode MII */

#elif defined RMII_MODE /** Mode RMII */

    /**
     *  ETH_RMII_REF_CLK    PA1
     *  ETH_RMII_CRS_DV     PA7
     *  ETH_RMII_RXD0       PC4
     *  ETH_RMII_RXD1       PC5
     *  ETH_RMII_TX_EN      PB11
     *  ETH_RMII_TXD0       PB12
     *  ETH_RMII_TXD1       PB13
     */

    /** Enable GPIOs clocks */
    RCM_EnableAHB1PeriphClock(ETH_RMII_REF_CLK_GPIO_CLK | ETH_RMII_CRS_DV_GPIO_CLK |
                              ETH_RMII_RXD0_GPIO_CLK | ETH_RMII_RXD1_GPIO_CLK |
                              ETH_RMII_TX_EN_GPIO_CLK | ETH_RMII_TXD0_GPIO_CLK |
                              ETH_RMII_TXD1_GPIO_CLK);

    /** Configure ETH_RMII_REF_CLK */
    configStruct.pin = ETH_RMII_REF_CLK_PIN;
    configStruct.mode = GPIO_MODE_AF;
    configStruct.speed = GPIO_SPEED_100MHz;
    GPIO_Config(ETH_RMII_REF_CLK_PORT, &configStruct);
    GPIO_ConfigPinAF(ETH_RMII_REF_CLK_PORT, ETH_RMII_REF_CLK_SOURCE, ETH_RMII_REF_CLK_AF);

    /** Configure ETH_RMII_CRS_DV */
    configStruct.pin = ETH_RMII_CRS_DV_PIN;
    GPIO_Config(ETH_RMII_CRS_DV_PORT, &configStruct);
    GPIO_ConfigPinAF(ETH_RMII_CRS_DV_PORT, ETH_RMII_CRS_DV_SOURCE, ETH_RMII_CRS_DV_AF);

    /** Configure ETH_RMII_RXD0 */
    configStruct.pin = ETH_RMII_RXD0_PIN;
    GPIO_Config(ETH_RMII_RXD0_PORT, &configStruct);
    GPIO_ConfigPinAF(ETH_RMII_RXD0_PORT, ETH_RMII_RXD0_SOURCE, ETH_RMII_RXD0_AF);

    /** Configure ETH_RMII_RXD1 */
    configStruct.pin = ETH_RMII_RXD1_PIN;
    GPIO_Config(ETH_RMII_RXD1_PORT, &configStruct);
    GPIO_ConfigPinAF(ETH_RMII_RXD1_PORT, ETH_RMII_RXD1_SOURCE, ETH_RMII_RXD1_AF);

    /** Configure ETH_RMII_TX_EN */
    configStruct.pin = ETH_RMII_TX_EN_PIN;
    GPIO_Config(ETH_RMII_TX_EN_PORT, &configStruct);
    GPIO_ConfigPinAF(ETH_RMII_TX_EN_PORT, ETH_RMII_TX_EN_SOURCE, ETH_RMII_TX_EN_AF);

    /** Configure ETH_RMII_TXD0 */
    configStruct.pin = ETH_RMII_TXD0_PIN;
    GPIO_Config(ETH_RMII_TXD0_PORT, &configStruct);
    GPIO_ConfigPinAF(ETH_RMII_TXD0_PORT, ETH_RMII_TXD0_SOURCE, ETH_RMII_TXD0_AF);

    /** Configure ETH_RMII_TXD1 */
    configStruct.pin = ETH_RMII_TXD1_PIN;
    GPIO_Config(ETH_RMII_TXD1_PORT, &configStruct);
    GPIO_ConfigPinAF(ETH_RMII_TXD1_PORT, ETH_RMII_TXD1_SOURCE, ETH_RMII_TXD1_AF);

    SYSCFG_ConfigMediaInterface(SYSCFG_INTERFACE_RMII);
#endif
}

/*!
 * @brief  Ethernet Reset Delay.
 *
 * @param  count  Reset Delay Time
 *
 * @retval None
 *
 * @note
 */
void EthResetDelay(__IO uint32_t count)
{
    __IO uint32_t i = 0;
    for (i = count; i != 0; i--)
    {
    }
}
