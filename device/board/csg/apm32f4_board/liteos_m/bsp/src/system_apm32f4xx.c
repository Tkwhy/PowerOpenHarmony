/*!
 * @file        system_apm32f4xx.c
 *
 * @brief       CMSIS Cortex-M4 Device Peripheral Access Layer System Source File
 *
 * @version     V1.0.0
 *
 * @date        2023-07-31
 *
 * @attention
 *
 *  Copyright (C) 2023 Geehy Semiconductor
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
#include "apm32f4xx.h"
#include "system_apm32f4xx.h"
#include "apm32f4xx_dmc.h"
#include "apm32f4xx_gpio.h"
#include "apm32f4xx_rcm.h"

// Init SDRAM
#define RCM_SDRAM_GPIO_PERIPH (RCM_AHB1_PERIPH_GPIOA | \
                               RCM_AHB1_PERIPH_GPIOD | \
                               RCM_AHB1_PERIPH_GPIOF | \
                               RCM_AHB1_PERIPH_GPIOG | \
                               RCM_AHB1_PERIPH_GPIOH | \
                               RCM_AHB1_PERIPH_GPIOI)
#define RCM_AHB3_PERIPH_EMMC (0x00000001)
/** SDRAM Address */
#define SDRAM_START_ADDR ((uint32_t)0x60000000)
#define SDRAM_END_ADDR ((uint32_t)0x60200000)
/** function */
void SDRAM_Init(void);
void SDRAM_GPIOConfig(void);

/** @addtogroup Examples
  @{
  */

/** @addtogroup OTGD_Custom_HID_Keyboard_HS
  @{
  */

/** @defgroup OTGD_Custom_HID_Keyboard_HS_System_Macros System_Macros
  @{
*/

/* Uncomment the following line if you need to use external SRAM as data memory  */
/* #define DATA_IN_ExtSRAM */

/* Uncomment the following line if you need to relocate your vector Table in Internal SRAM. */
/* #define VECT_TAB_SRAM */

/* Vector Table base offset field. This value must be a multiple of 0x200. */
#define VECT_TAB_OFFSET 0x00

/* PLL_VCO = (HSE_VALUE or HSI_VALUE / PLL_B) * PLL_A */
#define PLL_B 8

/* USB OTG FS, SDIO and RNG Clock =  PLL_VCO / PLL_D */
#define PLL_D 7

#define PLL_A 336
/* SYSCLK = PLL_VCO / PLL_C */
#define PLL_C 2

/**@} end of group OTGD_Custom_HID_Keyboard_HS_System_Macros*/

/** @defgroup OTGD_Custom_HID_Keyboard_HS_System_Variables System_Variables
  @{
*/

/**
 * @brief     APM32F4xx_System_Private_Variables
 */

uint32_t SystemCoreClock = 168000000;

__I uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};

/**@} end of group OTGD_Custom_HID_Keyboard_HS_System_Variables*/

/** @defgroup OTGD_Custom_HID_Keyboard_HS_System_Functions System_Functions
  @{
*/

/**
 * @brief    APM32F4xx_System_Private_FunctionPrototypes
 */

static void SystemClockConfig(void);

#if defined(DATA_IN_ExtSRAM)
static void SystemInit_ExtSRAM(void);
#endif /* DATA_IN_ExtSRAM */

/*!
 * @brief     Setup the microcontroller system
 *
 * @param     None
 *
 * @retval    None
 */
void SystemInit(void)
{
/* FPU settings */
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2)); /*!< set CP10 and CP11 Full Access*/
#endif
    /* Reset the RCM clock configuration to the default reset state */
    /* Set HSIEN bit */
    RCM->CTRL_B.HSIEN = BIT_SET;

    /* Reset CFG register */
    RCM->CFG = 0x00000000;

    /* Reset HSEEN, CSSEN and PLL1EN bits */
    RCM->CTRL &= (uint32_t)0xFEF6FFFF;

    /* Reset PLL1CFG register */
    RCM->PLL1CFG = 0x24003010;

    /* Reset HSEBCFG bit */
    RCM->CTRL &= (uint32_t)0xFFFBFFFF;

    /* Disable all interrupts */
    RCM->INT = 0x00000000;

#if defined(DATA_IN_ExtSRAM)
    SystemInit_ExtSRAM();
#endif /* DATA_IN_ExtSRAM */

    SystemClockConfig();
    // Init SDRAM
    RCM_ConfigSDRAM(RCM_SDRAM_DIV_4); // config SDRAM clock
    SDRAM_GPIOConfig();
    SDRAM_Init();

/* Configure the Vector Table location add offset address */
#ifdef VECT_TAB_SRAM
    SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM */
