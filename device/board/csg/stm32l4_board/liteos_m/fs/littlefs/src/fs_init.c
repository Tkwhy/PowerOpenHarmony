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

#include "stmflash.h"

struct fs_cfg {
    char *mount_point;
    struct PartitionCfg lfs_cfg;
};

static struct fs_cfg fs[LOSCFG_LFS_MAX_MOUNT_SIZE] = {0};
#ifdef LOSCFG_DRIVERS_HDF_CONFIG_MACRO
#define DISPLAY_MISC_FS_LITTLEFS_CONFIG HCS_NODE(HCS_NODE(HCS_NODE(HCS_ROOT, misc), fs_config), littlefs_config)
static uint32_t FsGetResource(struct fs_cfg *fs)
{
    int32_t num = HCS_ARRAYS_SIZE(HCS_NODE(DISPLAY_MISC_FS_LITTLEFS_CONFIG, mount_points));
    if (num < 0 || num > LOSCFG_LFS_MAX_MOUNT_SIZE) {
        HDF_LOGE("%s: invalid mount_points num %d", __func__, num);
        return HDF_FAILURE;
    }
    char * mount_points[] = HCS_ARRAYS(HCS_NODE(DISPLAY_MISC_FS_LITTLEFS_CONFIG, mount_points));
    uint32_t block_size[] = HCS_ARRAYS(HCS_NODE(DISPLAY_MISC_FS_LITTLEFS_CONFIG, blockSize));
    uint32_t block_count[] = HCS_ARRAYS(HCS_NODE(DISPLAY_MISC_FS_LITTLEFS_CONFIG, blockCount));
    uint32_t read_size[] = HCS_ARRAYS(HCS_NODE(DISPLAY_MISC_FS_LITTLEFS_CONFIG, readSize));
    uint32_t prog_size[] = HCS_ARRAYS(HCS_NODE(DISPLAY_MISC_FS_LITTLEFS_CONFIG, progSize));
    uint32_t cache_size[] = HCS_ARRAYS(HCS_NODE(DISPLAY_MISC_FS_LITTLEFS_CONFIG, cacheSize));
    uint32_t lookahead_size[] = HCS_ARRAYS(HCS_NODE(DISPLAY_MISC_FS_LITTLEFS_CONFIG, lookaheadSize));
    uint32_t block_cycles[] = HCS_ARRAYS(HCS_NODE(DISPLAY_MISC_FS_LITTLEFS_CONFIG, blockCycles));
    for (int32_t i = 0; i < num; i++) {
        fs[i].mount_point = mount_points[i];
        fs[i].lfs_cfg.blockSize = block_size[i];
        fs[i].lfs_cfg.blockCount = block_count[i];
        fs[i].lfs_cfg.readSize = read_size[i];
        fs[i].lfs_cfg.writeSize = prog_size[i];
        fs[i].lfs_cfg.cacheSize = cache_size[i];
        fs[i].lfs_cfg.lookaheadSize = lookahead_size[i];
        fs[i].lfs_cfg.blockCycles = block_cycles[i];

        HDF_LOGI("%s: fs[%d] mount_point=%s, block_size=%u, block_count=%u",
                 __func__, i, fs[i].mount_point,
                 fs[i].lfs_cfg.blockSize, fs[i].lfs_cfg.blockCount);
    }
    return HDF_SUCCESS;
}
#else
static uint32_t FsGetResource(struct fs_cfg *fs, const struct DeviceResourceNode *resourceNode)
{
    struct DeviceResourceIface *resource = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (resource == NULL) {
        HDF_LOGE("Invalid DeviceResourceIface");
        return HDF_FAILURE;
    }
    int32_t num = resource->GetElemNum(resourceNode, "mount_points");
    if (num < 0 || num > LOSCFG_LFS_MAX_MOUNT_SIZE) {
        HDF_LOGE("%s: invalid mount_points num %d", __func__, num);
        return HDF_FAILURE;
    }
    for (int32_t i = 0; i < num; i++) {
        if (resource->GetStringArrayElem(resourceNode, "mount_points",
            i, &fs[i].mount_point, NULL) != HDF_SUCCESS) {
            HDF_LOGE("%s: failed to get mount_points", __func__);
            return HDF_FAILURE;
        }
        if (resource->GetUint32ArrayElem(resourceNode, "blockSize",
            i, &fs[i].lfs_cfg.blockSize, 0) != HDF_SUCCESS) {
            HDF_LOGE("%s: failed to get block_size", __func__);
            return HDF_FAILURE;
        }
        if (resource->GetUint32ArrayElem(resourceNode, "blockCount",
            i, &fs[i].lfs_cfg.blockCount, 0) != HDF_SUCCESS) {
            HDF_LOGE("%s: failed to get block_count", __func__);
            return HDF_FAILURE;
        }
        if (resource->GetUint32ArrayElem(resourceNode, "readSize",
            i, &fs[i].lfs_cfg.readSize, 0) != HDF_SUCCESS) {
            HDF_LOGE("%s: failed to get readSize", __func__);
            return HDF_FAILURE;
        }
        if (resource->GetUint32ArrayElem(resourceNode, "progSize",
            i, &fs[i].lfs_cfg.progSize, 0) != HDF_SUCCESS) {
            HDF_LOGE("%s: failed to get progSize", __func__);
            return HDF_FAILURE;
        }
        if (resource->GetUint32ArrayElem(resourceNode, "cacheSize",
            i, &fs[i].lfs_cfg.cacheSize, 0) != HDF_SUCCESS) {
            HDF_LOGE("%s: failed to get cacheSize", __func__);
            return HDF_FAILURE;
        }
        if (resource->GetUint32ArrayElem(resourceNode, "lookaheadSize",
            i, &fs[i].lfs_cfg.lookaheadSize, 0) != HDF_SUCCESS) {
            HDF_LOGE("%s: failed to get lookaheadSize", __func__);
            return HDF_FAILURE;
        }
        if (resource->GetUint32ArrayElem(resourceNode, "blockCycles",
            i, &fs[i].lfs_cfg.blockCycles, 0) != HDF_SUCCESS) {
            HDF_LOGE("%s: failed to get blockCycles", __func__);
            return HDF_FAILURE;
        }
        HDF_LOGI("%s: fs[%d] mount_point=%s, block_size=%u, block_count=%u",
                 __func__, i,fs[i].mount_point,
                 fs[i].lfs_cfg.blockSize, fs[i].lfs_cfg.blockCount);
    }
    return HDF_SUCCESS;
}
#endif

