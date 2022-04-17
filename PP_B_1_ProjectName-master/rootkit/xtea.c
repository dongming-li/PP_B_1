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

#include <ntddk.h>

#include "xtea.h"

#define GET_UINT32_BE(n,b,i)                         \
{                                                    \
	(n) = ( (unsigned int) (b)[(i)    ] << 24 )         \
	    | ( (unsigned int) (b)[(i) + 1] << 16 )         \
	    | ( (unsigned int) (b)[(i) + 2] <<  8 )         \
	    | ( (unsigned int) (b)[(i) + 3]       );        \
}

#define PUT_UINT32_BE(n,b,i)                        \
{                                                   \
	(b)[(i)    ] = (UCHAR) ( (n) >> 24 );       \
	(b)[(i) + 1] = (UCHAR) ( (n) >> 16 );       \
	(b)[(i) + 2] = (UCHAR) ( (n) >>  8 );       \
	(b)[(i) + 3] = (UCHAR) ( (n)       );       \
}

VOID 
XTEAInit(
		PXTEA_CONTEXT ctx, 
		char key[16]
		)

/*++

  Routine Description:
  
  	Initializes the state of the XTEA cipher

  Arguments:

  	ctx - XTEA cipher context

	key - Encryption key to be used

  Return Value:
  
  	VOID (no return)

--*/

{
	unsigned int i = 0;

	RtlZeroMemory(ctx, sizeof(XTEA_CONTEXT));

	while(i < 4) {
		GET_UINT32_BE(ctx->k[i], key, i << 2 );
		i++;
	}
}

VOID 
XTEACryptBlock(
		PXTEA_CONTEXT ctx, 
		int mode,
		char input[8], 
		char output[8]
		)

/*++

  Routine Description:
  
  	Encrypts a given plaintext block using the XTEA cipher with a specified symmetric key,
	cipher operates in ECB mode

  Arguments:

  	ctx    - XTEA cipher context 

	mode   - 

	input  - Plaintext/ciphertext block to be encrypted/decrypted

	output - Plaintext/ciphertext block that was decrypted/encrypted	

  Return Value:
  
  	VOID (no return)

--*/

{
	unsigned int *k, v0, v1, i;
	unsigned int delta, sum;

	k = ctx->k;

	GET_UINT32_BE(v0, input, 0);
	GET_UINT32_BE(v1, input, 4);

	if(mode == XTEA_ENCRYPT) {
		sum = 0; 
		delta = 0x9E3779B9;

		i = 0;
		while(i < 32) {
			v0  += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + k[sum & 3]);
			sum += delta;
			v1  += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + k[(sum>>11) & 3]);
			i++;
		}

	} else {
		delta = 0x9E3779B9, sum = delta * 32;

		i = 0;
		while(i < 32) {
			v1  -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + k[(sum >> 11) & 3]);
			sum -= delta;
			v0  -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + k[sum & 3]);
			i++;
		}
	}

	PUT_UINT32_BE(v0, output, 0);
	PUT_UINT32_BE(v1, output, 4);
}
