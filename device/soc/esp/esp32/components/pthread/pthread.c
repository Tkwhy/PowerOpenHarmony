// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// This module implements pthread API on top of FreeRTOS. API is implemented to the level allowing
// libstdcxx threading framework to operate correctly. So not all original pthread routines are supported.
//

#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include "esp_err.h"
#include "esp_attr.h"
#include "sys/queue.h"
#include "esp_osal/esp_osal.h"
#include "esp_osal/task.h"
#include "esp_osal/semphr.h"
#include "soc/soc_memory_layout.h"

#include "pthread_internal.h"
#include "esp_pthread.h"

#define LOG_LOCAL_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include "esp_log.h"
const static char *TAG = "pthread";
#if 0
/** task state */
enum esp_pthread_task_state {
    PTHREAD_TASK_STATE_RUN,
    PTHREAD_TASK_STATE_EXIT
};

/** pthread thread FreeRTOS wrapper */
typedef struct esp_pthread_entry {
    SLIST_ENTRY(esp_pthread_entry)  list_node;  ///< Tasks list node struct.
    TaskHandle_t                handle;         ///< FreeRTOS task handle
    TaskHandle_t                join_task;      ///< Handle of the task waiting to join
    enum esp_pthread_task_state state;          ///< pthread task state
    bool                        detached;       ///< True if pthread is detached
    void                       *retval;         ///< Value supplied to calling thread during join
    void                       *task_arg;       ///< Task arguments
} esp_pthread_t;

/** pthread wrapper task arg */
typedef struct {
    void *(*func)(void *);  ///< user task entry
    void *arg;              ///< user task argument
    esp_pthread_cfg_t cfg;  ///< pthread configuration
} esp_pthread_task_arg_t;

/** pthread mutex FreeRTOS wrapper */
typedef struct {
    SemaphoreHandle_t   sem;        ///< Handle of the task waiting to join
    int                 type;       ///< Mutex type. Currently supported PTHREAD_MUTEX_NORMAL and PTHREAD_MUTEX_RECURSIVE
} esp_pthread_mutex_t;


static SemaphoreHandle_t s_threads_mux  = NULL;
static portMUX_TYPE s_mutex_init_lock   = portMUX_INITIALIZER_UNLOCKED;
static SLIST_HEAD(esp_thread_list_head, esp_pthread_entry) s_threads_list
                                        = SLIST_HEAD_INITIALIZER(s_threads_list);
static pthread_key_t s_pthread_cfg_key;


static int IRAM_ATTR pthread_mutex_lock_internal(esp_pthread_mutex_t *mux, TickType_t tmo);

static void esp_pthread_cfg_key_destructor(void *value)
{
    free(value);
}