#if (LOSCFG_LITTLEFS_FLASH == 1) //?¨²2?FLASH
HalLogicPartition g_halPartitions[] = {
    [FLASH_PARTITION_DATA0] = {
        .partitionDescription = "littlefs",
        .partitionStartAddr = 0x8000000 + 0x5a000, //360K start
        .partitionLength = 0x26000, //152K
    },
    [FLASH_PARTITION_DATA1] = {
        .partitionDescription = "vfat",
        .partitionStartAddr = 0x8000000 + 0x5a000, //360K start
        .partitionLength = 0x26000,//152K
    },
};

HalLogicPartition *getPartitionInfo(VOID)
{
    return g_halPartitions;
}

static int32_t FsDriverCheck(struct HdfDeviceObject *object)
{
    HDF_LOGI("Fs Driver Init\n");
    if (object == NULL) {
        return HDF_FAILURE;
    }
#ifdef LOSCFG_DRIVERS_HDF_CONFIG_MACRO
    if (FsGetResource(fs) != HDF_SUCCESS) {
        HDF_LOGE("%s: FsGetResource failed", __func__);
        return HDF_FAILURE;
    }
#else
    if (object->property) {
        if (FsGetResource(fs, object->property) != HDF_SUCCESS) {
            HDF_LOGE("%s: FsGetResource failed", __func__);
            return HDF_FAILURE;
        }
    }
#endif

    return HDF_SUCCESS;
}

static int32_t FsDriverInit(struct HdfDeviceObject *object)
{
    int ret;
    HalLogicPartition *halPartitionsInfo = getPartitionInfo();
    int32_t lengthArray[2];
    lengthArray[0]= halPartitionsInfo[FLASH_PARTITION_DATA0].partitionLength;

    int32_t addrArray[2];
    addrArray[0] = halPartitionsInfo[FLASH_PARTITION_DATA0].partitionStartAddr;

    ret = LOS_DiskPartition("flash0", "littlefs", lengthArray, addrArray, 2);
    printf("%s: DiskPartition %s\n", __func__, (ret == 0) ? "succeed" : "failed");
    if (ret != 0) {
        return -1;
    }
    
    if (HDF_SUCCESS != FsDriverCheck(object))
        return HDF_FAILURE;

    DIR *dir = NULL;
    for (int i = 0; i < sizeof(fs) / sizeof(fs[0]); i++) {
        if (fs[i].mount_point == NULL)
            continue;
        fs[i].lfs_cfg.readFunc = LittlefsRead;
        fs[i].lfs_cfg.writeFunc = LittlefsProg;
        fs[i].lfs_cfg.eraseFunc = LittlefsErase;

        int ret = LOS_PartitionFormat("flash0", "littlefs", &fs[i].lfs_cfg);
        printf("%s: PartitionFormat %s\n", __func__, (ret == 0) ? "succeed" : "failed");
        if (ret != 0) {
            return -1;
        }
        ret = mount(NULL, fs[i].mount_point, "littlefs", 0, &fs[i].lfs_cfg);
        HDF_LOGI("%s: mount fs on '%s' %s\n", __func__, fs[i].mount_point, (ret == 0) ? "succeed" : "failed");
        if ((dir = opendir(fs[i].mount_point)) == NULL) {
            HDF_LOGI("first time create file %s\n", fs[i].mount_point);
            ret = mkdir(fs[i].mount_point, S_IRUSR | S_IWUSR);
            if (ret != LOS_OK) {
                HDF_LOGE("Mkdir failed %d\n", ret);
                return HDF_FAILURE;
            } else {
                HDF_LOGI("mkdir success %d\n", ret);
            }
        } else {
            HDF_LOGI("open dir success!\n");
            closedir(dir);
        }
    }
    return HDF_SUCCESS;
}

