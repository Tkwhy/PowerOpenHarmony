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
#include "i2c_if.h"

#define I2C_PORT   2
#define I2C_TEST_MSG_NUM       2

int main(void)
{
    uint8_t data[2] = {0};
    struct I2cMsg g_msgs[I2C_TEST_MSG_NUM];
    uint8_t writeData[1];
    int ret;

    HDF_LOGE("%s: entry i2c", __func__);

    writeData[0]=0x1e;
    g_msgs[0].addr = 0x33;
    g_msgs[0].flags = 0;
    g_msgs[0].len = 1;
    g_msgs[0].buf = &writeData;

    g_msgs[1].addr = 0x33;
    g_msgs[1].flags = I2C_FLAG_READ;
    g_msgs[1].len = 1;
    g_msgs[1].buf = data;

    DevHandle handle = I2cOpen(I2C_PORT);
    if (handle == NULL) {
        HDF_LOGE("Failed to open i2c %d", I2C_PORT);
        return HDF_FAILURE;
    }

    ret = I2cTransfer(handle, &g_msgs[0], 1);
    if (ret!=1) {
        HDF_LOGE("I2cTransfer write fail %d",ret);
    }
    ret = I2cTransfer(handle, &g_msgs[1], 1);
    if (ret!=1) {
        HDF_LOGE("I2cTransfer read fail %d",ret);
    }
    HDF_LOGE("1erx: %02X %02X \r\n", data[0], data[1]);
    writeData[0]=0x81;
    g_msgs[0].addr = 0x33;
    g_msgs[0].flags = 0;
    g_msgs[0].len = 1;
    g_msgs[0].buf = &writeData;

    g_msgs[1].addr = 0x33;
    g_msgs[1].flags = I2C_FLAG_READ;
    g_msgs[1].len = 1;
    g_msgs[1].buf = data;

    ret = I2cTransfer(handle, &g_msgs[0], 1);
    if (ret!=1) {
        HDF_LOGE("I2cTransfer81 write fail %d",ret);
    }
    ret = I2cTransfer(handle, &g_msgs[1], 1);
    if (ret!=1) {
        HDF_LOGE("I2cTransfer81 read fail %d",ret);
    }

    HDF_LOGE("81rx: %02X %02X \r\n", data[0], data[1]);
    I2cClose(handle);
    return HDF_SUCCESS;
}