esp_err_t esp_pthread_init(void)
{
    if (pthread_key_create(&s_pthread_cfg_key, esp_pthread_cfg_key_destructor) != 0) {
        return ESP_ERR_NO_MEM;
    }
    s_threads_mux = xSemaphoreCreateMutex();
    if (s_threads_mux == NULL) {
        pthread_key_delete(s_pthread_cfg_key);
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

static void *pthread_list_find_item(void *(*item_check)(esp_pthread_t *, void *arg), void *check_arg)
{
    esp_pthread_t *it;
    SLIST_FOREACH(it, &s_threads_list, list_node) {
        void *val = item_check(it, check_arg);
        if (val) {
            return val;
        }
    }
    return NULL;
}

static void *pthread_get_handle_by_desc(esp_pthread_t *item, void *desc)
{
    if (item == desc) {
        return item->handle;
    }
    return NULL;
}

static void *pthread_get_desc_by_handle(esp_pthread_t *item, void *hnd)
{
    if (hnd == item->handle) {
        return item;
    }
    return NULL;
}

static inline TaskHandle_t pthread_find_handle(pthread_t thread)
{
    return pthread_list_find_item(pthread_get_handle_by_desc, (void *)thread);
}

static esp_pthread_t *pthread_find(TaskHandle_t task_handle)
{
    return pthread_list_find_item(pthread_get_desc_by_handle, task_handle);
}

static void pthread_delete(esp_pthread_t *pthread)
{
    SLIST_REMOVE(&s_threads_list, pthread, esp_pthread_entry, list_node);
    free(pthread);
}

/* Call this function to configure pthread stacks in Pthreads */
esp_err_t esp_pthread_set_cfg(const esp_pthread_cfg_t *cfg)
{
    if (cfg->stack_size < PTHREAD_STACK_MIN) {
        return ESP_ERR_INVALID_ARG;
    }

    /* If a value is already set, update that value */
    esp_pthread_cfg_t *p = pthread_getspecific(s_pthread_cfg_key);
    if (!p) {
        p = malloc(sizeof(esp_pthread_cfg_t));
        if (!p) {
            return ESP_ERR_NO_MEM;
        }
    }
    *p = *cfg;
    pthread_setspecific(s_pthread_cfg_key, p);
    return 0;
}

esp_err_t esp_pthread_get_cfg(esp_pthread_cfg_t *p)
{
    esp_pthread_cfg_t *cfg = pthread_getspecific(s_pthread_cfg_key);
    if (cfg) {
        *p = *cfg;
        return ESP_OK;
    }
    memset(p, 0, sizeof(*p));
    return ESP_ERR_NOT_FOUND;
}

static int get_default_pthread_core(void)
{
    return CONFIG_PTHREAD_TASK_CORE_DEFAULT == -1 ? tskNO_AFFINITY : CONFIG_PTHREAD_TASK_CORE_DEFAULT;
}

esp_pthread_cfg_t esp_pthread_get_default_config(void)
{
    esp_pthread_cfg_t cfg = {
        .stack_size = CONFIG_PTHREAD_TASK_STACK_SIZE_DEFAULT,
        .prio = CONFIG_PTHREAD_TASK_PRIO_DEFAULT,
        .inherit_cfg = false,
        .thread_name = NULL,
        .pin_to_core = get_default_pthread_core()
    };

    return cfg;
}

static void pthread_task_func(void *arg)
{
    void *rval = NULL;
    esp_pthread_task_arg_t *task_arg = (esp_pthread_task_arg_t *)arg;

    ESP_LOGV(TAG, "%s ENTER %p", __FUNCTION__, task_arg->func);

    // wait for start
    xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

    if (task_arg->cfg.inherit_cfg) {
        /* If inherit option is set, then do a set_cfg() ourselves for future forks,
        but first set thread_name to NULL to enable inheritance of the name too.
        (This also to prevents dangling pointers to name of tasks that might
        possibly have been deleted when we use the configuration).*/
        esp_pthread_cfg_t *cfg = &task_arg->cfg;
        cfg->thread_name = NULL;
        esp_pthread_set_cfg(cfg);
    }
    ESP_LOGV(TAG, "%s START %p", __FUNCTION__, task_arg->func);
    rval = task_arg->func(task_arg->arg);
    ESP_LOGV(TAG, "%s END %p", __FUNCTION__, task_arg->func);

    pthread_exit(rval);

    ESP_LOGV(TAG, "%s EXIT", __FUNCTION__);
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine) (void *), void *arg)
{
    TaskHandle_t xHandle = NULL;

    ESP_LOGV(TAG, "%s", __FUNCTION__);
    esp_pthread_task_arg_t *task_arg = calloc(1, sizeof(esp_pthread_task_arg_t));
    if (task_arg == NULL) {
        ESP_LOGE(TAG, "Failed to allocate task args!");
        return ENOMEM;
    }

    esp_pthread_t *pthread = calloc(1, sizeof(esp_pthread_t));
    if (pthread == NULL) {
        ESP_LOGE(TAG, "Failed to allocate pthread data!");
        free(task_arg);
        return ENOMEM;
    }

    uint32_t stack_size = CONFIG_PTHREAD_TASK_STACK_SIZE_DEFAULT;
    BaseType_t prio = CONFIG_PTHREAD_TASK_PRIO_DEFAULT;
    BaseType_t core_id = get_default_pthread_core();
    const char *task_name = CONFIG_PTHREAD_TASK_NAME_DEFAULT;

    esp_pthread_cfg_t *pthread_cfg = pthread_getspecific(s_pthread_cfg_key);
    if (pthread_cfg) {
        if (pthread_cfg->stack_size) {
            stack_size = pthread_cfg->stack_size;
        }
        if (pthread_cfg->prio && pthread_cfg->prio < configMAX_PRIORITIES) {
            prio = pthread_cfg->prio;
        }

        if (pthread_cfg->inherit_cfg) {
            if (pthread_cfg->thread_name == NULL) {
                // Inherit task name from current task.
                task_name = pcTaskGetTaskName(NULL);
            } else {
                // Inheriting, but new task name.
                task_name = pthread_cfg->thread_name;
            }
        } else if (pthread_cfg->thread_name == NULL) {
            task_name = CONFIG_PTHREAD_TASK_NAME_DEFAULT;
        } else {
            task_name = pthread_cfg->thread_name;
        }

        if (pthread_cfg->pin_to_core >= 0 && pthread_cfg->pin_to_core < portNUM_PROCESSORS) {
            core_id = pthread_cfg->pin_to_core;
        }

        task_arg->cfg = *pthread_cfg;
    }

    if (attr) {
        /* Overwrite attributes */
        stack_size = attr->stacksize;

        switch (attr->detachstate) {
        case PTHREAD_CREATE_DETACHED:
            pthread->detached = true;
            break;
        case PTHREAD_CREATE_JOINABLE:
        default:
            pthread->detached = false;
        }
    }

    task_arg->func = start_routine;
    task_arg->arg = arg;
    pthread->task_arg = task_arg;
    BaseType_t res = xTaskCreatePinnedToCore(&pthread_task_func,
                                             task_name,
                                             // stack_size is in bytes. This transformation ensures that the units are
                                             // transformed to the units used in FreeRTOS.
                                             // Note: float division of ceil(m / n) ==
                                             //       integer division of (m + n - 1) / n
                                             (stack_size + sizeof(StackType_t) - 1) / sizeof(StackType_t),
                                             task_arg,
                                             prio,
                                             &xHandle,
                                             core_id);

    if (res != pdPASS) {
        ESP_LOGE(TAG, "Failed to create task!");
        free(pthread);
        free(task_arg);
        if (res == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY) {
            return ENOMEM;
        } else {
            return EAGAIN;
        }
    }
    pthread->handle = xHandle;

    if (xSemaphoreTake(s_threads_mux, portMAX_DELAY) != pdTRUE) {
        assert(false && "Failed to lock threads list!");
    }
    SLIST_INSERT_HEAD(&s_threads_list, pthread, list_node);
    xSemaphoreGive(s_threads_mux);

    // start task
    xTaskNotify(xHandle, 0, eNoAction);

    *thread = (pthread_t)pthread; // pointer value fit into pthread_t (uint32_t)

    ESP_LOGV(TAG, "Created task %x", (uint32_t)xHandle);

    return 0;
}

int pthread_join(pthread_t thread, void **retval)
{
    esp_pthread_t *pthread = (esp_pthread_t *)thread;
    int ret = 0;
    bool wait = false;
    void *child_task_retval = 0;

    ESP_LOGV(TAG, "%s %p", __FUNCTION__, pthread);

    // find task
    if (xSemaphoreTake(s_threads_mux, portMAX_DELAY) != pdTRUE) {
        assert(false && "Failed to lock threads list!");
    }
    TaskHandle_t handle = pthread_find_handle(thread);
    if (!handle) {
        // not found
        ret = ESRCH;
    } else if (pthread->detached) {
        // Thread is detached
        ret = EDEADLK;
    } else if (pthread->join_task) {
        // already have waiting task to join
        ret = EINVAL;
    } else if (handle == xTaskGetCurrentTaskHandle()) {
        // join to self not allowed
        ret = EDEADLK;
    } else {
        esp_pthread_t *cur_pthread = pthread_find(xTaskGetCurrentTaskHandle());
        if (cur_pthread && cur_pthread->join_task == handle) {
            // join to each other not allowed
            ret = EDEADLK;
        } else {
            if (pthread->state == PTHREAD_TASK_STATE_RUN) {
                pthread->join_task = xTaskGetCurrentTaskHandle();
                wait = true;
            } else { // thread has exited and task is already suspended, or about to be suspended
                child_task_retval = pthread->retval;
                pthread_delete(pthread);
            }
        }
    }
    xSemaphoreGive(s_threads_mux);

    if (ret == 0) {
        if (wait) {
            xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
            if (xSemaphoreTake(s_threads_mux, portMAX_DELAY) != pdTRUE) {
                assert(false && "Failed to lock threads list!");
            }
            child_task_retval = pthread->retval;
            pthread_delete(pthread);
            xSemaphoreGive(s_threads_mux);
        }
        vTaskDelete(handle);
    }

    if (retval) {
        *retval = child_task_retval;
    }

    ESP_LOGV(TAG, "%s %p EXIT %d", __FUNCTION__, pthread, ret);
    return ret;
}

int pthread_detach(pthread_t thread)
{
    esp_pthread_t *pthread = (esp_pthread_t *)thread;
    int ret = 0;

    if (xSemaphoreTake(s_threads_mux, portMAX_DELAY) != pdTRUE) {
        assert(false && "Failed to lock threads list!");
    }
    TaskHandle_t handle = pthread_find_handle(thread);
    if (!handle) {
        ret = ESRCH;
    } else if (pthread->detached) {
        // already detached
        ret = EINVAL;
    } else if (pthread->join_task) {
        // already have waiting task to join
        ret = EINVAL;
    } else if (pthread->state == PTHREAD_TASK_STATE_RUN) {
        // pthread still running
        pthread->detached = true;
    } else {
        // pthread already stopped
        pthread_delete(pthread);
        vTaskDelete(handle);
    }
    xSemaphoreGive(s_threads_mux);
    ESP_LOGV(TAG, "%s %p EXIT %d", __FUNCTION__, pthread, ret);
    return ret;
}

void pthread_exit(void *value_ptr)
{
    bool detached = false;
    /* preemptively clean up thread local storage, rather than
       waiting for the idle task to clean up the thread */
    pthread_internal_local_storage_destructor_callback();

    if (xSemaphoreTake(s_threads_mux, portMAX_DELAY) != pdTRUE) {
        assert(false && "Failed to lock threads list!");
    }
    esp_pthread_t *pthread = pthread_find(xTaskGetCurrentTaskHandle());
    if (!pthread) {
        assert(false && "Failed to find pthread for current task!");
    }
    if (pthread->task_arg) {
        free(pthread->task_arg);
    }
    if (pthread->detached) {
        // auto-free for detached threads
        pthread_delete(pthread);
        detached = true;
    } else {
        // Set return value
        pthread->retval = value_ptr;
        // Remove from list, it indicates that task has exited
        if (pthread->join_task) {
            // notify join
            xTaskNotify(pthread->join_task, 0, eNoAction);
        } else {
            pthread->state = PTHREAD_TASK_STATE_EXIT;
        }
    }

    ESP_LOGD(TAG, "Task stk_wm = %d", uxTaskGetStackHighWaterMark(NULL));

    xSemaphoreGive(s_threads_mux);
    // note: if this thread is joinable then after giving back s_threads_mux
    // this task could be deleted at any time, so don't take another lock or
    // do anything that might lock (such as printing to stdout)

    if (detached) {
        vTaskDelete(NULL);
    } else {
        vTaskSuspend(NULL);
    }

    // Should never be reached
    abort();
}

int pthread_cancel(pthread_t thread)
{
    ESP_LOGE(TAG, "%s: not supported!", __FUNCTION__);
    return ENOSYS;
}

int sched_yield( void )
{
    vTaskDelay(0);
    return 0;
}

pthread_t pthread_self(void)
{
    if (xSemaphoreTake(s_threads_mux, portMAX_DELAY) != pdTRUE) {
        assert(false && "Failed to lock threads list!");
    }
    esp_pthread_t *pthread = pthread_find(xTaskGetCurrentTaskHandle());
    if (!pthread) {
        assert(false && "Failed to find current thread ID!");
    }
    xSemaphoreGive(s_threads_mux);
    return (pthread_t)pthread;
}

int pthread_equal(pthread_t t1, pthread_t t2)
{
    return t1 == t2 ? 1 : 0;
}

/***************** ONCE ******************/
int pthread_once(pthread_once_t *once_control, void (*init_routine)(void))
{
    if (once_control == NULL || init_routine == NULL || !once_control->is_initialized) {
        ESP_LOGE(TAG, "%s: Invalid args!", __FUNCTION__);
        return EINVAL;
    }

    uint32_t res = 1;
#if defined(CONFIG_SPIRAM)
    if (esp_ptr_external_ram(once_control)) {
        uxPortCompareSetExtram((uint32_t *) &once_control->init_executed, 0, &res);
    } else {
#endif
        uxPortCompareSet((uint32_t *) &once_control->init_executed, 0, &res);
#if defined(CONFIG_SPIRAM)
    }
#endif
    // Check if compare and set was successful
    if (res == 0) {
        ESP_LOGV(TAG, "%s: call init_routine %p", __FUNCTION__, once_control);
        init_routine();
    }

    return 0;
}

/***************** MUTEX ******************/
static int mutexattr_check(const pthread_mutexattr_t *attr)
{
    if (attr->type != PTHREAD_MUTEX_NORMAL &&
        attr->type != PTHREAD_MUTEX_RECURSIVE &&
        attr->type != PTHREAD_MUTEX_ERRORCHECK) {
        return EINVAL;
    }
    return 0;
}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
    int type = PTHREAD_MUTEX_NORMAL;

    if (!mutex) {
        return EINVAL;
    }

    if (attr) {
        if (!attr->is_initialized) {
            return EINVAL;
        }
        int res = mutexattr_check(attr);
        if (res) {
            return res;
        }
        type = attr->type;
    }

    esp_pthread_mutex_t *mux = (esp_pthread_mutex_t *)malloc(sizeof(esp_pthread_mutex_t));
    if (!mux) {
        return ENOMEM;
    }
    mux->type = type;

    if (mux->type == PTHREAD_MUTEX_RECURSIVE) {
        mux->sem = xSemaphoreCreateRecursiveMutex();
    } else {
        mux->sem = xSemaphoreCreateMutex();
    }
    if (!mux->sem) {
        free(mux);
        return EAGAIN;
    }

    *mutex = (pthread_mutex_t)mux; // pointer value fit into pthread_mutex_t (uint32_t)

    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    esp_pthread_mutex_t *mux;

    ESP_LOGV(TAG, "%s %p", __FUNCTION__, mutex);

    if (!mutex) {
        return EINVAL;
    }
    mux = (esp_pthread_mutex_t *)*mutex;
    if (!mux) {
        return EINVAL;
    }

    // check if mux is busy
    int res = pthread_mutex_lock_internal(mux, 0);
    if (res == EBUSY) {
        return EBUSY;
    }

    if (mux->type == PTHREAD_MUTEX_RECURSIVE) {
        res = xSemaphoreGiveRecursive(mux->sem);
    } else {
        res = xSemaphoreGive(mux->sem);
    }
    if (res != pdTRUE) {
        assert(false && "Failed to release mutex!");
    }
    vSemaphoreDelete(mux->sem);
    free(mux);

    return 0;
}

