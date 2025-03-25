/*!
 * @file       ethernetif.h
 *
 * @brief      Header for ethernetif.c module
 *
 * @version     V1.0.1
 *
 * @date        2022-02-25
 *
 * @attention
 *
 *  Copyright (C) 2021-2022 Geehy Semiconductor
 *
 *  You may not use this file except in compliance with the
 *  GEEHY COPYRIGHT NOTICE (GEEHY SOFTWARE PACKAGE LICENSE).
 *
 *  The program is only for reference, which is distributed in the hope
 *  that it will be usefull and instructional for customers to develop
 *  their software. Unless required by applicable law or agreed to in
 *  writing, the program is distributed on an "AS IS" BASIS, WITHOUT
 *  ANY WARRANTY OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the GEEHY SOFTWARE PACKAGE LICENSE for the governing permissions
 *  and limitations under the License.
 */

#ifndef __ETHERNETIF_H
#define __ETHERNETIF_H

#include "lwip/err.h"
#include "lwip/netif.h"

extern UINT32 s_BinarySem_Handle; // 二值信号量

err_t ethernetif_init(struct netif *netif);
void ethernetif_input(void *pParams);

#endif /** __ETHERNETIF_H */
