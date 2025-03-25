#include <stdio.h>
#include "ohos_init.h"
#include "ohos_types.h"
#include "los_task.h"
#include "apm32f4xx.h"
#include "apm32f4xx_gpio.h"
#include "apm32f4xx_rcm.h"
#include "Board_APM32F407_EVAL.h"

void HelloWorld(void)
{
    while (1)
    {
        // printf("Hello world!\r\n");
        APM_EVAL_LEDToggle(LED2);
        LOS_TaskDelay(1000); // 1s 延时
    }
}
VOID HelloWorldTaskSample(VOID)
{
    UINT32 ret;
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

SYS_RUN(HelloWorldTaskSample);