static int IRAM_ATTR pthread_mutex_lock_internal(esp_pthread_mutex_t *mux, TickType_t tmo)
{
    if (!mux) {
        return EINVAL;
    }

    if ((mux->type == PTHREAD_MUTEX_ERRORCHECK) &&
        (xSemaphoreGetMutexHolder(mux->sem) == xTaskGetCurrentTaskHandle())) {
        return EDEADLK;
    }

    if (mux->type == PTHREAD_MUTEX_RECURSIVE) {
        if (xSemaphoreTakeRecursive(mux->sem, tmo) != pdTRUE) {
            return EBUSY;
        }
    } else {
        if (xSemaphoreTake(mux->sem, tmo) != pdTRUE) {
            return EBUSY;
        }
    }

    return 0;
}

static int pthread_mutex_init_if_static(pthread_mutex_t *mutex)
{
    int res = 0;
    if ((intptr_t) *mutex == PTHREAD_MUTEX_INITIALIZER) {
        portENTER_CRITICAL(&s_mutex_init_lock);
        if ((intptr_t) *mutex == PTHREAD_MUTEX_INITIALIZER) {
            res = pthread_mutex_init(mutex, NULL);
        }
        portEXIT_CRITICAL(&s_mutex_init_lock);
    }
    return res;
}

