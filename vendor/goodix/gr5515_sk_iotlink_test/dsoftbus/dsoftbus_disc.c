/*
 * Copyright (c) 2021 GOODIX.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

#include "softbus_server_frame.h"
#include "softbus_common.h"
#include "session.h"
#include "softbus_errcode.h"
#include "discovery_service.h"

extern void InitSoftBusServer(void);

static bool g_successFlag = false;

static int32_t g_sessionId = -1;
static int32_t g_openAuthSession = 0;
static int32_t g_BytesReceived = -1;

static int OnSessionOpened(int sessionId, int result)
{
    if (result == SOFTBUS_OK) {
        if (g_sessionId == -1 || sessionId == g_sessionId) {
            if (g_sessionId == -1) {
                g_sessionId = sessionId;
            }
            g_successFlag = true;
        }
    }
    printf("############# session opened,sesison id[%d] result[%d], g_sessionId[%d]\n", sessionId, result, g_sessionId);
    return result;
}

static void OnSessionClosed(int sessionId)
{
    printf("############# session closed, session id = %d\n", sessionId);
    g_sessionId = -1;
    g_successFlag = false;
}

static void OnBytesReceived(int sessionId, const void *data, unsigned int len)
{
    (void)data;
    if (g_sessionId == -1 || sessionId == g_sessionId) {
        printf("#############client bytes received, data[%s], dataLen[%u]\n", data, len);
    } else {
        printf("#############server bytes received, sessionid[%d], data[%s], dataLen[%u]\n", sessionId, data, len);
    }
    g_BytesReceived = 0;
}

static void OnMessageReceived(int sessionId, const void *data, unsigned int len)
{
    (void)data;
    if (g_sessionId == -1 || sessionId == g_sessionId) {
        printf("#############client msg received, data[%s], dataLen[%u]\n", data, len);
    } else {
        printf("#############server msg received, sessionid[%d], data[%s], dataLen[%u]\n", sessionId, data, len);
    }
}

static ISessionListener g_serverSessionlistener;
static SessionAttribute g_sessionServerAttr;
static void TestSessionListenerInit(void)
{
    g_serverSessionlistener.OnSessionOpened = OnSessionOpened;
    g_serverSessionlistener.OnSessionClosed = OnSessionClosed;
    g_serverSessionlistener.OnBytesReceived = OnBytesReceived;
    g_serverSessionlistener.OnMessageReceived = OnMessageReceived;

    g_sessionServerAttr.dataType = TYPE_BYTES;
}

static const char *g_testServerModuleName = "dsoftbus.module.server";
static const char *g_testServerSessionName   = "dsoftbus.session.server";
static const char *g_testClientSessionName   = "dsoftbus.session.client";
static int32_t ServerCreateSessionServer()
{
    printf("\r\n");
    printf("******%s: CreateSessionServer!***\n", __func__);
    int32_t ret = CreateSessionServer(g_testServerModuleName, g_testServerSessionName, &g_serverSessionlistener);
    printf("%s: %d\n", __func__, __LINE__);
    if (ret != SOFTBUS_SERVER_NAME_REPEATED && ret != SOFTBUS_OK) { // -986: SOFTBUS_SERVER_NAME_REPEATED
        printf("CreateSessionServer ret: %d \n", ret);
        return SOFTBUS_ERR;
    }
    printf("%s: %d\n", __func__, __LINE__);
    return SOFTBUS_OK;
}

static int GetSubscribeId(void)
{
    static int32_t subscribeId = 0;
    subscribeId++;
    return subscribeId;
}

static const char *g_testGroupId = "g_testGroupId";
static void OnDeviceFound(const DeviceInfo *device)
{
    printf("***%s:%s %d***\n", __FILE__, __func__, __LINE__);

    if (device == NULL) {
        printf("ondevice found device is null\n");
        return;
    }
    printf("***********OnDeviceFound!!!!!******************************************\n");
    printf("id : %s.\n", device->devId);
    printf("name : %s.\n", device->devName);
    printf("device type : %u.\n", device->devType);
    printf("capNum : %u.\n", device->capabilityBitmapNum);
    for (uint32_t i = 0; i < device->capabilityBitmapNum; i++) {
        printf("capBitmap[%u] : %u.\n", i, device->capabilityBitmap[i]);
    }
    printf("addr num : %u.\n", device->addrNum);
    printf("connect type : %d.\n", device->addr[0].type);
    printf("ip : %s.\n", device->addr[0].info.ble.bleMac);
    //printf("ip : %s.\n", device->addr[0].info.ip.ip);
    //printf("port : %d.\n", device->addr[0].info.ip.port);
    //printf("peerUid : %s.\n", device->addr[0].peerUid);
    printf("**********************************************************************\n");
    return;
}

static void TestDeviceFound(const DeviceInfo *device)
{
    printf("[server]TestDeviceFound\n");
    OnDeviceFound(device);
}

static void TestDiscoverFailed(int subscribeId, DiscoveryFailReason failReason)
{
    printf("[server]TestDiscoverFailed, subscribeId = %d, failReason = %d\n", subscribeId, failReason);
}

static void TestDiscoverySuccess(int subscribeId)
{
    printf("[server]TestDiscoverySuccess, subscribeId = %d\n", subscribeId);
}
//发现回调
static IDiscoveryCallback g_subscribeCb = {
    .OnDeviceFound = TestDeviceFound,
    .OnDiscoverFailed = TestDiscoverFailed,
    .OnDiscoverySuccess = TestDiscoverySuccess
};

static SubscribeInfo g_sInfo1 = {
    .subscribeId = 1,
    .medium = BLE,
    .mode = DISCOVER_MODE_ACTIVE,
    .freq = MID,
    .capability = "osdCapability",
    .capabilityData = (unsigned char *)"capdata4",
    .dataLen = sizeof("capdata4"),
    .isSameAccount = true,
    .isWakeRemote = false
};

#define SEND_DATA_SIZE_256 256
static const char *g_testData = "1234";

static int32_t TestSendBytesData()
{
    char *testData = (char *)SoftBusCalloc(SEND_DATA_SIZE_256);
    if (memcpy_s(testData, SEND_DATA_SIZE_256, g_testData, strlen(g_testData)) != 0) {
        printf("memcpy_s g_testData failed!\n");
        return;
    }

    printf("******SendBytes start\n");
    int32_t ret = SendBytes(g_sessionId, testData, strlen(testData));
    if (ret != SOFTBUS_OK) {
        printf("SendBytes failed ret[%d] len[%u]\n", ret, strlen(testData));
    }
    printf("******SendBytes end\n");

    SoftBusFree(testData);
    return ret;
}

#define DISC_TEST_PKG_NAME "DISC_TEST"
void dsoftbus_Task()
{
    printf("%s:  Dsoftbus StartDiscovery !!!\n", __func__);
    osDelay(1000);

    InitSoftBusServer();

    TestSessionListenerInit();
    ServerCreateSessionServer();
    printf("%s: %d\n", __func__, __LINE__);
    g_sInfo1.subscribeId = GetSubscribeId();
    printf("******%s: StartDiscovery subscribeId %d!***\n", __func__, g_sInfo1.subscribeId);
    StartDiscovery(DISC_TEST_PKG_NAME, &g_sInfo1, &g_subscribeCb);

    g_sessionId = OpenSession(g_testServerSessionName, g_testClientSessionName, NULL, g_testGroupId, &g_sessionServerAttr);
    if (g_sessionId < 0) {
        printf("OpenSession ret[%d]", g_sessionId);
        return SOFTBUS_ERR;
    }
    printf("OpenSession ok,g_sessionId[%d]", g_sessionId);

    while (1) {
        osDelay(1000);
    }
}

void Dsoftbus_TaskEntry(void)
{
    osThreadAttr_t attr;

    attr.name       = "dsoftbus_task";
    attr.attr_bits  = 0U;
    attr.cb_mem     = NULL;
    attr.cb_size    = 0U;
    attr.stack_mem  = NULL;
    attr.stack_size = 1024 * 16;
    attr.priority   = 20;

    if (osThreadNew((osThreadFunc_t)dsoftbus_Task, NULL, &attr) == NULL) {
        printf("[HelloDemo] Failed to create HelloTask!\n");
    }
}

SYS_RUN(Dsoftbus_TaskEntry);