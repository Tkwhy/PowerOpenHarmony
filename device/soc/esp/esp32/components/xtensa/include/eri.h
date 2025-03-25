// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
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

#ifndef ERI_H
#define ERI_H

#include <stdint.h>

/*
 The ERI is a bus internal to each Xtensa core. It connects, amongst others, to the debug interface, where it
 allows reading/writing the same registers as available over JTAG.
*/


/**
 * @brief  Perform an ERI read
 * @param  addr : ERI register to read from
 *
 * @return Value read
 */
uint32_t eri_read(int addr);


/**
 * @brief  Perform an ERI write
 * @param  addr : ERI register to write to
 * @param  data : Value to write
 *
 * @return Value read
 */
void eri_write(int addr, uint32_t data);


#endif
