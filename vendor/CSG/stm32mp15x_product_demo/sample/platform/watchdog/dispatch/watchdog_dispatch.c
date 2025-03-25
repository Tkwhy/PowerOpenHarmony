/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include <string.h>
#include "hdf_log.h"
#include "osal_mem.h"
#include "watchdog_if.h"

#define WATCHDOG_TEST_TIMEOUT     2
#define WATCHDOG_TEST_FEED_TIME   6

int main(void)
{
    int32_t ret;
    DevHandle handle;
    uint32_t seconds;
    int32_t i;

    HDF_LOGE("%s: enter watchdog", __func__);

    ret = WatchdogOpen(0,&handle);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("Failed to open watchdog\r\n");
        return HDF_FAILURE;
    }
    
    WatchdogSetTimeout(handle,WATCHDOG_TEST_TIMEOUT);
    WatchdogGetTimeout(handle,&seconds);
    HDF_LOGE("seconds:%d\r\n",seconds);

    ret = WatchdogStart(handle);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: satrt fail! ret:%d", __func__, ret);
        return ret;
    }

    for (i = 0; i < WATCHDOG_TEST_FEED_TIME; i++) {
        HDF_LOGE("%s: feeding watchdog %d times... ", __func__, i);
        ret = WatchdogFeed(handle);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s: feed dog fail! ret:%d", __func__, ret);
            return ret;
        }
        OsalSleep(1);
    }

    WatchdogClose(handle);
    return HDF_SUCCESS;
}