static void FsDriverRelease(struct HdfDeviceObject *device)
{
    (void)device;
}

#endif

#if (LOSCFG_LITTLEFS_W25QXX == 1) //use w25qxx
static int32_t FsDriverCheck(struct HdfDeviceObject *object)
{
    HDF_LOGI("Fs Driver Init\n");
    if (object == NULL) {
        return HDF_FAILURE;
    }
#ifdef LOSCFG_DRIVERS_HDF_CONFIG_MACRO
    if (FsGetResource(fs) != HDF_SUCCESS) {
        HDF_LOGE("%s: FsGetResource failed", __func__);
        return HDF_FAILURE;
    }
#else
    if (object->property) {
        if (FsGetResource(fs, object->property) != HDF_SUCCESS) {
            HDF_LOGE("%s: FsGetResource failed", __func__);
            return HDF_FAILURE;
        }
    }
#endif

    if (W25Qxx_InitSpiFlash(0, 0) != 0) {
        HDF_LOGI("InitSpiFlash failed\n");
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t FsDriverInit(struct HdfDeviceObject *object)
{
    int ret;
    INT32 lengthArray = 0x00800000;
    INT32 addrArray = 4096;
    ret = LOS_DiskPartition("flash0", "littlefs", &lengthArray, &addrArray, 1);
    printf("%s: DiskPartition %s\n", __func__, (ret == 0) ? "succeed" : "failed");
    if (ret != 0) {
        return -1;
    }
    
    if (HDF_SUCCESS != FsDriverCheck(object))
        return HDF_FAILURE;

    DIR *dir = NULL;
    for (int i = 0; i < sizeof(fs) / sizeof(fs[0]); i++) {
        if (fs[i].mount_point == NULL)
            continue;
        fs[i].lfs_cfg.readFunc = LittlefsRead;
        fs[i].lfs_cfg.writeFunc = LittlefsProg;
        fs[i].lfs_cfg.eraseFunc = LittlefsErase;

        int ret = LOS_PartitionFormat("flash0", "littlefs", &fs[i].lfs_cfg);
        printf("%s: PartitionFormat %s\n", __func__, (ret == 0) ? "succeed" : "failed");
        if (ret != 0) {
            return -1;
        }
        ret = mount(NULL, fs[i].mount_point, "littlefs", 0, &fs[i].lfs_cfg);
        HDF_LOGI("%s: mount fs on '%s' %s\n", __func__, fs[i].mount_point, (ret == 0) ? "succeed" : "failed");
        if ((dir = opendir(fs[i].mount_point)) == NULL) {
            HDF_LOGI("first time create file %s\n", fs[i].mount_point);
            ret = mkdir(fs[i].mount_point, S_IRUSR | S_IWUSR);
            if (ret != LOS_OK) {
                HDF_LOGE("Mkdir failed %d\n", ret);
                return HDF_FAILURE;
            } else {
                HDF_LOGI("mkdir success %d\n", ret);
            }
        } else {
            HDF_LOGI("open dir success!\n");
            closedir(dir);
        }
    }
    return HDF_SUCCESS;
}

static void FsDriverRelease(struct HdfDeviceObject *device)
{
    (void)device;
    W25Qxx_DeInitSpiFlash();
}
#endif

static int32_t FsDriverBind(struct HdfDeviceObject *device)
{
    (void)device;
    return HDF_SUCCESS;
}

static struct HdfDriverEntry g_fsDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_FS_LITTLEFS",
    .Bind = FsDriverBind,
    .Init = FsDriverInit,
    .Release = FsDriverRelease,
};

HDF_INIT(g_fsDriverEntry);

