/*
 * Copyright (c) 2022 Hihope Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if defined(USE_FULL_LL_DRIVER)

#include "hal_exti.h"
#include "stm32f4xx_ll_system.h"

#if defined (EXTI)

Pin_ST g_pinsGroup[STM32_GPIO_GROUP_MAX*STM32_GPIO_PIN_MAX] = {0};
static uint16_t g_exitSetupCounts = 0;

static const uint32_t g_sysCfgExitLineMap[] = {
    LL_SYSCFG_EXTI_LINE0,
    LL_SYSCFG_EXTI_LINE1,
    LL_SYSCFG_EXTI_LINE2,
    LL_SYSCFG_EXTI_LINE3,
    LL_SYSCFG_EXTI_LINE4,
    LL_SYSCFG_EXTI_LINE5,
    LL_SYSCFG_EXTI_LINE6,
    LL_SYSCFG_EXTI_LINE7,
    LL_SYSCFG_EXTI_LINE8,
    LL_SYSCFG_EXTI_LINE9,
    LL_SYSCFG_EXTI_LINE10,
    LL_SYSCFG_EXTI_LINE11,
    LL_SYSCFG_EXTI_LINE12,
    LL_SYSCFG_EXTI_LINE13,
    LL_SYSCFG_EXTI_LINE14,
    LL_SYSCFG_EXTI_LINE15
};

#define PIN_EXIT_ZERO 0
#define PIN_EXIT_FIVE 5
#define PIN_EXIT_TEN  10
#define PIN_EXIT_SIXTEEN  16

static void LL_Gpio_Exti_Handler(void)
{
    ITStatus status = RESET;
    for (uint16_t index = 0; index < g_exitSetupCounts; index++) {
        if (g_pinsGroup[index].setup) {
            if (g_pinsGroup[index].trigger == LL_EXTI_TRIGGER_FALLING) {
                status = RESET;
            } else {
                status = SET;
            }
            if (LL_EXTI_IsActiveFlag_0_31(g_pinsGroup[index].exitLine) != status) {
                LL_EXTI_ClearFlag_0_31(g_pinsGroup[index].exitLine);
                if (LL_GPIO_IsInputPinSet(g_pinsGroup[index].gpiox, g_pinsGroup[index].pinReg) == status) {
                    g_pinsGroup[index].handler(g_pinsGroup[index].localPin);
                    break;
                }
            }
        }
    }
}

uint32_t LL_SETUP_EXTI(LL_EXTI_InitConfig* cfg, uint16_t pin, uint16_t local, uint8_t group)
{
    ErrorStatus status = SUCCESS;
    uint32_t ret = SUCCESS;
    if (cfg == NULL) {
        status = ERROR;
        return status;
    }

    LL_APB2_GRP1_EnableClock(RCC_APB2ENR_SYSCFGEN);
    LL_SYSCFG_SetEXTISource(group, g_sysCfgExitLineMap[pin]);

    status = LL_EXTI_Init(&cfg->initType);
    if (status != SUCCESS) {
        return status;
    }

    if (cfg->initType.LineCommand) {
        g_pinsGroup[g_exitSetupCounts].setup = SET;
    } else {
        g_pinsGroup[g_exitSetupCounts].setup= RESET;
    }
    g_pinsGroup[g_exitSetupCounts].pin = pin;
    g_pinsGroup[g_exitSetupCounts].localPin = local;
    g_pinsGroup[g_exitSetupCounts].exitLine = cfg->initType.Line_0_31;
    g_pinsGroup[g_exitSetupCounts].group = group;
    g_pinsGroup[g_exitSetupCounts].handler = cfg->Exithandler;
    g_pinsGroup[g_exitSetupCounts].trigger = cfg->initType.Trigger;
    g_pinsGroup[g_exitSetupCounts].pinReg = cfg->PinReg;
    g_pinsGroup[g_exitSetupCounts].gpiox = cfg->Gpiox;
    if (pin < PIN_EXIT_FIVE) {
        NVIC_SetVector(EXTI0_IRQn + pin, (uint32_t)LL_Gpio_Exti_Handler);
        NVIC_SetPriority(EXTI0_IRQn + pin, 0);
        NVIC_EnableIRQ(EXTI0_IRQn + pin);
    } else if (pin >= PIN_EXIT_FIVE && pin < PIN_EXIT_TEN) {
        NVIC_SetVector(EXTI9_5_IRQn, (uint32_t)LL_Gpio_Exti_Handler);
        NVIC_SetPriority(EXTI9_5_IRQn, 0);
        NVIC_EnableIRQ(EXTI9_5_IRQn);
    } else if (pin >= PIN_EXIT_TEN && pin < PIN_EXIT_SIXTEEN) {
        NVIC_SetVector(EXTI15_10_IRQn, (uint32_t)LL_Gpio_Exti_Handler);
        NVIC_SetPriority(EXTI15_10_IRQn, 0);
        NVIC_EnableIRQ(EXTI15_10_IRQn);
    }
    g_exitSetupCounts++;

    if (ret != 0) {
        status = ret;
    }

    return status;
}

#endif /* defined (EXTI) */

#endif /* USE_FULL_LL_DRIVER */
