#include "softbus_adapter_json.h"

#include "cJSON.h"
#include "securec.h"
#include "softbus_adapter_mem.h"

JsonObj *JSON_CreateObject(void)
{
    JsonObj *obj = (JsonObj *)SoftBusMalloc(sizeof(JsonObj));
    if (obj == NULL) {
        printf("malloc JsonObj fail");
        return NULL;
    }

    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        printf("cJSON_CreateObject fail");
        SoftBusFree(obj);
        return NULL;
    }

    obj->context = (void *)json;
    return obj;
}

void JSON_Delete(JsonObj *obj)
{
    if (obj == NULL) {
        return;
    }
    if (obj->context != NULL) {
        cJSON *json = (cJSON *)obj->context;
        cJSON_Delete(json);
        obj->context = NULL;
    }
    SoftBusFree(obj);
}

void JSON_Free(void *obj)
{
    if (obj != NULL) {
        SoftBusFree(obj);
    }
}

char *JSON_PrintUnformatted(const JsonObj *obj)
{
    if (obj == NULL) {
        printf("invalid param");
        return NULL;
    }

    cJSON *json = (cJSON *)obj->context;
    if (json == NULL) {
        return NULL;
    }

    char *jsonString = cJSON_PrintUnformatted(json);
    if (jsonString == NULL) {
        printf("cJSON_PrintUnformatted fail");
        return NULL;
    }

    return jsonString;
}

JsonObj *JSON_Parse(const char *str, uint32_t len)
{
    JsonObj *obj = JSON_CreateObject();
    if (obj == NULL) {
        printf("create json object fail");
        return NULL;
    }

    cJSON *json = (cJSON *)obj->context;
    if (json == NULL) {
        JSON_Delete(obj);
        printf("cast json fail");
        return NULL;
    }

    json = cJSON_ParseWithLength(str, len);
    if (json == NULL) {
        JSON_Delete(obj);
        printf("parse json fail");
        return NULL;
    }

    obj->context = (void *)json;
    return obj;
}

bool JSON_AddBoolToObject(JsonObj *obj, const char *key, bool value)
{
    if (obj == NULL || key == NULL) {
        printf("invalid param");
        return false;
    }

    cJSON *json = (cJSON *)obj->context;
    if (json == NULL) {
        return false;
    }

    cJSON *item = cJSON_CreateBool(value);
    if (item == NULL) {
        return false;
    }

    cJSON_AddItemToObject(json, key, item);
    return true;
}

bool JSON_GetBoolFromOject(const JsonObj *obj, const char *key, bool *value)
{
    if (obj == NULL || key == NULL || value == NULL) {
        printf("invalid param");
        return false;
    }

    cJSON *json = (cJSON *)obj->context;
    if (json == NULL) {
        return false;
    }

    cJSON *item = cJSON_GetObjectItem(json, key);
    if (!cJSON_IsBool(item)) {
        printf("Cannot find or invalid [%{public}s]", key);
        return false;
    }

    *value = cJSON_IsTrue(item);
    return true;
}

bool JSON_AddIntToObject(JsonObj *obj, const char *key, int value)
{
    if (obj == NULL || key == NULL) {
        printf("invalid param");
        return false;
    }

    cJSON *json = (cJSON *)obj->context;
    if (json == NULL) {
        return false;
    }

    cJSON *item = cJSON_CreateNumber(value);
    if (item == NULL) {
        return false;
    }

    cJSON_AddItemToObject(json, key, item);
    return true;
}

bool JSON_AddInt16ToObject(JsonObj *obj, const char *key, int16_t value)
{
    return JSON_AddIntToObject(obj, key, value);
}

bool JSON_AddInt32ToObject(JsonObj *obj, const char *key, int32_t value)
{
    return JSON_AddIntToObject(obj, key, value);
}

bool JSON_AddInt64ToObject(JsonObj *obj, const char *key, int64_t value)
{
    return JSON_AddIntToObject(obj, key, value);
}

bool JSON_GetIntFromObject(const JsonObj *obj, const char *key, int *value)
{
    if (obj == NULL || key == NULL || value == NULL) {
        printf("invalid param");
        return false;
    }

    cJSON *json = (cJSON *)obj->context;
    if (json == NULL) {
        return false;
    }

    cJSON *item = cJSON_GetObjectItem(json, key);
    if (!cJSON_IsNumber(item)) {
        printf("Cannot find or invalid [%{public}s]", key);
        return false;
    }

    *value = item->valueint;
    return true;
}