int IRAM_ATTR pthread_mutex_lock(pthread_mutex_t *mutex)
{
    if (!mutex) {
        return EINVAL;
    }
    int res = pthread_mutex_init_if_static(mutex);
    if (res != 0) {
        return res;
    }
    return pthread_mutex_lock_internal((esp_pthread_mutex_t *)*mutex, portMAX_DELAY);
}

int IRAM_ATTR pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *timeout)
{
    if (!mutex) {
        return EINVAL;
    }
    int res = pthread_mutex_init_if_static(mutex);
    if (res != 0) {
        return res;
    }

    struct timespec currtime;
    clock_gettime(CLOCK_REALTIME, &currtime);
    TickType_t tmo = ((timeout->tv_sec - currtime.tv_sec)*1000 +
                     (timeout->tv_nsec - currtime.tv_nsec)/1000000)/portTICK_PERIOD_MS;

    res = pthread_mutex_lock_internal((esp_pthread_mutex_t *)*mutex, tmo);
    if (res == EBUSY) {
        return ETIMEDOUT;
    }
    return res;
}

int IRAM_ATTR pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    if (!mutex) {
        return EINVAL;
    }
    int res = pthread_mutex_init_if_static(mutex);
    if (res != 0) {
        return res;
    }
    return pthread_mutex_lock_internal((esp_pthread_mutex_t *)*mutex, 0);
}

int IRAM_ATTR pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    esp_pthread_mutex_t *mux;

    if (!mutex) {
        return EINVAL;
    }
    mux = (esp_pthread_mutex_t *)*mutex;
    if (!mux) {
        return EINVAL;
    }

    if (((mux->type == PTHREAD_MUTEX_RECURSIVE) ||
        (mux->type == PTHREAD_MUTEX_ERRORCHECK)) &&
        (xSemaphoreGetMutexHolder(mux->sem) != xTaskGetCurrentTaskHandle())) {
        return EPERM;
    }

    int ret;
    if (mux->type == PTHREAD_MUTEX_RECURSIVE) {
        ret = xSemaphoreGiveRecursive(mux->sem);
    } else {
        ret = xSemaphoreGive(mux->sem);
    }
    if (ret != pdTRUE) {
        assert(false && "Failed to unlock mutex!");
    }
    return 0;
}

int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
    if (!attr) {
        return EINVAL;
    }
    attr->type = PTHREAD_MUTEX_NORMAL;
    attr->is_initialized = 1;
    return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
    if (!attr) {
        return EINVAL;
    }
    attr->is_initialized = 0;
    return 0;
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
    if (!attr) {
        return EINVAL;
    }
    *type = attr->type;
    return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
    if (!attr) {
        return EINVAL;
    }
    pthread_mutexattr_t tmp_attr = {.type = type};
    int res = mutexattr_check(&tmp_attr);
    if (!res) {
        attr->type = type;
    }
    return res;
}

/***************** ATTRIBUTES ******************/
int pthread_attr_init(pthread_attr_t *attr)
{
    if (attr) {
        /* Nothing to allocate. Set everything to default */
        attr->stacksize   = CONFIG_PTHREAD_TASK_STACK_SIZE_DEFAULT;
        attr->detachstate = PTHREAD_CREATE_JOINABLE;
        return 0;
    }
    return EINVAL;
}

int pthread_attr_destroy(pthread_attr_t *attr)
{
    if (attr) {
        /* Nothing to deallocate. Reset everything to default */
        attr->stacksize   = CONFIG_PTHREAD_TASK_STACK_SIZE_DEFAULT;
        attr->detachstate = PTHREAD_CREATE_JOINABLE;
        return 0;
    }
    return EINVAL;
}

int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize)
{
    if (attr) {
        *stacksize = attr->stacksize;
        return 0;
    }
    return EINVAL;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
    if (attr && !(stacksize < PTHREAD_STACK_MIN)) {
        attr->stacksize = stacksize;
        return 0;
    }
    return EINVAL;
}

int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate)
{
    if (attr) {
        *detachstate = attr->detachstate;
        return 0;
    }
    return EINVAL;
}

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
    if (attr) {
        switch (detachstate) {
        case PTHREAD_CREATE_DETACHED:
            attr->detachstate = PTHREAD_CREATE_DETACHED;
            break;
        case PTHREAD_CREATE_JOINABLE:
            attr->detachstate = PTHREAD_CREATE_JOINABLE;
            break;
        default:
            return EINVAL;
        }
        return 0;
    }
    return EINVAL;
}

