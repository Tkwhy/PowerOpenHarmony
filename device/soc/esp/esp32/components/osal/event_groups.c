/*
 * Copyright (c) 2022 Hunan OpenValley Digital Industry Development Co., Ltd.
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

#include <stdlib.h>
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "esp_osal.h"
#include "task.h"
#include "timers.h"
#include "event_groups.h"

#define EVENT_LOG() esp_rom_printf("\n%s\n",__func__);while(1);
EventBits_t IRAM_ATTR xEventGroupWaitBits( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToWaitFor, const BaseType_t xClearOnExit, const BaseType_t xWaitForAllBits, TickType_t xTicksToWait )
{
    EVENT_LOG();
    return 0;
}
EventGroupHandle_t IRAM_ATTR xEventGroupCreate( void )
{
    EVENT_LOG();
    return 0;
}
EventBits_t IRAM_ATTR xEventGroupClearBits( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToClear )
{
    EVENT_LOG();
    return 0;
}
EventBits_t IRAM_ATTR xEventGroupSetBits( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet )
{
    EVENT_LOG();
    return 0;
}
void IRAM_ATTR vEventGroupDelete( EventGroupHandle_t xEventGroup )
{
    EVENT_LOG();
}