bool JSON_GetInt16FromOject(const JsonObj *obj, const char *key, int16_t *value)
{
    if (value == NULL) {
        printf("invalid param");
        return false;
    }
    return JSON_GetIntFromObject(obj, key, *value);
}

bool JSON_GetInt32FromOject(const JsonObj *obj, const char *key, int32_t *value)
{
    if (value == NULL) {
        printf("invalid param");
        return false;
    }
    return JSON_GetIntFromObject(obj, key, *value);
}

bool JSON_GetInt64FromOject(const JsonObj *obj, const char *key, int64_t *value)
{
    return JSON_GetIntFromObject(obj, key, value);
}

bool JSON_AddStringToObject(JsonObj *obj, const char *key, const char *value)
{
    if (obj == NULL || key == NULL || value == NULL) {
        printf("invalid param");
        return false;
    }

    cJSON *json = (cJSON *)obj->context;
    if (json == NULL) {
        return false;
    }

    cJSON *item = cJSON_CreateString(value);
    if (item == NULL) {
        return false;
    }

    cJSON_AddItemToObject(json, key, item);
    return true;
}

bool JSON_GetStringFromOject(const JsonObj *obj, const char *key, char *value, uint32_t size)
{
    if (obj == NULL || key == NULL || value == NULL) {
        printf("invalid param");
        return false;
    }

    cJSON *json = (cJSON *)obj->context;
    if (json == NULL) {
        return false;
    }

    cJSON *item = cJSON_GetObjectItem(json, key);
    if (!cJSON_IsString(item)) {
        printf("Cannot find or invalid [%{public}s]", key);
        return false;
    }

    if (strcpy_s(value, size, item->valuestring) != EOK) {
        printf("strcpy [%{public}s] value err, size=%{public}u, value=%{public}s",
                  key, size, item->valuestring);
        return false;
    }
    return true;
}

bool JSON_AddStringArrayToObject(JsonObj *obj, const char *key, const char **value, int32_t len)
{
    if (value == NULL || obj == NULL || key == NULL || len == 0) {
        printf("input invalid");
        return false;
    }

    cJSON *json = (cJSON *)obj->context;
    if (json == NULL) {
        return false;
    }

    cJSON *array = cJSON_CreateArray();
    if (array == NULL) {
        return false;
    }

    for (int32_t i = 0; i < len; i++) {
        cJSON *item = cJSON_CreateString(value[i]);
        if (item == NULL) {
            cJSON_Delete(array);
            return false;
        }
        cJSON_AddItemToArray(array, item);
    }

    cJSON_AddItemToObject(json, key, array);
    return true;
}

bool JSON_GetStringArrayFromObject(const JsonObj *obj, const char *key, char **value, int32_t *len)
{
    if (value == NULL || obj == NULL || key == NULL || len == NULL || *len <= 0) {
        printf("input invalid");
        return false;
    }

    cJSON *json = (cJSON *)obj->context;
    if (json == NULL) {
        return false;
    }

    cJSON *array = cJSON_GetObjectItem(json, key);
    if (!cJSON_IsArray(array)) {
        printf("Cannot find or invalid [%{public}s]", key);
        return false;
    }

    int32_t arraySize = cJSON_GetArraySize(array);
    if (*len < arraySize) {
        printf("item size invalid, size=%d.", arraySize);
        return false;
    }

    for (int32_t i = 0; i < arraySize; i++) {
        cJSON *item = cJSON_GetArrayItem(array, i);
        if (!cJSON_IsString(item)) {
            printf("Invalid item at index [%{public}d]", i);
            return false;
        }
        const char *itemString = item->valuestring;
        uint32_t len = strlen(itemString) + 1;
        value[i] = (char *)SoftBusCalloc(len);
        if (value[i] == NULL) {
            return false;
        }
        if (strcpy_s(value[i], len, itemString) != EOK) {
            printf("strcpy [%{public}s] value err, value=%{public}s", key, itemString);
            return false;
        }
    }

    *len = arraySize;
    return true;
}