#else
    SCB->VTOR = FMC_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH */
#endif
}

/*!
 * @brief       SDRAM GPIO Config
 *
 * @param       None
 *
 * @retval      None
 *
 */
void SDRAM_GPIOConfig(void)
{
    GPIO_Config_T gpioConfig;

    RCM_EnableAHB1PeriphClock(RCM_SDRAM_GPIO_PERIPH);

    /* SDRAM pin configuration
       A0   ----> PF1
       A1   ----> PF2
       A2   ----> PF3
       A3   ----> PF4
       A4   ----> PF6
       A5   ----> PF7
       A6   ----> PF8
       A7   ----> PF9
       A8   ----> PF10
       A9   ----> PH3
       A10  ----> PF0
       D0   ----> PG3
       D1   ----> PG4
       D2   ----> PG5
       D3   ----> PG6
       D4   ----> PG8
       D5   ----> PH13
       D6   ----> PH15
       D7   ----> PI3
       D8   ----> PH8
       D9   ----> PH10
       D10  ----> PD10
       D11  ----> PD12
       D12  ----> PD13
       D13  ----> PD14
       D14  ----> PD15
       D15  ----> PG2
       BA   ----> PI11
       CKE  ----> PA3 (Version C)
       CKE  ----> PH5 (Version A)
       CLK  ----> PG1
       LDQM ----> PG15
       UNQM ----> PF11
       NWE  ----> PI7
       NCAS ----> PI8
       NRAS ----> PI9
       NCS  ----> PI10
    */
    gpioConfig.speed = GPIO_SPEED_50MHz;
    gpioConfig.mode = GPIO_MODE_AF;
    gpioConfig.otype = GPIO_OTYPE_PP;
    gpioConfig.pupd = GPIO_PUPD_NOPULL;

    gpioConfig.pin = GPIO_PIN_10 | GPIO_PIN_12 |
                     GPIO_PIN_13 | GPIO_PIN_14 |
                     GPIO_PIN_15;
    GPIO_Config(GPIOD, &gpioConfig);

    GPIO_ConfigPinAF(GPIOD, GPIO_PIN_SOURCE_10, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOD, GPIO_PIN_SOURCE_12, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOD, GPIO_PIN_SOURCE_13, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOD, GPIO_PIN_SOURCE_14, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOD, GPIO_PIN_SOURCE_15, GPIO_AF_FSMC);

    gpioConfig.pin = GPIO_PIN_0 | GPIO_PIN_1 |
                     GPIO_PIN_2 | GPIO_PIN_3 |
                     GPIO_PIN_4 | GPIO_PIN_6 |
                     GPIO_PIN_7 | GPIO_PIN_8 |
                     GPIO_PIN_9 | GPIO_PIN_10 |
                     GPIO_PIN_11;
    GPIO_Config(GPIOF, &gpioConfig);

    GPIO_ConfigPinAF(GPIOF, GPIO_PIN_SOURCE_0, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOF, GPIO_PIN_SOURCE_1, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOF, GPIO_PIN_SOURCE_2, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOF, GPIO_PIN_SOURCE_3, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOF, GPIO_PIN_SOURCE_4, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOF, GPIO_PIN_SOURCE_6, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOF, GPIO_PIN_SOURCE_7, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOF, GPIO_PIN_SOURCE_8, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOF, GPIO_PIN_SOURCE_9, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOF, GPIO_PIN_SOURCE_10, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOF, GPIO_PIN_SOURCE_11, GPIO_AF_FSMC);

    gpioConfig.pin = GPIO_PIN_1 | GPIO_PIN_2 |
                     GPIO_PIN_3 | GPIO_PIN_4 |
                     GPIO_PIN_5 | GPIO_PIN_6 |
                     GPIO_PIN_8 | GPIO_PIN_15;
    GPIO_Config(GPIOG, &gpioConfig);

    GPIO_ConfigPinAF(GPIOG, GPIO_PIN_SOURCE_1, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOG, GPIO_PIN_SOURCE_2, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOG, GPIO_PIN_SOURCE_3, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOG, GPIO_PIN_SOURCE_4, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOG, GPIO_PIN_SOURCE_5, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOG, GPIO_PIN_SOURCE_6, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOG, GPIO_PIN_SOURCE_8, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOG, GPIO_PIN_SOURCE_15, GPIO_AF_FSMC);

    gpioConfig.pin = GPIO_PIN_3 | GPIO_PIN_8 |
                     GPIO_PIN_10 | GPIO_PIN_13 |
                     GPIO_PIN_15;
    GPIO_Config(GPIOH, &gpioConfig);

    GPIO_ConfigPinAF(GPIOA, GPIO_PIN_SOURCE_3, GPIO_AF_FSMC);

    gpioConfig.pin = GPIO_PIN_3;
    GPIO_Config(GPIOA, &gpioConfig);

    GPIO_ConfigPinAF(GPIOH, GPIO_PIN_SOURCE_3, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOH, GPIO_PIN_SOURCE_5, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOH, GPIO_PIN_SOURCE_8, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOH, GPIO_PIN_SOURCE_10, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOH, GPIO_PIN_SOURCE_13, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOH, GPIO_PIN_SOURCE_15, GPIO_AF_FSMC);

    gpioConfig.pin = GPIO_PIN_3 | GPIO_PIN_7 |
                     GPIO_PIN_8 | GPIO_PIN_9 |
                     GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_Config(GPIOI, &gpioConfig);

    GPIO_ConfigPinAF(GPIOI, GPIO_PIN_SOURCE_3, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOI, GPIO_PIN_SOURCE_7, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOI, GPIO_PIN_SOURCE_8, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOI, GPIO_PIN_SOURCE_9, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOI, GPIO_PIN_SOURCE_10, GPIO_AF_FSMC);
    GPIO_ConfigPinAF(GPIOI, GPIO_PIN_SOURCE_11, GPIO_AF_FSMC);
}

