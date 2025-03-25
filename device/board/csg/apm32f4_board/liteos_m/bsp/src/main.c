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
#include "main.h"
#include "los_config.h"
#include "Board_APM32F407_EVAL.h"
#include "ohos_init.h"
#include "ohos_types.h"
#include "los_task.h"
// #include "fs_init_sdk.h"

#define DEBUG_USART USART1

__attribute__((weak)) void ohos_app_main(void)
{
    printf("No application run, Maybe you should config your application in BUILD.gn!\n");
    return;
}

int main(void)
{
    int ret;

    USART_Config_T usartConfigStruct;

    usartConfigStruct.baudRate = 115200;
    usartConfigStruct.hardwareFlow = USART_HARDWARE_FLOW_NONE;
    usartConfigStruct.mode = USART_MODE_TX;
    usartConfigStruct.parity = USART_PARITY_NONE;
    usartConfigStruct.stopBits = USART_STOP_BIT_1;
    usartConfigStruct.wordLength = USART_WORD_LEN_8B;
    APM_BOARD_COMInit(COM1, &usartConfigStruct);

    APM_BOARD_LEDInit(LED2);
    APM_BOARD_LEDInit(LED3);
    APM_BOARD_LEDOff(LED2);
    APM_BOARD_LEDOff(LED3);

    APM_BOARD_LEDOn(LED2);
    APM_BOARD_LEDOn(LED3);
    ret = LOS_KernelInit();
    if (ret == LOS_OK)
    {
        OHOS_SystemInit();
        LOS_Start();
    }

    while (1)
    {
        printf("Error\r\n");
    }
}

int fputc(int ch, FILE *f)
{
    /** send a byte of data to the serial port */
    USART_TxData(DEBUG_USART, (uint8_t)ch);

    /** wait for the data to be send  */
    while (USART_ReadStatusFlag(DEBUG_USART, USART_FLAG_TXBE) == RESET)
        ;

    return (ch);
}

#ifdef USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}
#endif