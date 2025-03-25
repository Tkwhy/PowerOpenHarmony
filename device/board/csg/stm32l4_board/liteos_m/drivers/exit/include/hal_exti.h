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

#ifndef _HAL_EXTI_H_
#define _HAL_EXTI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx_ll_exti.h"
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_gpio.h"

#if defined(USE_FULL_LL_DRIVER)

#if defined (EXTI)

typedef enum {
    STM32_GPIO_PIN_0 = 0, /* Pin 0 selected    */
    STM32_GPIO_PIN_1,     /* Pin 1 selected    */
    STM32_GPIO_PIN_2,     /* Pin 2 selected    */
    STM32_GPIO_PIN_3,     /* Pin 3 selected    */
    STM32_GPIO_PIN_4,     /* Pin 4 selected    */
    STM32_GPIO_PIN_5,     /* Pin 5 selected    */
    STM32_GPIO_PIN_6,     /* Pin 6 selected    */
    STM32_GPIO_PIN_7,     /* Pin 7 selected    */
    STM32_GPIO_PIN_8,     /* Pin 8 selected    */
    STM32_GPIO_PIN_9,     /* Pin 9 selected    */
    STM32_GPIO_PIN_10,    /* Pin 10 selected   */
    STM32_GPIO_PIN_11,    /* Pin 11 selected   */
    STM32_GPIO_PIN_12,    /* Pin 12 selected   */
    STM32_GPIO_PIN_13,    /* Pin 13 selected   */
    STM32_GPIO_PIN_14,    /* Pin 14 selected   */
    STM32_GPIO_PIN_15,    /* Pin 15 selected   */
    STM32_GPIO_PIN_MAX,   /* Pin Max */
} STM32_GPIO_PIN;

typedef enum {
    STM32_GPIO_GROUP_A = 0,
    STM32_GPIO_GROUP_B,
    STM32_GPIO_GROUP_C,
    STM32_GPIO_GROUP_D,
    STM32_GPIO_GROUP_E,
    STM32_GPIO_GROUP_F,
    STM32_GPIO_GROUP_G,
    STM32_GPIO_GROUP_H,
    STM32_GPIO_GROUP_I,
    STM32_GPIO_GROUP_MAX,
}STM32_GPIO_GROUP;

typedef void (*HAL_GPIO_PIN_EXIT_HANDLER)(uint16_t pin);

typedef struct {
    LL_EXTI_InitTypeDef initType;
    HAL_GPIO_PIN_EXIT_HANDLER Exithandler;
    uint32_t PinReg;
    GPIO_TypeDef* Gpiox;
} LL_EXTI_InitConfig;

typedef struct {
    uint16_t pin;
    uint16_t localPin;
    uint32_t exitLine;
    uint8_t setup;
    uint8_t group;
    uint8_t trigger;
    GPIO_TypeDef* gpiox;
    uint32_t pinReg;
    HAL_GPIO_PIN_EXIT_HANDLER handler;
} Pin_ST;

typedef struct {
    uint32_t trigger;
} HAL_GPIO_EXIT_CFG_T;

uint32_t LL_SETUP_EXTI(LL_EXTI_InitConfig* cfg, uint16_t pin, uint16_t local, uint8_t group);

#endif /* EXTI */

#endif /* USE_FULL_LL_DRIVER */

#ifdef __cplusplus
}
#endif

#endif