/* Hook function to force linking this file */
void pthread_include_pthread_impl(void)
{
}
#elif 1
#include "los_config.h"
#include <string.h>
#include <stdlib.h>
typedef struct{
	UINT32 id;
	char type;
}s_pthreadSt;
static pthread_key_t s_pthread_cfg_key;
static char pthreadName[] = {"posix_pthread"};
static void esp_pthread_cfg_key_destructor(void *value)
{
    free(value);
}
esp_err_t esp_pthread_init(void)
{
    if (pthread_key_create(&s_pthread_cfg_key, esp_pthread_cfg_key_destructor) != 0) {
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}
int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	char type;
	UINT32 id;
	s_pthreadSt *sem = (s_pthreadSt *)mutex;
	if(!mutex) return EINVAL;
    if ((intptr_t) *mutex == PTHREAD_MUTEX_INITIALIZER) {
		ESP_LOGE(TAG,"pthread_mutex_lock(PTHREAD_MUTEX_INITIALIZER)");
		pthread_mutex_init(mutex,NULL);
    }
	type = sem->type;
	id   = sem->id;
	if(0x50 == type) {
		return (LOS_OK != LOS_MuxPend(id, LOS_WAIT_FOREVER)) ? EBUSY : 0;
	}else
	if(0x51 == type) {
		return (LOS_OK != LOS_SemPend(id, LOS_WAIT_FOREVER)) ? EBUSY : 0;
	}
	return EINVAL;
}
int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	char type;
	UINT32 id;
	s_pthreadSt *sem = (s_pthreadSt *)mutex;
	if(!mutex) return EINVAL;
	if((intptr_t) *mutex == PTHREAD_MUTEX_INITIALIZER) return EINVAL;
	type = sem->type;
	id   = sem->id;
	if(0x50 == type) {
		return (LOS_OK != LOS_MuxPost(id)) ? EPERM : 0;
	}else
	if(0x51 == type) {
		return (LOS_OK != LOS_SemPost(id)) ? EPERM : 0;
	}
	return 0;
}
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
    int type = PTHREAD_MUTEX_NORMAL;

    if (!mutex) {
        return EINVAL;
    }

    if (attr) {
        if (!attr->is_initialized) {
            return EINVAL;
        }
		type = attr->type;
	    if (type != PTHREAD_MUTEX_NORMAL &&
	        type != PTHREAD_MUTEX_RECURSIVE &&
	        type != PTHREAD_MUTEX_ERRORCHECK) {
	        return EINVAL;
	    }
    }

    s_pthreadSt *mux = (s_pthreadSt *)malloc(sizeof(s_pthreadSt));
    if (!mux) {
        return ENOMEM;
    }

	mux->id = 0;
    if (type == PTHREAD_MUTEX_RECURSIVE) {
		mux->type = 0x50;
		if(LOS_OK != LOS_MuxCreate(&mux->id)){
			free(mux);
			return EAGAIN;
		}
    } else {
		mux->type = 0x51;
		if(LOS_OK != LOS_BinarySemCreate(1,&mux->id)){
			free(mux);
			return EAGAIN;
		}
    }
    *mutex = (pthread_mutex_t)mux; // pointer value fit into pthread_mutex_t (uint32_t)
    return 0;
}
int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	char type;
	UINT32 id;
	s_pthreadSt *sem = (s_pthreadSt *)mutex;
	if(!mutex) return 0;
	type = sem->type;
	id   = sem->id;
	sem->type = 0;
	sem->id = 0;
	free(sem);
	if(0x50 == type) {
		LOS_MuxPost(id);
		LOS_MuxDelete(id);
	}else
	if(0x51 == type) {
		LOS_SemPost(id);
		LOS_SemDelete(id);
	}
	return 0;
}
#define MUTEXATTR_TYPE_MASK   0x0FU
int pthread_mutexattr_init(pthread_mutexattr_t *mutexAttr)
{
    if (mutexAttr == NULL) {
        return EINVAL;
    }

    mutexAttr->type = PTHREAD_MUTEX_DEFAULT;

    return 0;
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *outType)
{
    INT32 type;

    if ((attr == NULL) || (outType == NULL)) {
        return EINVAL;
    }

    type = (INT32)(attr->type & MUTEXATTR_TYPE_MASK);
    if ((type != PTHREAD_MUTEX_NORMAL) &&
        (type != PTHREAD_MUTEX_RECURSIVE) &&
        (type != PTHREAD_MUTEX_ERRORCHECK)) {
        return EINVAL;
    }

    *outType = type;

    return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *mutexAttr, int type)
{
    if (mutexAttr == NULL) {
        return EINVAL;
    }

    if (((unsigned)type != PTHREAD_MUTEX_NORMAL) &&
        ((unsigned)type != PTHREAD_MUTEX_RECURSIVE) &&
        ((unsigned)type != PTHREAD_MUTEX_ERRORCHECK)) {
        return EINVAL;
    }
    mutexAttr->type = (UINT8)((mutexAttr->type & ~MUTEXATTR_TYPE_MASK) | (UINT32)type);

    return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *mutexAttr)
{
    if (mutexAttr == NULL) {
        return EINVAL;
    }

    memset(mutexAttr, 0, sizeof(pthread_mutexattr_t));

    return 0;
}

void pthread_exit(void *value_ptr)
{
	LOS_TaskDelete(LOS_CurTaskIDGet());
	while (1) {
		LOS_TaskSuspend(LOS_CurTaskIDGet());
		LOS_TaskDelay(10);
	}
}

int sched_get_priority_min(int policy)
{
    if (policy != SCHED_RR) {
        errno = EINVAL;
        return -1;
    }

    return LOS_TASK_PRIORITY_LOWEST;
}

int sched_get_priority_max(int policy)
{
    if (policy != SCHED_RR) {
        errno = EINVAL;
        return -1;
    }

    return LOS_TASK_PRIORITY_HIGHEST;
}

#define POSIX_THREAD_STACK_SIZE LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE
int pthread_attr_init(pthread_attr_t *attr)
{
    if (NULL == attr) {
        return EINVAL;
    }
	memset(attr, 0, sizeof(pthread_attr_t));
    attr->detachstate                 = PTHREAD_CREATE_JOINABLE;
    attr->schedpolicy                 = SCHED_RR;
    attr->schedparam.sched_priority   = LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO;
    attr->inheritsched                = PTHREAD_INHERIT_SCHED;
    attr->stackaddr                   = NULL;
    attr->stacksize                   = POSIX_THREAD_STACK_SIZE;

    return 0;
}
int ServerStubInit(void)
{

    return 0;
}

int pthread_attr_destroy(pthread_attr_t *attr)
{
    if (attr == NULL) {
        return EINVAL;
    }

    memset(attr, 0, sizeof(pthread_attr_t));

    return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachState)
{
    if ((attr != NULL) && ((detachState == PTHREAD_CREATE_JOINABLE) || (detachState == PTHREAD_CREATE_DETACHED))) {
        attr->detachstate = (UINT32)detachState;
        return 0;
    }

    return EINVAL;
}

int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachState)
{
    if ((attr == NULL) || (detachState == NULL)) {
        return EINVAL;
    }

    *detachState = (int)attr->detachstate;

    return 0;
}

