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
static int32_t g_BytesReceived = -1;
int32_t g_openSessionFlag = -1;

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
    g_openSessionFlag = 1;
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

static ISessionListener g_clientSessionlistener;
static SessionAttribute g_sessionClientAttr;
static void TestSessionListenerInit(void)
{
    g_clientSessionlistener.OnSessionOpened = OnSessionOpened;
    g_clientSessionlistener.OnSessionClosed = OnSessionClosed;
    g_clientSessionlistener.OnBytesReceived = OnBytesReceived;
    g_clientSessionlistener.OnMessageReceived = OnMessageReceived;

    g_sessionClientAttr.dataType = TYPE_BYTES;
}

static const char *g_testClientModuleName = "dsoftbus.module.client";
static const char *g_testClientSessionName   = "dsoftbus.session.client";
static int32_t ServerCreateSessionServer()
{
    printf("\r\n");
    printf("******%s: CreateSessionServer!***\n", __func__);
    int32_t ret = CreateSessionServer(g_testClientModuleName, g_testClientSessionName, &g_clientSessionlistener);
    if (ret != SOFTBUS_SERVER_NAME_REPEATED && ret != SOFTBUS_OK) { // -986: SOFTBUS_SERVER_NAME_REPEATED
        printf("CreateSessionServer ret: %d \n", ret);
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

static void TestPublishSuccess(int publishId)
{
    printf("[client]TestPublishSuccess, publishId = %d\n", publishId);
    printf("\r\n");
}

static void TestPublishFail(int publishId, PublishFailReason reason)
{
    printf("[client]TestPublishFail, publishId = %d, PublishFailReason = %d\n", publishId, reason);
    printf("\r\n");
}

static PublishInfo g_pInfo1 = {
    .publishId = 1,
    .medium = BLE,
    .mode = DISCOVER_MODE_ACTIVE,
    .freq = MID,
    .capability = "osdCapability",
    .capabilityData = (unsigned char *)"capdata4",
    .dataLen = sizeof("capdata4")
};

static IPublishCallback g_publishCb = {
    .OnPublishSuccess = TestPublishSuccess,
    .OnPublishFail = TestPublishFail
};

static int GetPublishId(void)
{
    static int32_t publishId = 0;
    publishId++;
    return publishId;
}

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

#define PUB_TEST_PKG_NAME "PUB_TEST"

void dsoftbus_Task()
{
    printf("%s:  Dsoftbus Publish !!!\n", __func__);

    InitSoftBusServer();

    TestSessionListenerInit();
    ServerCreateSessionServer();

    g_pInfo1.publishId = GetPublishId();
    printf("******%s: PublishService publishId %d!***\n", __func__, g_pInfo1.publishId);
    PublishService(PUB_TEST_PKG_NAME, &g_pInfo1, &g_publishCb);

    while (1) {
        TestSendBytesData();
        osDelay(5000);
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

//SYS_RUN(Dsoftbus_TaskEntry);