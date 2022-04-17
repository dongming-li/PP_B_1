/*
 *  An 32-bit implementation of the XTEA algorithm
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  Code was taken from the mbedtls project and modified/refractored
 *  to work with the VFS. Code is originally licensed under the Apache 2.0
 *  license which is not a copyleft license
 */

#pragma once

#include <ntddk.h>

#define XTEA_ENCRYPT 0xCAFEBABE
#define XTEA_DECRYPT 0xBAADF00D

typedef struct {
	unsigned int k[4];
} XTEA_CONTEXT, *PXTEA_CONTEXT;

VOID 
XTEAInit(
		PXTEA_CONTEXT ctx, 
		char key[16]
		);

VOID 
XTEACryptBlock(
		PXTEA_CONTEXT ctx, 
		int mode,
		char input[8], 
		char output[8]
		);