int pthread_attr_setscope(pthread_attr_t *attr, int scope)
{
    if (attr == NULL) {
        return EINVAL;
    }

    if (scope == PTHREAD_SCOPE_SYSTEM) {
        return 0;
    }

    if (scope == PTHREAD_SCOPE_PROCESS) {
        return ENOTSUP;
    }

    return EINVAL;
}
#if 0
int pthread_attr_getscope(const pthread_attr_t *attr, int *scope)
{
    if ((attr == NULL) || (scope == NULL)) {
        return EINVAL;
    }

    *scope = (int)attr->scope;

    return 0;
}
#endif
int pthread_setname_np(pthread_t thread, const char *name)
{
	LosTaskCB *taskCB;
	taskCB = OS_TCB_FROM_TID((UINT32)thread);
	taskCB->taskName = (char *)name;
	return 0;
}
int strcpy_s(char *strDest, size_t destMax, const char *strSrc);
int pthread_getname_np(pthread_t thread, char *buf, size_t buflen)
{
	LosTaskCB *taskCB;
	taskCB = OS_TCB_FROM_TID((UINT32)thread);
	if (pthreadName == taskCB->taskName) return EINVAL;
	if (NULL == taskCB->taskName) return EINVAL;
	strcpy_s(buf, buflen, taskCB->taskName);
    return ERANGE;
}

int pthread_attr_setinheritsched(pthread_attr_t *attr, int inherit)
{
    if ((attr != NULL) && ((inherit == PTHREAD_INHERIT_SCHED) || (inherit == PTHREAD_EXPLICIT_SCHED))) {
        attr->inheritsched = (UINT32)inherit;
        return 0;
    }

    return EINVAL;
}

int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inherit)
{
    if ((attr == NULL) || (inherit == NULL)) {
        return EINVAL;
    }

    *inherit = (int)attr->inheritsched;

    return 0;
}

int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
    if ((attr != NULL) && (policy == SCHED_RR)) {
        attr->schedpolicy = SCHED_RR;
        return 0;
    }

    return EINVAL;
}

int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy)
{
    if ((attr == NULL) || (policy == NULL)) {
        return EINVAL;
    }

    *policy = (int)attr->schedpolicy;

    return 0;
}

int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param)
{
    if ((attr == NULL) || (param == NULL)) {
        return EINVAL;
    } else if ((param->sched_priority < LOS_TASK_PRIORITY_HIGHEST) ||
               (param->sched_priority >= LOS_TASK_PRIORITY_LOWEST)) {
        return ENOTSUP;
    }

    attr->schedparam = *param;

    return 0;
}

int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param)
{
    if ((attr == NULL) || (param == NULL)) {
        return EINVAL;
    }

    *param = attr->schedparam;

    return 0;
}

int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackAddr)
{
    if (attr == NULL) {
        return EINVAL;
    }

    attr->stackaddr     = stackAddr;

    return 0;
}

int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackAddr)
{
    if (((attr != NULL) && (stackAddr != NULL))) {
        *stackAddr = attr->stackaddr;
        return 0;
    }

    return EINVAL; /* Stack address not set, return EINVAL. */
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stackSize)
{
    /* Reject inadequate stack sizes */
    if ((attr == NULL) || (stackSize < PTHREAD_STACK_MIN)) {
        return EINVAL;
    }

    attr->stacksize     = stackSize;
    return 0;
}

int pthread_attr_setstack(pthread_attr_t *attr, void *stackAddr, size_t stackSize)
{
    if ((attr == NULL) || (stackAddr == NULL) || (stackSize < PTHREAD_STACK_MIN)) {
        return EINVAL;
    }

    attr->stacksize     = stackSize;
    attr->stackaddr     = stackAddr;
    return 0;
}

int pthread_attr_getstack(const pthread_attr_t *attr, void **stackAddr, size_t *stackSize)
{
    if ((attr == NULL) || (stackAddr == NULL) || (stackSize == NULL)) {
        return EINVAL;
    }

    *stackAddr = attr->stackaddr;
    *stackSize = attr->stacksize;
    return 0;
}

int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stackSize)
{
    /* Reject attempts to get a stack size when one has not been set. */
    if ((attr == NULL) || (stackSize == NULL)) {
        return EINVAL;
    }

    *stackSize = attr->stacksize;

    return 0;
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*startRoutine) (void *), void *arg)
{
    TaskHandle_t xHandle = NULL;
	TSK_INIT_PARAM_S param;
    if ((thread == NULL) || (startRoutine == NULL)) {
        return EINVAL;
    }
	memset(&param, 0, sizeof(param));
	param.pfnTaskEntry = (TSK_ENTRY_FUNC)startRoutine;
	param.usTaskPrio = LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO;
	param.uwArg = (UINT32)arg;
	param.uwStackSize = POSIX_THREAD_STACK_SIZE;
	param.uwResved = LOS_TASK_ATTR_JOINABLE;
	param.pcName = pthreadName;
	if (attr) {
		if (attr->stackaddr) {
			param.stackAddr = (UINTPTR)attr->stackaddr;
			param.uwStackSize = attr->stacksize ? attr->stacksize : POSIX_THREAD_STACK_SIZE;
		}
		if(PTHREAD_CREATE_DETACHED == attr->detachstate ){
			param.uwResved = 0;
		}
		if(0 != attr->schedparam.sched_priority) param.usTaskPrio = attr->schedparam.sched_priority;
	}

	{
		UINT32 taskID = 0;
		LOS_TaskCreate(&taskID, &param);
		if (thread) *(UINT32 *)thread = taskID;
	}
    return 0;
}

pthread_t pthread_self(void)
{
    return (pthread_t)LOS_CurTaskIDGet();
}

int pthread_join(pthread_t thread, void **retval)
{
    UINTPTR result;
    UINT32 ret;
	
    ret = LOS_TaskJoin((UINT32)thread, &result);
    if (ret == LOS_ERRNO_TSK_NOT_JOIN_SELF) {
        return EDEADLK;
    } else if ((ret == LOS_ERRNO_TSK_NOT_CREATED) ||
               (ret == LOS_ERRNO_TSK_OPERATE_IDLE) ||
               (ret == LOS_ERRNO_TSK_ID_INVALID) ||
               (ret == LOS_ERRNO_TSK_SUSPEND_SWTMR_NOT_ALLOWED)) {
        return ESRCH;
    } else if (ret != LOS_OK) {
        return EINVAL;
    }

    if (NULL != retval) {
        *retval = (VOID *)result;
    }

    return 0;
}

int pthread_setschedprio(pthread_t thread, int prio)
{
    if (LOS_TaskPriSet((UINT32)thread, (UINT16)prio) != LOS_OK) {
        return EINVAL;
    }

    return 0;
}

int pthread_detach(pthread_t thread)
{
    UINT32 ret;

    ret = LOS_TaskDetach((UINT32)thread);
    if (ret == LOS_ERRNO_TSK_NOT_JOIN) {
        return ESRCH;
    } else if (ret != LOS_OK) {
        return EINVAL;
    }

    return 0;
}

