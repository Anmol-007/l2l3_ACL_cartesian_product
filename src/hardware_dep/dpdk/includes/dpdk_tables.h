// Copyright 2016 Eotvos Lorand University, Budapest, Hungary
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef DPDK_TABLES_H
#define DPDK_TABLES_H

typedef struct extended_table_s {
    void*     rte_table;
    uint8_t   size;
    uint8_t** content;
} extended_table_t;

//=============================================================================
// Table size limits

#define VLAN_INGRESS            64              // vid = 16, mac = 128, ip = 1024 //8M entries
#define MAC_LEARNING            128
#define ROUTABLE                2048
#define ROUTING                 1024
#define SWITCHING               2048
#define ACL                     4 //Anmol
#define VLAN_EGRESS             64


#define KEY1			16
#define KEY2			8
#define KEY3			14
#define KEY4 			17
#define KEY5			10


#ifdef RTE_ARCH_X86_64
#define HASH_ENTRIES		8
#else
#define HASH_ENTRIES		1024
#endif
#define LPM_MAX_RULES         1024
#define LPM6_NUMBER_TBL8S (1 << 16)

#define TABLE_MAX 256

#endif
