/**
  **************************************************************************
  * @file     main.c
  * @version  v2.0.7
  * @date     2022-02-11
  * @brief    main program
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to 
  * download from Artery official website is the copyrighted work of Artery. 
  * Artery authorizes customers to use, copy, and distribute the BSP 
  * software and its related documentation for the purpose of design and 
  * development in conjunction with Artery microcontrollers. Use of the 
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

#include <stdio.h>
#include "at32f403a_407.h"
#include "los_task.h"
#include "devmgr_service_start.h"
#include "los_memory.h"

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
    int ret;
    system_clock_config();
    at32_board_init();
    uart_print_init(115200);
    printf("User main!\n");

    ret = LOS_KernelInit();
    //if (DeviceManagerStart()) {
    //    printf("[%s] No drivers need load by hdf manager!", __func__);
    //}
    OHOS_SystemInit();
    if (ret == LOS_OK) {
        LOS_Start();
    }
    while(1) {}
}