#include "los_swtmr.h"
static struct timespec g_accDeltaFromSet;
static bool ValidTimeSpec(const struct timespec *tp)
{
    /* Fail a NULL pointer */
    if (tp == NULL) {
        return false;
    }

    /* Fail illegal nanosecond values */
    if ((tp->tv_nsec < 0) || (tp->tv_nsec >= OS_SYS_NS_PER_SECOND) || (tp->tv_sec < 0)) {
        return false;
    }

    return true;
}
int timer_create(clockid_t clockID, struct sigevent *restrict evp, timer_t *restrict timerID)
{
    UINT32 ret;
    UINT32 swtmrID;

    if (!timerID || (clockID != CLOCK_REALTIME) || !evp) {
        errno = EINVAL;
        return -1;
    }

    if ((evp->sigev_notify != SIGEV_THREAD) || evp->sigev_notify_attributes) {
        errno = ENOTSUP;
        return -1;
    }

    ret = LOS_SwtmrCreate(1, LOS_SWTMR_MODE_ONCE, (SWTMR_PROC_FUNC)evp->sigev_notify_function,
                          &swtmrID, (UINT32)(UINTPTR)evp->sigev_value.sival_ptr
#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
                          , OS_SWTMR_ROUSES_IGNORE, OS_SWTMR_ALIGN_INSENSITIVE
#endif
    );
    if (ret != LOS_OK) {
        errno = (ret == LOS_ERRNO_SWTMR_MAXSIZE) ? EAGAIN : EINVAL;
        return -1;
    }

    *timerID = (timer_t)(UINTPTR)swtmrID;
    return 0;
}

