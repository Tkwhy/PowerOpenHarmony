/*
 * FreeRTOS Kernel V10.2.1
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* Standard includes. */
#include <stdlib.h>
#include <string.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE


/* esp_osal includes. */
#include "esp_osal.h"
#include "task.h"
#include "timers.h"
#include "stack_macros.h"
#include "los_task.h"
#include "los_sched.h"
#include "xtensa_rtos.h"
#include "esp_task_wdt.h"

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE
/*-----------------------------------------------------------*/
#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )

BaseType_t xTaskCreatePinnedToCore(	TaskFunction_t pvTaskCode,
									const char * const pcName,
									const uint32_t usStackDepth,
									void * const pvParameters,
									UBaseType_t uxPriority,
									TaskHandle_t * const pvCreatedTask,
									const BaseType_t xCoreID)
{
	UINT32 taskID1 = 0, ret;
	TSK_INIT_PARAM_S attr = { 0 };
	static char defName[] = {"null"};
	attr.pfnTaskEntry = (TSK_ENTRY_FUNC)(int)pvTaskCode;
	attr.uwStackSize  = usStackDepth + 256;
	attr.pcName       = pcName ? pcName : defName;
	attr.usTaskPrio   = configMAX_PRIORITIES - 1 - uxPriority;// ((uxPriority * 31) / configMAX_PRIORITIES);
	attr.uwArg        = (UINT32)pvParameters;
	if(attr.uwStackSize < 4096)attr.uwStackSize = 4096;
	ret = LOS_TaskCreate(&taskID1, &attr);
	if(pvCreatedTask) *pvCreatedTask = (TaskHandle_t)taskID1;
	// esp_rom_printf("\n\e[1;36mxTaskCreate.%s stack=%d Priority=%d id=%d\e[0m\n",pcName,usStackDepth,uxPriority,taskID1);
	return (LOS_OK == ret) ? pdPASS : errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
}

#endif /* configSUPPORT_DYNAMIC_ALLOCATION */
/*----------------------------------------------------------*/
struct _reent* __getreent(void) {return _GLOBAL_REENT;}
/*----------------------------------------------------------*/
extern unsigned port_interruptNesting[];
#if 0
static portMUX_TYPE Kernel_spinlock = portMUX_INITIALIZER_UNLOCKED;
UINT32 ArchIntLock(VOID)
{
    if(port_interruptNesting[0]) {
		taskENTER_CRITICAL_ISR(&Kernel_spinlock);
    }else {
    	taskENTER_CRITICAL(&Kernel_spinlock);
    }
	return 0;
}
VOID ArchIntRestore(UINT32 intSave)
{
    if(port_interruptNesting[0]) {
		taskEXIT_CRITICAL_ISR(&Kernel_spinlock);
    }else {
    	taskEXIT_CRITICAL(&Kernel_spinlock);
    }
}
#else
//UINT32 ArchIntLock(VOID) {return portENTER_CRITICAL_NESTED();}
//VOID ArchIntRestore(UINT32 intSave) {portEXIT_CRITICAL_NESTED(intSave);}
UINT32 ArchIntLock(VOID)
{
	UINT32 tmp;
    __asm__ __volatile__("rsil	%0, 3\n": "=a"(tmp): :"memory");
	return tmp;
}
VOID ArchIntRestore(UINT32 intSave)
{
    __asm__ __volatile__("wsr    %0, ps\n": :"a"(intSave):"memory");
}
#endif

BaseType_t xTaskGetAffinity( TaskHandle_t xTask )
{
   return 0;
}

char *pcTaskGetName( TaskHandle_t xTaskToQuery )
{
	return LOS_TaskNameGet((UINT32)xTaskToQuery);
}

TaskHandle_t xTaskGetIdleTaskHandleForCPU( UBaseType_t cpuid )
{
	return (TaskHandle_t)g_idleTaskID;
}

UINT32 ArchIsIntActive(VOID) 
{
    return port_interruptNesting[0];
}
/*----------------------------------------------------------*/
// xMPU_SETTINGS
#if ( portUSING_MPU_WRAPPERS == 1 )
_Static_assert(sizeof(xMPU_SETTINGS) >= sizeof(((LosTaskCB *)0)->MPUSettings),"sizeof(xMPU_SETTINGS) < sizeof(LosTaskCB.MPUSettings)");
_Static_assert(sizeof(xMPU_SETTINGS) <= sizeof(((LosTaskCB *)0)->MPUSettings),"sizeof(xMPU_SETTINGS) > sizeof(LosTaskCB.MPUSettings)");

_Static_assert(CP_TOPOFSTACK_OFFS >= (int)&((LosTaskCB *)0)->MPUSettings[0],"CP_TOPOFSTACK_OFFS < offset(LosTaskCB.MPUSettings)");
_Static_assert(CP_TOPOFSTACK_OFFS <= (int)&((LosTaskCB *)0)->MPUSettings[0],"CP_TOPOFSTACK_OFFS > offset(LosTaskCB.MPUSettings)");
#endif

