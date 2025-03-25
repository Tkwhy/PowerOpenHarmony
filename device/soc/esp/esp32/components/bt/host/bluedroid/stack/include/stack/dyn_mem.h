/******************************************************************************
 *
 *  Copyright (C) 2002-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/
#ifndef DYN_MEM_H
#define DYN_MEM_H

#include "common/bluedroid_user_config.h"

#if UC_BT_BLE_DYNAMIC_ENV_MEMORY
#define BTU_DYNAMIC_MEMORY         1
#define BTM_DYNAMIC_MEMORY         1
#define L2C_DYNAMIC_MEMORY         1
#define GATT_DYNAMIC_MEMORY        1
#define SMP_DYNAMIC_MEMORY         1
#define BTA_DYNAMIC_MEMORY         1
#define SDP_DYNAMIC_MEMORY         1
#define GAP_DYNAMIC_MEMORY         1
#define RFC_DYNAMIC_MEMORY         1
#define TCS_DYNAMIC_MEMORY         1
#define BNEP_DYNAMIC_MEMORY        1
#define AVDT_DYNAMIC_MEMORY        1
#define AVCT_DYNAMIC_MEMORY        1
#define MCA_DYNAMIC_MEMORY         1
#define A2D_DYNAMIC_MEMORY         1
#define VDP_DYNAMIC_MEMORY         1
#define AVRC_DYNAMIC_MEMORY        1
#define BIP_DYNAMIC_MEMORY         1
#define BPP_DYNAMIC_MEMORY         1
#define CTP_DYNAMIC_MEMORY         1
#define FTP_DYNAMIC_MEMORY         1
#define HCRP_DYNAMIC_MEMORY        1
#define HFP_DYNAMIC_MEMORY         1
#define HID_DYNAMIC_MEMORY         1
#define HSP2_DYNAMIC_MEMORY        1
#define ICP_DYNAMIC_MEMORY         1
#define OPP_DYNAMIC_MEMORY         1
#define PAN_DYNAMIC_MEMORY         1
#define SPP_DYNAMIC_MEMORY         1
#define SLIP_DYNAMIC_MEMORY        1
#define LLCP_DYNAMIC_MEMORY        1
#define BTC_SBC_DEC_DYNAMIC_MEMORY 1

#else  /* #if UC_BT_BLE_DYNAMIC_ENV_MEMORY */
#define BTU_DYNAMIC_MEMORY         0
#define BTM_DYNAMIC_MEMORY         0
#define L2C_DYNAMIC_MEMORY         0
#define GATT_DYNAMIC_MEMORY        0
#define SMP_DYNAMIC_MEMORY         0
#define BTA_DYNAMIC_MEMORY         0
#define SDP_DYNAMIC_MEMORY         0
#define GAP_DYNAMIC_MEMORY         0
#define RFC_DYNAMIC_MEMORY         0
#define TCS_DYNAMIC_MEMORY         0
#define BNEP_DYNAMIC_MEMORY        0
#define AVDT_DYNAMIC_MEMORY        0
#define AVCT_DYNAMIC_MEMORY        0
#define MCA_DYNAMIC_MEMORY         0
#define A2D_DYNAMIC_MEMORY         0
#define VDP_DYNAMIC_MEMORY         0
#define AVRC_DYNAMIC_MEMORY        0
#define BIP_DYNAMIC_MEMORY         0
#define BPP_DYNAMIC_MEMORY         0
#define CTP_DYNAMIC_MEMORY         0
#define FTP_DYNAMIC_MEMORY         0
#define HCRP_DYNAMIC_MEMORY        0
#define HFP_DYNAMIC_MEMORY         0
#define HID_DYNAMIC_MEMORY         0
#define HSP2_DYNAMIC_MEMORY        0
#define ICP_DYNAMIC_MEMORY         0
#define OPP_DYNAMIC_MEMORY         0
#define PAN_DYNAMIC_MEMORY         0
#define SPP_DYNAMIC_MEMORY         0
#define SLIP_DYNAMIC_MEMORY        0
#define LLCP_DYNAMIC_MEMORY        0
#define BTC_SBC_DEC_DYNAMIC_MEMORY 0

#endif /* #if UC_BT_BLE_DYNAMIC_ENV_MEMORY */


