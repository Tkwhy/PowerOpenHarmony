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
#include "spi_if.h"

#define SPI_PORT   2

int main(void)
{
    struct SpiMsg msg;
    uint8_t writeData[6] = {0x9f, 0, 0};
    uint8_t data[6] = {0};
    uint8_t iddata[6] = {0};
    uint32_t readid;
    struct SpiDevInfo info;
    int32_t ret;

    HDF_LOGE("%s: enter spi %d", __func__);
    info.busNum = 2;
    info.csNum = 0;

    DevHandle handle = SpiOpen(&info);
    if (handle == NULL) {
        HDF_LOGE("Failed to open spi");
        return HDF_FAILURE;
    }
    msg.rbuf = data;
    msg.wbuf = writeData;
    msg.len = 6;
    //msg.csChange = 1; // switch off the CS after transfer
    msg.delayUs = 0;
    msg.speed = 0;    // use default speed
    HDF_LOGE("%s: entry spi %d", __func__,__LINE__);
    ret = SpiTransfer(handle, &msg, 1);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: spi transfer err", __func__);
        return ret;
    }

    readid = (data[2] << 16) | (data[3] << 8);
    HDF_LOGE("%s %d jedecid:0x%X", __func__,__LINE__,readid);
    HDF_LOGE("%s %d data0:0x%X data1:0x%X data2:0x%X data3:0x%X data4:0x%X", __func__,__LINE__,
             data[0],data[1],data[2],data[3],data[4],data[5]);

    writeData[0]=0xab;
    msg.rbuf = iddata;
    ret = SpiTransfer(handle, &msg, 1);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: spi transfer err", __func__);
        return ret;
    }
    HDF_LOGE("%s %d data0:0x%X data1:0x%X data2:0x%X data3:0x%X data4:0x%X", __func__,__LINE__,
             iddata[0],iddata[1],iddata[2],iddata[3],iddata[4],iddata[5]);
    readid = iddata[4];
    HDF_LOGE("%s %d jedecid:0x%X", __func__,__LINE__,readid);
    SpiClose(handle);
    return HDF_SUCCESS;
}