void vPortStoreTaskSettings(StackType_t *pxBottomOfStack, uint32_t usStackDepth, UINT32 taskID)
{
	// static_assert
	LosTaskCB *pxNewTCB = OS_TCB_FROM_TID(taskID);
	#if ( portUSING_MPU_WRAPPERS == 1 )
	vPortStoreTaskMPUSettings( pxNewTCB->MPUSettings, NULL, pxBottomOfStack, usStackDepth );
	// vPortStoreTaskMPUSettings((((char *)pxBottomOfStack) + sizeof(void *)), NULL, pxBottomOfStack, usStackDepth );
	#endif
	#if ( configUSE_MUTEXES == 1 )
	{
		pxNewTCB->basePriority = pxNewTCB->priority;
		// pxNewTCB->mutexesHeld = 0;
	}
	#endif /* configUSE_MUTEXES */
}
BaseType_t IRAM_ATTR xTaskGetSchedulerState( void )
{
	extern BOOL g_taskScheduled;
	extern UINT16 g_losTaskLock;
	return g_taskScheduled ? (LOS_CHECK_SCHEDULE ? taskSCHEDULER_RUNNING : taskSCHEDULER_SUSPENDED) : taskSCHEDULER_NOT_STARTED;
}
void vTaskDelete( TaskHandle_t xTaskToDelete )
{
	if(!xTaskToDelete) LOS_TaskDelete(LOS_CurTaskIDGet());
	else LOS_TaskDelete((UINT32)xTaskToDelete);
}
void vTaskDelay( const TickType_t xTicksToDelay )
{
	LOS_TaskDelay(xTicksToDelay);
}
TaskHandle_t xTaskGetCurrentTaskHandle( void )
{
	return (TaskHandle_t)LOS_CurTaskIDGet();
}

TaskHandle_t xTaskGetCurrentTaskHandleForCPU( BaseType_t cpuid )
{
    return (TaskHandle_t)LOS_CurTaskIDGet();
}

#if 0
// TaskHandle_t xTaskGetCurrentTaskHandle( void );
struct {
	TaskHandle_t handle;
	void *pvValue;
}pthread_tls[4];
static volatile unsigned char pthread_tlsi = 0;
static void *pvTaskGetThreadLocalStoragePointer( TaskHandle_t xTaskToQuery, BaseType_t xIndex )
{
	unsigned char maxi = pthread_tlsi;
	if(!xTaskToQuery) xTaskToQuery = xTaskGetCurrentTaskHandle();
	// ESP_LOGW("STL", "Get.handle:%p\n",xTaskToQuery);
	for(unsigned char i = 0;i < maxi;++i) {
		if(xTaskToQuery == pthread_tls[i].handle) return pthread_tls[i].pvValue;
	}
	return NULL;
}
static void vTaskSetThreadLocalStoragePointerAndDelCallback( TaskHandle_t xTaskToSet, BaseType_t xIndex, void *pvValue , void *xDelCallback)
{
	unsigned char maxi = pthread_tlsi;
	if(!xTaskToSet) xTaskToSet = xTaskGetCurrentTaskHandle();
	// ESP_LOGW("STL", "Set.handle:%p value:%p\n",xTaskToSet,pvValue);
	for(unsigned char i = 0;i < maxi;++i) {
		if(xTaskToSet == pthread_tls[i].handle) {pthread_tls[i].pvValue = pvValue;return;}
	}
	pthread_tlsi = maxi + 1;
	pthread_tls[maxi].handle  = xTaskToSet;
	pthread_tls[maxi].pvValue = pvValue;
}
#else
void *pvTaskGetThreadLocalStoragePointer( TaskHandle_t handle, BaseType_t xIndex )
{
	UINT32 taskID = (UINT32)handle;
	LosTaskCB *pxTCB;
	if(!taskID) taskID = LOS_CurTaskIDGet();
	if(xIndex >= sizeof(pxTCB->LocalStoragePointer)/sizeof(pxTCB->LocalStoragePointer[0])){
		esp_rom_printf("\n\e[1;36m%s.xIndex=%d",__func__, xIndex);
		return NULL;
	}
	pxTCB = OS_TCB_FROM_TID(taskID);
	return pxTCB ? pxTCB->LocalStoragePointer[xIndex] : NULL;
}
void vTaskSetThreadLocalStoragePointerAndDelCallback( TaskHandle_t handle, BaseType_t xIndex, void *pvValue, TlsDeleteCallbackFunction_t pvDelCallback)
{
	UINT32 taskID = (UINT32)handle;
	LosTaskCB *pxTCB;
	if(!taskID) taskID = LOS_CurTaskIDGet();
	if(xIndex >= sizeof(pxTCB->LocalStoragePointer)/sizeof(pxTCB->LocalStoragePointer[0])){
		esp_rom_printf("\n\e[1;36m%s.xIndex=%d\e[0m\n",__func__, xIndex);
		return;
	}
	pxTCB = OS_TCB_FROM_TID(taskID);
	if(pxTCB) {
		pxTCB->LocalStoragePointer[xIndex] = pvValue;
		if(xIndex < sizeof(pxTCB->LocalDelCallback)/sizeof(pxTCB->LocalDelCallback[0])){
			pxTCB->LocalDelCallback[xIndex] = (void *)pvDelCallback;
		}
	}
}
void TaskDeleteExtensionHook(void *tcb)
{
	LosTaskCB *pxTCB = (LosTaskCB *)tcb;
	if(pxTCB) {
		for(unsigned i = 0;i < sizeof(pxTCB->LocalDelCallback)/sizeof(pxTCB->LocalDelCallback[0]); ++i) {
			if(pxTCB->LocalDelCallback[i]) {
				((TlsDeleteCallbackFunction_t)pxTCB->LocalDelCallback[i])(i,pxTCB->LocalStoragePointer[i]);
			}
		}
	}
}
#endif