void SDRAM_Init(void)
{
    uint32_t sdramCapacity;
    DMC_Config_T dmcConfig;
    DMC_TimingConfig_T timingConfig;

    RCM_EnableAHB3PeriphClock(RCM_AHB3_PERIPH_EMMC);

    timingConfig.latencyCAS = DMC_CAS_LATENCY_3; //!< Configure CAS latency period
    timingConfig.tARP = DMC_AUTO_REFRESH_10;     //!< Configure auto refresh period
    timingConfig.tRAS = DMC_RAS_MINIMUM_2;       //!< Configure line activation and precharging minimum time
    timingConfig.tCMD = DMC_ATA_CMD_1;           //!< Configure active to active period
    timingConfig.tRCD = DMC_DELAY_TIME_1;        //!< Configure RAS To CAS delay Time
    timingConfig.tRP = DMC_PRECHARGE_1;          //!< Configure precharge period
    timingConfig.tWR = DMC_NEXT_PRECHARGE_2;     //!< Configure time between the Last Data and The Next Precharge for write
    timingConfig.tXSR = 3;                       //!< Configure XSR0
    timingConfig.tRFP = 0x2F9;                   //!< Configure refresh Cycle

    dmcConfig.bankWidth = DMC_BANK_WIDTH_1;     //!< Configure bank address width
    dmcConfig.clkPhase = DMC_CLK_PHASE_REVERSE; //!< Configure clock phase
    dmcConfig.rowWidth = DMC_ROW_WIDTH_11;      //!< Configure row address width
    dmcConfig.colWidth = DMC_COL_WIDTH_8;       //!< Configure column address width
    dmcConfig.timing = timingConfig;

    DMC_Config(&dmcConfig);
    DMC_ConfigOpenBank(DMC_BANK_NUMBER_2);
    DMC_EnableAccelerateModule();

    DMC_Enable();

    /** sdramCapacity = row * col * 16bit * bank (16bit == 2Bytes) */
    sdramCapacity = (1 << (dmcConfig.rowWidth + 1)) * (1 << (dmcConfig.colWidth + 1)) * 2 * (1 << (dmcConfig.bankWidth + 1));

    printf("* Row Address Width    :%d bit\r\n", dmcConfig.rowWidth + 1);
    printf("* Column Address Width :%d bit\r\n", dmcConfig.colWidth + 1);
    printf("* Bank Address Width   :%d bit band addr\r\n", dmcConfig.bankWidth + 1);
    printf("* capacity             :%d MByte\r\n", sdramCapacity / 1024 / 1024);
}

/*!
 * @brief     Update SystemCoreClock variable according to Clock Register Values
 *            The SystemCoreClock variable contains the core clock (HCLK)
 *
 * @param     None
 *
 * @retval    None
 */