int timer_delete(timer_t timerID)
{
    UINT32 swtmrID = (UINT32)(UINTPTR)timerID;
    if (LOS_SwtmrDelete(swtmrID) != LOS_OK) {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

static UINT32 OsTimeSpec2Tick(const struct timespec *tp)
{
    UINT64 tick, ns;

    ns = (UINT64)tp->tv_sec * OS_SYS_NS_PER_SECOND + tp->tv_nsec;
    /* Round up for ticks */
    tick = (ns * LOSCFG_BASE_CORE_TICK_PER_SECOND + (OS_SYS_NS_PER_SECOND - 1)) / OS_SYS_NS_PER_SECOND;
    if (tick > LOS_WAIT_FOREVER) {
        tick = LOS_WAIT_FOREVER;
    }
    return (UINT32)tick;
}

static VOID OsTick2TimeSpec(struct timespec *tp, UINT32 tick)
{
    UINT64 ns = ((UINT64)tick * OS_SYS_NS_PER_SECOND) / LOSCFG_BASE_CORE_TICK_PER_SECOND;
    tp->tv_sec = (time_t)(ns / OS_SYS_NS_PER_SECOND);
    tp->tv_nsec = (long)(ns % OS_SYS_NS_PER_SECOND);
}
static VOID OsGetHwTime(struct timespec *hwTime)
{
    UINT64 cycle = LOS_SysCycleGet();
    UINT64 nowNsec = (cycle / g_sysClock) * OS_SYS_NS_PER_SECOND +
                     (cycle % g_sysClock) * OS_SYS_NS_PER_SECOND / g_sysClock;

    hwTime->tv_sec = nowNsec / OS_SYS_NS_PER_SECOND;
    hwTime->tv_nsec = nowNsec % OS_SYS_NS_PER_SECOND;
}
static VOID OsGetRealTime(struct timespec *realTime)
{
    UINT32 intSave;
    struct timespec hwTime = {0};
    OsGetHwTime(&hwTime);
    intSave = LOS_IntLock();
    realTime->tv_nsec = hwTime.tv_nsec + g_accDeltaFromSet.tv_nsec;
    realTime->tv_sec = hwTime.tv_sec + g_accDeltaFromSet.tv_sec + (realTime->tv_nsec >= OS_SYS_NS_PER_SECOND);
    realTime->tv_nsec %= OS_SYS_NS_PER_SECOND;
    LOS_IntRestore(intSave);
}

static VOID OsSetRealTime(const struct timespec *realTime)
{
    UINT32 intSave;
    struct timespec hwTime = {0};
    OsGetHwTime(&hwTime);
    intSave = LOS_IntLock();
    g_accDeltaFromSet.tv_nsec = realTime->tv_nsec - hwTime.tv_nsec;
    g_accDeltaFromSet.tv_sec = realTime->tv_sec - hwTime.tv_sec - (g_accDeltaFromSet.tv_nsec < 0);
    g_accDeltaFromSet.tv_nsec = (g_accDeltaFromSet.tv_nsec + OS_SYS_NS_PER_SECOND) % OS_SYS_NS_PER_SECOND;
    LOS_IntRestore(intSave);
}
int timer_settime(timer_t timerID, int flags,
                  const struct itimerspec *restrict value,
                  struct itimerspec *restrict oldValue)
{
    UINT32 intSave;
    UINT32 swtmrID = (UINT32)(UINTPTR)timerID;
    SWTMR_CTRL_S *swtmr = NULL;
    UINT32 interval, expiry, ret;

    if (flags != 0) {
        /* flags not supported currently */
        errno = ENOTSUP;
        return -1;
    }

    if (value == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (!ValidTimeSpec(&value->it_value) || !ValidTimeSpec(&value->it_interval)) {
        errno = EINVAL;
        return -1;
    }

    expiry = OsTimeSpec2Tick(&value->it_value);
    interval = OsTimeSpec2Tick(&value->it_interval);

    /* if specified interval, it must be same with expiry due to the limitation of liteos-m */
    if (interval && interval != expiry) {
        errno = ENOTSUP;
        return -1;
    }

    if (oldValue) {
        (VOID)timer_gettime(timerID, oldValue);
    }

    ret = LOS_SwtmrStop(swtmrID);
    if ((ret != LOS_OK) && (ret != LOS_ERRNO_SWTMR_NOT_STARTED)) {
        errno = EINVAL;
        return -1;
    }

    intSave = LOS_IntLock();
    swtmr = OS_SWT_FROM_SID(swtmrID);
    swtmr->ucMode = (interval ? LOS_SWTMR_MODE_PERIOD : LOS_SWTMR_MODE_NO_SELFDELETE);
    swtmr->uwInterval = (interval ? interval : expiry);

    swtmr->ucOverrun = 0;
    LOS_IntRestore(intSave);

    if ((value->it_value.tv_sec == 0) && (value->it_value.tv_nsec == 0)) {
        /*
         * 1) when expiry is 0, means timer should be stopped.
         * 2) If timer is ticking, stopping timer is already done before.
         * 3) If timer is created but not ticking, return 0 as well.
         */
        return 0;
    }

    if (LOS_SwtmrStart(swtmr->usTimerID) != LOS_OK) {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

int timer_gettime(timer_t timerID, struct itimerspec *value)
{
    UINT32 tick = 0;
    SWTMR_CTRL_S *swtmr = NULL;
    UINT32 swtmrID = (UINT32)(UINTPTR)timerID;
    UINT32 ret;

    if (value == NULL) {
        errno = EINVAL;
        return -1;
    }

    swtmr = OS_SWT_FROM_SID(swtmrID);

    /* get expire time */
    ret = LOS_SwtmrTimeGet(swtmr->usTimerID, &tick);
    if ((ret != LOS_OK) && (ret != LOS_ERRNO_SWTMR_NOT_STARTED)) {
        errno = EINVAL;
        return -1;
    }
    if (ret == LOS_ERRNO_SWTMR_NOT_STARTED) {
        tick = 0;
    }
    OsTick2TimeSpec(&value->it_value, tick);
    OsTick2TimeSpec(&value->it_interval, (swtmr->ucMode == LOS_SWTMR_MODE_ONCE) ? 0 : swtmr->uwInterval);
    return 0;
}

#include "kal/libc/newlib/porting/include/semaphore.h"
#include "los_sem.h"

#define _SEM_MAGIC 0xEBCFDEA1

#define s_magic __val[0]
#define s_handle __val[1]
static INT32 OsGetTickTimeFromNow(const struct timespec *ts, clockid_t clockId, UINT64 *absTicks)
{
    struct timespec tp;
    UINT64 nseconds;
    UINT64 currTime;
    const UINT32 nsPerTick = OS_SYS_NS_PER_SECOND / LOSCFG_BASE_CORE_TICK_PER_SECOND;

    if (!ValidTimeSpec(ts)) {
        return EINVAL;
    }

    clock_gettime(clockId, &tp);
    currTime = (UINT64)tp.tv_sec * OS_SYS_NS_PER_SECOND + tp.tv_nsec;
    nseconds = (UINT64)ts->tv_sec * OS_SYS_NS_PER_SECOND + ts->tv_nsec;
    if (currTime >= nseconds) {
        return ETIMEDOUT;
    }
    *absTicks = ((nseconds - currTime) + nsPerTick - 1) / nsPerTick + 1;

    return 0;
}
static inline int MapError(UINT32 err)
{
    switch (err) {
        case LOS_OK:
            return 0;
        case LOS_ERRNO_SEM_INVALID:
        case LOS_ERRNO_SEM_UNAVAILABLE:
            return EINVAL;
        case LOS_ERRNO_SEM_ALL_BUSY:
            return ENOSPC;
        case LOS_ERRNO_SEM_OVERFLOW:
            return ENOMEM;
        case LOS_ERRNO_SEM_PENDED:
            return EBUSY;
        case LOS_ERRNO_SEM_PEND_IN_LOCK:
            return EPERM;
        case LOS_ERRNO_SEM_PEND_INTERR:
            return EINTR;
        case LOS_ERRNO_SEM_TIMEOUT:
            return ETIMEDOUT;
        default:
            return EINVAL;
    }
}

int sem_init(sem_t *sem, int shared, unsigned int value)
{
    UINT32 semHandle = 0;
    UINT32 ret;

    (VOID)shared;
    if ((sem == NULL) || (value >= OS_SEM_COUNTING_MAX_COUNT)) {
        errno = EINVAL;
        return -1;
    }

    ret = LOS_SemCreate(value, &semHandle);
    if (ret != LOS_OK) {
        errno = MapError(ret);
        return -1;
    }

    sem->s_magic = (INT32)_SEM_MAGIC;
    sem->s_handle = (INT32)semHandle;

    return 0;
}

int sem_destroy(sem_t *sem)
{
    UINT32 ret;

    if ((sem == NULL) || (sem->s_magic != (INT32)_SEM_MAGIC)) {
        errno = EINVAL;
        return -1;
    }

    ret = LOS_SemDelete((UINT32)sem->s_handle);
    if (ret != LOS_OK) {
        errno = MapError(ret);
        return -1;
    }

    return 0;
}

int sem_wait(sem_t *sem)
{
    UINT32 ret;

    if ((sem == NULL) || (sem->s_magic != (INT32)_SEM_MAGIC)) {
        errno = EINVAL;
        return -1;
    }

    ret = LOS_SemPend((UINT32)sem->s_handle, LOS_WAIT_FOREVER);
    if (ret != LOS_OK) {
        errno = MapError(ret);
        return -1;
    }

    return 0;
}

int sem_post(sem_t *sem)
{
    UINT32 ret;

    if ((sem == NULL) || (sem->s_magic != (INT32)_SEM_MAGIC)) {
        errno = EINVAL;
        return -1;
    }

    ret = LOS_SemPost((UINT32)sem->s_handle);
    if (ret != LOS_OK) {
        errno = MapError(ret);
        return -1;
    }

    return 0;
}

int sem_trywait(sem_t *sem)
{
    UINT32 ret;

    if ((sem == NULL) || (sem->s_magic != (INT32)_SEM_MAGIC)) {
        errno = EINVAL;
        return -1;
    }

    ret = LOS_SemPend((UINT32)sem->s_handle, LOS_NO_WAIT);
    if (ret != LOS_OK) {
        errno = MapError(ret);
        return -1;
    }

    return 0;
}

int sem_timedwait(sem_t *sem, const struct timespec *timeout)
{
    UINT32 ret;
    UINT64 tickCnt;

    if ((sem == NULL) || (sem->s_magic != (INT32)_SEM_MAGIC)) {
        errno = EINVAL;
        return -1;
    }

    ret = OsGetTickTimeFromNow(timeout, CLOCK_REALTIME, &tickCnt);
    if (ret != 0) {
        errno = (INT32)ret;
        return -1;
    }

    if (tickCnt > LOS_WAIT_FOREVER) {
        tickCnt = LOS_WAIT_FOREVER;
    }

    ret = LOS_SemPend((UINT32)sem->s_handle, (UINT32)tickCnt);
    if (ret != LOS_OK) {
        errno = MapError(ret);
        return -1;
    }

    return 0;
}

int sem_getvalue(sem_t *sem, int *currVal)
{
    UINT32 ret;

    if ((sem == NULL) || (sem->s_magic != (INT32)_SEM_MAGIC)|| (currVal == NULL)) {
        errno = EINVAL;
        return -1;
    }

    ret = LOS_SemGetValue(sem->s_handle, currVal);
    if (ret) {
        errno = EINVAL;
        return -1;
    }

    return LOS_OK;
}
#endif
