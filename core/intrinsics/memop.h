#pragma once

#include "stdint.h"

#ifdef __cplusplus
extern "C"{
#endif

// from rt-thread
// SPDX-License-Identifier: Apache-2.0

/**
 * This function will set the content of memory to specified value
 *
 * @param s the address of source memory
 * @param c the value shall be set in content
 * @param count the copied length
 *
 * @return the address of source memory
 */
__attribute__((used, externally_visible, optimize( "-Ofast" )))
void *memset(void *s, int c, unsigned int count);

// from rt-thread
// SPDX-License-Identifier: Apache-2.0

/**
 * This function will copy memory content from source address to destination
 * address.
 *
 * @param dst the address of destination memory
 * @param src  the address of source memory
 * @param count the copied length
 *
 * @return the address of destination memory
 */
__attribute__((used, externally_visible, optimize( "-Ofast" )))
void *memcpy(void * __restrict dst, const void * __restrict src, unsigned int count);


#ifdef __cplusplus
}
#endif