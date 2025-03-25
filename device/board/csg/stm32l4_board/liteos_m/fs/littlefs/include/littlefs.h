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

#ifndef _LITTLEFS_H_
#define _LITTLEFS_H_

#include "lfs.h"
#include "w25qxx.h"

#ifdef __cplusplus
extern "C" {
#endif

int LittlefsRead(int partition, UINT32 *offset, void *buf, UINT32 size);
int LittlefsProg(int partition, UINT32 *offset, const void *buf, UINT32 size);
int LittlefsErase(int partition, UINT32 offset, UINT32 size);

#ifdef __cplusplus
}
#endif

#endif /* _LITTLEFS_H_ */

