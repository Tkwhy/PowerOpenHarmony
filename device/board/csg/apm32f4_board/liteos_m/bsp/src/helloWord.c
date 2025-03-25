#include <stdio.h>
#include "ohos_init.h"
#include "ohos_types.h"
#include "los_task.h"
#include "los_config.h"
#include "Board_APM32F407_EVAL.h"
void HelloWorld(void)
{
    APM_EVAL_LEDInit(LED2);
    while (1)
    {
        APM_EVAL_LEDToggle(LED2);
        // printf("Hello world!\r\n");
        LOS_TaskDelay(1000); // 1s 延时
    }
}
void HelloWorldTaskSample(VOID)
{
    uint32 ret;
    UINT32 taskID1;
    TSK_INIT_PARAM_S stTask = {0};
    printf("TaskSample: Task1 create start...\n");
    stTask.pfnTaskEntry = (TSK_ENTRY_FUNC)HelloWorld;
    stTask.uwStackSize = 4096;
    stTask.pcName = "HelloWorld";
    stTask.usTaskPrio = 6;
    ret = LOS_TaskCreate(&taskID1, &stTask);
    if (ret != LOS_OK)
    {
        printf("Task1 create failed\n");
    }
}
// SYS_RUN(HelloWorldTaskSample);