/****************************************************************************
** Define memory usage for each CORE component (if not defined in bdroid_buildcfg.h)
**  The default for each component is to use static memory allocations.
*/
#ifndef BTU_DYNAMIC_MEMORY
#define BTU_DYNAMIC_MEMORY  0
#endif

#ifndef BTM_DYNAMIC_MEMORY
#define BTM_DYNAMIC_MEMORY  0
#endif

#ifndef SDP_DYNAMIC_MEMORY
#define SDP_DYNAMIC_MEMORY  0
#endif

#ifndef GAP_DYNAMIC_MEMORY
#define GAP_DYNAMIC_MEMORY  0
#endif

#ifndef L2C_DYNAMIC_MEMORY
#define L2C_DYNAMIC_MEMORY  0
#endif

#ifndef RFC_DYNAMIC_MEMORY
#define RFC_DYNAMIC_MEMORY  0
#endif

#ifndef TCS_DYNAMIC_MEMORY
#define TCS_DYNAMIC_MEMORY  0
#endif

#ifndef BNEP_DYNAMIC_MEMORY
#define BNEP_DYNAMIC_MEMORY 0
#endif

#ifndef AVDT_DYNAMIC_MEMORY
#define AVDT_DYNAMIC_MEMORY 0
#endif

#ifndef AVCT_DYNAMIC_MEMORY
#define AVCT_DYNAMIC_MEMORY 0
#endif

#ifndef MCA_DYNAMIC_MEMORY
#define MCA_DYNAMIC_MEMORY 0
#endif

#ifndef GATT_DYNAMIC_MEMORY
#define GATT_DYNAMIC_MEMORY  0
#endif

#ifndef SMP_DYNAMIC_MEMORY
#define SMP_DYNAMIC_MEMORY  0
#endif

/****************************************************************************
** Define memory usage for each PROFILE component (if not defined in bdroid_buildcfg.h)
**  The default for each component is to use static memory allocations.
*/
#ifndef A2D_DYNAMIC_MEMORY
#define A2D_DYNAMIC_MEMORY  0
#endif

#ifndef VDP_DYNAMIC_MEMORY
#define VDP_DYNAMIC_MEMORY  0
#endif

#ifndef AVRC_DYNAMIC_MEMORY
#define AVRC_DYNAMIC_MEMORY 0
#endif

#ifndef BIP_DYNAMIC_MEMORY
#define BIP_DYNAMIC_MEMORY  0
#endif

#ifndef BPP_DYNAMIC_MEMORY
#define BPP_DYNAMIC_MEMORY  0
#endif

#ifndef CTP_DYNAMIC_MEMORY
#define CTP_DYNAMIC_MEMORY  0
#endif

#ifndef FTP_DYNAMIC_MEMORY
#define FTP_DYNAMIC_MEMORY  0
#endif

#ifndef HCRP_DYNAMIC_MEMORY
#define HCRP_DYNAMIC_MEMORY 0
#endif

#ifndef HFP_DYNAMIC_MEMORY
#define HFP_DYNAMIC_MEMORY  0
#endif

#ifndef HID_DYNAMIC_MEMORY
#define HID_DYNAMIC_MEMORY  0
#endif

#ifndef HSP2_DYNAMIC_MEMORY
#define HSP2_DYNAMIC_MEMORY 0
#endif

#ifndef ICP_DYNAMIC_MEMORY
#define ICP_DYNAMIC_MEMORY  0
#endif

#ifndef OPP_DYNAMIC_MEMORY
#define OPP_DYNAMIC_MEMORY  0
#endif

#ifndef PAN_DYNAMIC_MEMORY
#define PAN_DYNAMIC_MEMORY  0
#endif

#ifndef SPP_DYNAMIC_MEMORY
#define SPP_DYNAMIC_MEMORY  0
#endif

#ifndef SLIP_DYNAMIC_MEMORY
#define SLIP_DYNAMIC_MEMORY  0
#endif

#ifndef LLCP_DYNAMIC_MEMORY
#define LLCP_DYNAMIC_MEMORY  0
#endif

/****************************************************************************
** Define memory usage for BTA and BTC (if not defined in bdroid_buildcfg.h)
**  The default for each component is to use static memory allocations.
*/
#ifndef BTA_DYNAMIC_MEMORY
#define BTA_DYNAMIC_MEMORY 0
#endif

#ifndef BTC_DYNAMIC_MEMORY
#define BTC_DYNAMIC_MEMORY 0
#endif

#endif  /* #ifdef DYN_MEM_H */