void SystemCoreClockUpdate(void)
{
    uint32_t sysClock, pllvco, pllc, pllClock, pllb;

    /* Get SYSCLK source */
    sysClock = RCM->CFG_B.SCLKSWSTS;

    switch (sysClock)
    {
    case 0:
        SystemCoreClock = HSI_VALUE;
        break;

    case 1:
        SystemCoreClock = HSE_VALUE;
        break;

    case 2:
        pllClock = RCM->PLL1CFG_B.PLL1CLKS;
        pllb = RCM->PLL1CFG_B.PLLB;

        if (pllClock == 0)
        {
            /* HSI used as PLL clock source */
            pllvco = (HSI_VALUE / pllb) * (RCM->PLL1CFG_B.PLL1A);
        }
        else
        {
            /* HSE used as PLL clock source */
            pllvco = (HSE_VALUE / pllb) * (RCM->PLL1CFG_B.PLL1A);
        }

        pllc = ((RCM->PLL1CFG_B.PLL1C) + 1) * 2;
        SystemCoreClock = pllvco / pllc;
        break;

    default:
        SystemCoreClock = HSI_VALUE;
        break;
    }

    /* Compute HCLK frequency */
    /* Get HCLK prescaler */
    sysClock = AHBPrescTable[(RCM->CFG_B.AHBPSC)];
    /* HCLK frequency */
    SystemCoreClock >>= sysClock;
}

/*!
 * @brief     Configures the System clock source, PLL Multiplier and Divider factors,
 *            AHB/APBx prescalers and Flash settings
 *
 * @param     None
 *
 * @retval    None
 */
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

        /* HCLK = SYSCLK / 1*/
        RCM->CFG_B.AHBPSC = 0x0000;

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

#if defined(DATA_IN_ExtSRAM)

/*!
 * @brief     Setup the external memory controller. Called in startup_apm32f4xx.s before jump to main.
 *            This function configures the external SRAM
 *            This SRAM will be used as program data memory (including heap and stack).
 *
 * @param     None
 *
 * @retval    None
 */
void SystemInit_ExtSRAM(void)
{
    /* Enable GPIOD, GPIOE, GPIOF and GPIOG interface clock */
    RCM->AHB1CLKEN |= 0x00000078;

    /* Connect PDx pins to FMC Alternate function */
    GPIOD->ALFL = 0x00cc00cc;
    GPIOD->ALFH = 0xcccccccc;
    /* Configure PDx pins in Alternate function mode */
    GPIOD->MODE = 0xaaaa0a0a;
    /* Configure PDx pins speed to 100 MHz */
    GPIOD->OSSEL = 0xffff0f0f;
    /* Configure PDx pins Output type to push-pull */
    GPIOD->OMODE = 0x00000000;
    /* No pull-up, pull-down for PDx pins */
    GPIOD->PUPD = 0x00000000;

    /* Connect PEx pins to FMC Alternate function */
    GPIOE->ALFL = 0xcccccccc;
    GPIOE->ALFH = 0xcccccccc;
    /* Configure PEx pins in Alternate function mode */
    GPIOE->MODE = 0xaaaaaaaa;
    /* Configure PEx pins speed to 100 MHz */
    GPIOE->OSSEL = 0xffffffff;
    /* Configure PEx pins Output type to push-pull */
    GPIOE->OMODE = 0x00000000;
    /* No pull-up, pull-down for PEx pins */
    GPIOE->PUPD = 0x00000000;

    /* Connect PFx pins to FMC Alternate function */
    GPIOF->ALFL = 0x00cccccc;
    GPIOF->ALFH = 0xcccc0000;
    /* Configure PFx pins in Alternate function mode */
    GPIOF->MODE = 0xaa000aaa;
    /* Configure PFx pins speed to 100 MHz */
    GPIOF->OSSEL = 0xff000fff;
    /* Configure PFx pins Output type to push-pull */
    GPIOF->OMODE = 0x00000000;
    /* No pull-up, pull-down for PFx pins */
    GPIOF->PUPD = 0x00000000;

    /* Connect PGx pins to FMC Alternate function */
    GPIOG->ALFL = 0x00cccccc;
    GPIOG->ALFH = 0x000000c0;
    /* Configure PGx pins in Alternate function mode */
    GPIOG->MODE = 0x00080aaa;
    /* Configure PGx pins speed to 100 MHz */
    GPIOG->OSSEL = 0x000c0fff;
    /* Configure PGx pins Output type to push-pull */
    GPIOG->OMODE = 0x00000000;
    /* No pull-up, pull-down for PGx pins */
    GPIOG->PUPD = 0x00000000;

    /* FMC Configuration */
    /* Enable the FMC/SMC interface clock */
    RCM->AHB3CLKEN |= 0x00000001;

    /* Configure and enable Bank1_SRAM2 */
    SMC_Bank1->CSCTRL2 = 0x00001011;
    SMC_Bank1->CSTIM2 = 0x00000201;
    SMC_Bank1E->WRTTIM2 = 0x0fffffff;
}
#endif /* DATA_IN_ExtSRAM */

/**@} end of group OTGD_Custom_HID_Keyboard_HS_System_Functions */
/**@} end of group OTGD_Custom_HID_Keyboard_HS */
/**@} end of group Examples */
