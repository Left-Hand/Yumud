#pragma once

#include <cstdint>


// https://github.com/matrixcascade/PainterEngine/blob/master/core/PX_curve25519.h

/* Copyright 2008, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * curve25519-donna: Curve25519 elliptic curve, public key function
 *
 * http://code.google.com/p/curve25519-donna/
 *
 * Adam Langley <agl@imperialviolet.org>
 *
 * Derived from public domain C code by Daniel J. Bernstein <djb@cr.yp.to>
 *
 * More information about curve25519 can be found here
 *   http://cr.yp.to/ecdh.html
 *
 * djb's sample implementation of curve25519 is written in a special assembly
 * language called qhasm and uses the floating point registers.
 *
 * This is, almost, a clean room reimplementation from the curve25519 paper. It
 * uses many of the tricks described therein. Only the crecip function is taken
 * from the sample implementation. 
 
 To generate a private key, generate 32 random bytes and:
 uint8_t mysecret[32];
 mysecret[0] &= 248;
 mysecret[31] &= 127;
 mysecret[31] |= 64;
 To generate the public key, just do:

 static const uint8_t basepoint[32] = {9};
 curve25519_donna(mypublic, mysecret, basepoint);
 To generate a shared key do:

 uint8_t shared_key[32];
 curve25519_donna(shared_key, mysecret, theirpublic);

 
 */

#define COPY_X10(dst, src)  for(size_t i = 0; i < 10; i++) dst[i] = src[i];

namespace ymd::encrypt::curve25519::details{
/* Field element representation:
 *
 * Field elements are written as an array of signed, 64-bit limbs, least
 * significant first. The value of the field element is:
 *   x[0] + 2^26·x[1] + x^51·x[2] + 2^102·x[3] + ...
 *
 * i.e. the limbs are 26, 25, 26, 25, ... bits wide. */

/* Sum two numbers: output += in */
static constexpr void fsum(int64_t *output, const int64_t *in) {
    for (size_t i = 0; i < 10; i += 2) {
        output[0+i] = output[0+i] + in[0+i];
        output[1+i] = output[1+i] + in[1+i];
    }
}

/* Find the difference of two numbers: output = in - output
 * (note the order of the arguments!). */
static constexpr void fdifference(int64_t *output, const int64_t *in) {
    for (size_t i = 0; i < 10; ++i) {
        output[i] = in[i] - output[i];
    }
}

/* Multiply a number by a scalar: output = in * scalar */
static constexpr void fscalar_product(int64_t *output, const int64_t *in, const int64_t scalar) {
    for (size_t i = 0; i < 10; ++i) {
        output[i] = in[i] * scalar;
    }
}

/* Multiply two numbers: output = in2 * in
 *
 * output must be distinct to both inputs. The inputs are reduced coefficient
 * form, the output is not.
 *
 * output[x] <= 14 * the largest product of the input limbs. */
static constexpr void fproduct(int64_t *output, const int64_t *in2, const int64_t *in) {
  output[0] =       ((int64_t) ((int32_t) in2[0])) * ((int32_t) in[0]);
  output[1] =       ((int64_t) ((int32_t) in2[0])) * ((int32_t) in[1]) +
                    ((int64_t) ((int32_t) in2[1])) * ((int32_t) in[0]);
  output[2] =  2 *  ((int64_t) ((int32_t) in2[1])) * ((int32_t) in[1]) +
                    ((int64_t) ((int32_t) in2[0])) * ((int32_t) in[2]) +
                    ((int64_t) ((int32_t) in2[2])) * ((int32_t) in[0]);
  output[3] =       ((int64_t) ((int32_t) in2[1])) * ((int32_t) in[2]) +
                    ((int64_t) ((int32_t) in2[2])) * ((int32_t) in[1]) +
                    ((int64_t) ((int32_t) in2[0])) * ((int32_t) in[3]) +
                    ((int64_t) ((int32_t) in2[3])) * ((int32_t) in[0]);
  output[4] =       ((int64_t) ((int32_t) in2[2])) * ((int32_t) in[2]) +
               2 * (((int64_t) ((int32_t) in2[1])) * ((int32_t) in[3]) +
                    ((int64_t) ((int32_t) in2[3])) * ((int32_t) in[1])) +
                    ((int64_t) ((int32_t) in2[0])) * ((int32_t) in[4]) +
                    ((int64_t) ((int32_t) in2[4])) * ((int32_t) in[0]);
  output[5] =       ((int64_t) ((int32_t) in2[2])) * ((int32_t) in[3]) +
                    ((int64_t) ((int32_t) in2[3])) * ((int32_t) in[2]) +
                    ((int64_t) ((int32_t) in2[1])) * ((int32_t) in[4]) +
                    ((int64_t) ((int32_t) in2[4])) * ((int32_t) in[1]) +
                    ((int64_t) ((int32_t) in2[0])) * ((int32_t) in[5]) +
                    ((int64_t) ((int32_t) in2[5])) * ((int32_t) in[0]);
  output[6] =  2 * (((int64_t) ((int32_t) in2[3])) * ((int32_t) in[3]) +
                    ((int64_t) ((int32_t) in2[1])) * ((int32_t) in[5]) +
                    ((int64_t) ((int32_t) in2[5])) * ((int32_t) in[1])) +
                    ((int64_t) ((int32_t) in2[2])) * ((int32_t) in[4]) +
                    ((int64_t) ((int32_t) in2[4])) * ((int32_t) in[2]) +
                    ((int64_t) ((int32_t) in2[0])) * ((int32_t) in[6]) +
                    ((int64_t) ((int32_t) in2[6])) * ((int32_t) in[0]);
  output[7] =       ((int64_t) ((int32_t) in2[3])) * ((int32_t) in[4]) +
                    ((int64_t) ((int32_t) in2[4])) * ((int32_t) in[3]) +
                    ((int64_t) ((int32_t) in2[2])) * ((int32_t) in[5]) +
                    ((int64_t) ((int32_t) in2[5])) * ((int32_t) in[2]) +
                    ((int64_t) ((int32_t) in2[1])) * ((int32_t) in[6]) +
                    ((int64_t) ((int32_t) in2[6])) * ((int32_t) in[1]) +
                    ((int64_t) ((int32_t) in2[0])) * ((int32_t) in[7]) +
                    ((int64_t) ((int32_t) in2[7])) * ((int32_t) in[0]);
  output[8] =       ((int64_t) ((int32_t) in2[4])) * ((int32_t) in[4]) +
               2 * (((int64_t) ((int32_t) in2[3])) * ((int32_t) in[5]) +
                    ((int64_t) ((int32_t) in2[5])) * ((int32_t) in[3]) +
                    ((int64_t) ((int32_t) in2[1])) * ((int32_t) in[7]) +
                    ((int64_t) ((int32_t) in2[7])) * ((int32_t) in[1])) +
                    ((int64_t) ((int32_t) in2[2])) * ((int32_t) in[6]) +
                    ((int64_t) ((int32_t) in2[6])) * ((int32_t) in[2]) +
                    ((int64_t) ((int32_t) in2[0])) * ((int32_t) in[8]) +
                    ((int64_t) ((int32_t) in2[8])) * ((int32_t) in[0]);
  output[9] =       ((int64_t) ((int32_t) in2[4])) * ((int32_t) in[5]) +
                    ((int64_t) ((int32_t) in2[5])) * ((int32_t) in[4]) +
                    ((int64_t) ((int32_t) in2[3])) * ((int32_t) in[6]) +
                    ((int64_t) ((int32_t) in2[6])) * ((int32_t) in[3]) +
                    ((int64_t) ((int32_t) in2[2])) * ((int32_t) in[7]) +
                    ((int64_t) ((int32_t) in2[7])) * ((int32_t) in[2]) +
                    ((int64_t) ((int32_t) in2[1])) * ((int32_t) in[8]) +
                    ((int64_t) ((int32_t) in2[8])) * ((int32_t) in[1]) +
                    ((int64_t) ((int32_t) in2[0])) * ((int32_t) in[9]) +
                    ((int64_t) ((int32_t) in2[9])) * ((int32_t) in[0]);
  output[10] = 2 * (((int64_t) ((int32_t) in2[5])) * ((int32_t) in[5]) +
                    ((int64_t) ((int32_t) in2[3])) * ((int32_t) in[7]) +
                    ((int64_t) ((int32_t) in2[7])) * ((int32_t) in[3]) +
                    ((int64_t) ((int32_t) in2[1])) * ((int32_t) in[9]) +
                    ((int64_t) ((int32_t) in2[9])) * ((int32_t) in[1])) +
                    ((int64_t) ((int32_t) in2[4])) * ((int32_t) in[6]) +
                    ((int64_t) ((int32_t) in2[6])) * ((int32_t) in[4]) +
                    ((int64_t) ((int32_t) in2[2])) * ((int32_t) in[8]) +
                    ((int64_t) ((int32_t) in2[8])) * ((int32_t) in[2]);
  output[11] =      ((int64_t) ((int32_t) in2[5])) * ((int32_t) in[6]) +
                    ((int64_t) ((int32_t) in2[6])) * ((int32_t) in[5]) +
                    ((int64_t) ((int32_t) in2[4])) * ((int32_t) in[7]) +
                    ((int64_t) ((int32_t) in2[7])) * ((int32_t) in[4]) +
                    ((int64_t) ((int32_t) in2[3])) * ((int32_t) in[8]) +
                    ((int64_t) ((int32_t) in2[8])) * ((int32_t) in[3]) +
                    ((int64_t) ((int32_t) in2[2])) * ((int32_t) in[9]) +
                    ((int64_t) ((int32_t) in2[9])) * ((int32_t) in[2]);
  output[12] =      ((int64_t) ((int32_t) in2[6])) * ((int32_t) in[6]) +
               2 * (((int64_t) ((int32_t) in2[5])) * ((int32_t) in[7]) +
                    ((int64_t) ((int32_t) in2[7])) * ((int32_t) in[5]) +
                    ((int64_t) ((int32_t) in2[3])) * ((int32_t) in[9]) +
                    ((int64_t) ((int32_t) in2[9])) * ((int32_t) in[3])) +
                    ((int64_t) ((int32_t) in2[4])) * ((int32_t) in[8]) +
                    ((int64_t) ((int32_t) in2[8])) * ((int32_t) in[4]);
  output[13] =      ((int64_t) ((int32_t) in2[6])) * ((int32_t) in[7]) +
                    ((int64_t) ((int32_t) in2[7])) * ((int32_t) in[6]) +
                    ((int64_t) ((int32_t) in2[5])) * ((int32_t) in[8]) +
                    ((int64_t) ((int32_t) in2[8])) * ((int32_t) in[5]) +
                    ((int64_t) ((int32_t) in2[4])) * ((int32_t) in[9]) +
                    ((int64_t) ((int32_t) in2[9])) * ((int32_t) in[4]);
  output[14] = 2 * (((int64_t) ((int32_t) in2[7])) * ((int32_t) in[7]) +
                    ((int64_t) ((int32_t) in2[5])) * ((int32_t) in[9]) +
                    ((int64_t) ((int32_t) in2[9])) * ((int32_t) in[5])) +
                    ((int64_t) ((int32_t) in2[6])) * ((int32_t) in[8]) +
                    ((int64_t) ((int32_t) in2[8])) * ((int32_t) in[6]);
  output[15] =      ((int64_t) ((int32_t) in2[7])) * ((int32_t) in[8]) +
                    ((int64_t) ((int32_t) in2[8])) * ((int32_t) in[7]) +
                    ((int64_t) ((int32_t) in2[6])) * ((int32_t) in[9]) +
                    ((int64_t) ((int32_t) in2[9])) * ((int32_t) in[6]);
  output[16] =      ((int64_t) ((int32_t) in2[8])) * ((int32_t) in[8]) +
               2 * (((int64_t) ((int32_t) in2[7])) * ((int32_t) in[9]) +
                    ((int64_t) ((int32_t) in2[9])) * ((int32_t) in[7]));
  output[17] =      ((int64_t) ((int32_t) in2[8])) * ((int32_t) in[9]) +
                    ((int64_t) ((int32_t) in2[9])) * ((int32_t) in[8]);
  output[18] = 2 *  ((int64_t) ((int32_t) in2[9])) * ((int32_t) in[9]);
}

/* Reduce a long form to a short form by taking the input mod 2^255 - 19.
 *
 * On entry: |output[i]| < 14*2^54
 * On exit: |output[0..8]| < 280*2^54 */
static constexpr void freduce_degree(int64_t *output) {
    /* Each of these shifts and adds ends up multiplying the value by 19.
    *
    * For output[0..8], the absolute entry value is < 14*2^54 and we add, at
    * most, 19*14*2^54 thus, on exit, |output[0..8]| < 280*2^54. */
    output[8] += output[18] << 4;
    output[8] += output[18] << 1;
    output[8] += output[18];
    output[7] += output[17] << 4;
    output[7] += output[17] << 1;
    output[7] += output[17];
    output[6] += output[16] << 4;
    output[6] += output[16] << 1;
    output[6] += output[16];
    output[5] += output[15] << 4;
    output[5] += output[15] << 1;
    output[5] += output[15];
    output[4] += output[14] << 4;
    output[4] += output[14] << 1;
    output[4] += output[14];
    output[3] += output[13] << 4;
    output[3] += output[13] << 1;
    output[3] += output[13];
    output[2] += output[12] << 4;
    output[2] += output[12] << 1;
    output[2] += output[12];
    output[1] += output[11] << 4;
    output[1] += output[11] << 1;
    output[1] += output[11];
    output[0] += output[10] << 4;
    output[0] += output[10] << 1;
    output[0] += output[10];
}

#if (-1 & 3) != 3
#error "This code only works on a two's complement system"
#endif

/* return v / 2^26, using only shifts and adds.
 *
 * On entry: v can take any value. */
static constexpr int64_t div_by_2_26(const int64_t v)
{
    /* High word of v; no shift needed. */
    const uint32_t highword = (uint32_t) (((uint64_t) v) >> 32);
    /* Set to all 1s if v was negative; else set to 0s. */
    const int32_t sign = ((int32_t) highword) >> 31;
    /* Set to 0x3ffffff if v was negative; else set to 0. */
    const int32_t roundoff = ((uint32_t) sign) >> 6;
    /* Should return v / (1<<26) */
    return (v + roundoff) >> 26;
}

/* return v / (2^25), using only shifts and adds.
 *
 * On entry: v can take any value. */
static constexpr int64_t div_by_2_25(const int64_t v)
{
    /* High word of v; no shift needed*/
    const uint32_t highword = (uint32_t) (((uint64_t) v) >> 32);
    /* Set to all 1s if v was negative; else set to 0s. */
    const int32_t sign = ((int32_t) highword) >> 31;
    /* Set to 0x1ffffff if v was negative; else set to 0. */
    const int32_t roundoff = ((uint32_t) sign) >> 7;
    /* Should return v / (1<<25) */
    return (v + roundoff) >> 25;
}

/* Reduce all coefficients of the short form input so that |x| < 2^26.
 *
 * On entry: |output[i]| < 280*2^54 */
static constexpr void freduce_coefficients(int64_t *output) {

    output[10] = 0;

    for(size_t i = 0; i < 10; i += 2) {
        int64_t over = div_by_2_26(output[i]);
        /* The entry condition (that |output[i]| < 280*2^54) means that over is, at
        * most, 280*2^28 in the first iteration of this loop. This is added to the
        * next int64_t and we can approximate the resulting bound of that int64_t by
        * 281*2^54. */
        output[i] -= over << 26;
        output[i+1] += over;

        /* For the first iteration, |output[i+1]| < 281*2^54, thus |over| <
        * 281*2^29. When this is added to the next int64_t, the resulting bound can
        * be approximated as 281*2^54.
        *
        * For subsequent iterations of the loop, 281*2^54 remains a conservative
        * bound and no overflow occurs. */
        over = div_by_2_25(output[i+1]);
        output[i+1] -= over << 25;
        output[i+2] += over;
    }
    /* Now |output[10]| < 281*2^29 and all other coefficients are reduced. */
    output[0] += output[10] << 4;
    output[0] += output[10] << 1;
    output[0] += output[10];

    output[10] = 0;

    /* Now output[1..9] are reduced, and |output[0]| < 2^26 + 19*281*2^29
    * So |over| will be no more than 2^16. */
    {
        int64_t over = div_by_2_26(output[0]);
        output[0] -= over << 26;
        output[1] += over;
    }

    /* Now output[0,2..9] are reduced, and |output[1]| < 2^25 + 2^16 < 2^26. The
    * bound on |output[1]| is sufficient to meet our needs. */
}

/* A helpful wrapper around fproduct: output = in * in2.
 *
 * On entry: |in[i]| < 2^27 and |in2[i]| < 2^27.
 *
 * output must be distinct to both inputs. The output is reduced degree
 * (indeed, one need only provide storage for 10 limbs) and |output[i]| < 2^26. */
static void constexpr
fmul(int64_t *output, const int64_t *in, const int64_t *in2) {
    int64_t t[19];
    fproduct(t, in, in2);
    /* |t[i]| < 14*2^54 */
    freduce_degree(t);
    freduce_coefficients(t);
    /* |t[i]| < 2^26 */
    //   PX_memcpy(output, t, sizeof(int64_t) * 10);
        COPY_X10(output, t);
}

/* Square a number: output = in**2
 *
 * output must be distinct from the input. The inputs are reduced coefficient
 * form, the output is not.
 *
 * output[x] <= 14 * the largest product of the input limbs. */
static constexpr void fsquare_inner(int64_t *output, const int64_t *in) {
  output[0] =       ((int64_t) ((int32_t) in[0])) * ((int32_t) in[0]);
  output[1] =  2 *  ((int64_t) ((int32_t) in[0])) * ((int32_t) in[1]);
  output[2] =  2 * (((int64_t) ((int32_t) in[1])) * ((int32_t) in[1]) +
                    ((int64_t) ((int32_t) in[0])) * ((int32_t) in[2]));
  output[3] =  2 * (((int64_t) ((int32_t) in[1])) * ((int32_t) in[2]) +
                    ((int64_t) ((int32_t) in[0])) * ((int32_t) in[3]));
  output[4] =       ((int64_t) ((int32_t) in[2])) * ((int32_t) in[2]) +
               4 *  ((int64_t) ((int32_t) in[1])) * ((int32_t) in[3]) +
               2 *  ((int64_t) ((int32_t) in[0])) * ((int32_t) in[4]);
  output[5] =  2 * (((int64_t) ((int32_t) in[2])) * ((int32_t) in[3]) +
                    ((int64_t) ((int32_t) in[1])) * ((int32_t) in[4]) +
                    ((int64_t) ((int32_t) in[0])) * ((int32_t) in[5]));
  output[6] =  2 * (((int64_t) ((int32_t) in[3])) * ((int32_t) in[3]) +
                    ((int64_t) ((int32_t) in[2])) * ((int32_t) in[4]) +
                    ((int64_t) ((int32_t) in[0])) * ((int32_t) in[6]) +
               2 *  ((int64_t) ((int32_t) in[1])) * ((int32_t) in[5]));
  output[7] =  2 * (((int64_t) ((int32_t) in[3])) * ((int32_t) in[4]) +
                    ((int64_t) ((int32_t) in[2])) * ((int32_t) in[5]) +
                    ((int64_t) ((int32_t) in[1])) * ((int32_t) in[6]) +
                    ((int64_t) ((int32_t) in[0])) * ((int32_t) in[7]));
  output[8] =       ((int64_t) ((int32_t) in[4])) * ((int32_t) in[4]) +
               2 * (((int64_t) ((int32_t) in[2])) * ((int32_t) in[6]) +
                    ((int64_t) ((int32_t) in[0])) * ((int32_t) in[8]) +
               2 * (((int64_t) ((int32_t) in[1])) * ((int32_t) in[7]) +
                    ((int64_t) ((int32_t) in[3])) * ((int32_t) in[5])));
  output[9] =  2 * (((int64_t) ((int32_t) in[4])) * ((int32_t) in[5]) +
                    ((int64_t) ((int32_t) in[3])) * ((int32_t) in[6]) +
                    ((int64_t) ((int32_t) in[2])) * ((int32_t) in[7]) +
                    ((int64_t) ((int32_t) in[1])) * ((int32_t) in[8]) +
                    ((int64_t) ((int32_t) in[0])) * ((int32_t) in[9]));
  output[10] = 2 * (((int64_t) ((int32_t) in[5])) * ((int32_t) in[5]) +
                    ((int64_t) ((int32_t) in[4])) * ((int32_t) in[6]) +
                    ((int64_t) ((int32_t) in[2])) * ((int32_t) in[8]) +
               2 * (((int64_t) ((int32_t) in[3])) * ((int32_t) in[7]) +
                    ((int64_t) ((int32_t) in[1])) * ((int32_t) in[9])));
  output[11] = 2 * (((int64_t) ((int32_t) in[5])) * ((int32_t) in[6]) +
                    ((int64_t) ((int32_t) in[4])) * ((int32_t) in[7]) +
                    ((int64_t) ((int32_t) in[3])) * ((int32_t) in[8]) +
                    ((int64_t) ((int32_t) in[2])) * ((int32_t) in[9]));
  output[12] =      ((int64_t) ((int32_t) in[6])) * ((int32_t) in[6]) +
               2 * (((int64_t) ((int32_t) in[4])) * ((int32_t) in[8]) +
               2 * (((int64_t) ((int32_t) in[5])) * ((int32_t) in[7]) +
                    ((int64_t) ((int32_t) in[3])) * ((int32_t) in[9])));
  output[13] = 2 * (((int64_t) ((int32_t) in[6])) * ((int32_t) in[7]) +
                    ((int64_t) ((int32_t) in[5])) * ((int32_t) in[8]) +
                    ((int64_t) ((int32_t) in[4])) * ((int32_t) in[9]));
  output[14] = 2 * (((int64_t) ((int32_t) in[7])) * ((int32_t) in[7]) +
                    ((int64_t) ((int32_t) in[6])) * ((int32_t) in[8]) +
               2 *  ((int64_t) ((int32_t) in[5])) * ((int32_t) in[9]));
  output[15] = 2 * (((int64_t) ((int32_t) in[7])) * ((int32_t) in[8]) +
                    ((int64_t) ((int32_t) in[6])) * ((int32_t) in[9]));
  output[16] =      ((int64_t) ((int32_t) in[8])) * ((int32_t) in[8]) +
               4 *  ((int64_t) ((int32_t) in[7])) * ((int32_t) in[9]);
  output[17] = 2 *  ((int64_t) ((int32_t) in[8])) * ((int32_t) in[9]);
  output[18] = 2 *  ((int64_t) ((int32_t) in[9])) * ((int32_t) in[9]);
}

/* fsquare sets output = in^2.
 *
 * On entry: The |in| argument is in reduced coefficients form and |in[i]| <
 * 2^27.
 *
 * On exit: The |output| argument is in reduced coefficients form (indeed, one
 * need only provide storage for 10 limbs) and |out[i]| < 2^26. */
static constexpr void
fsquare(int64_t *output, const int64_t *in) {
    int64_t t[19];
    fsquare_inner(t, in);
    /* |t[i]| < 14*2^54 because the largest product of two limbs will be <
    * 2^(27+27) and fsquare_inner adds together, at most, 14 of those
    * products. */
    freduce_degree(t);
    freduce_coefficients(t);
    /* |t[i]| < 2^26 */
    for(size_t i = 0; i < 10; i++){
        output[i] = t[i];
    }
}

/* Take a little-endian, 32-byte number and expand it into polynomial form */
static constexpr void
fexpand(int64_t *output, const uint8_t *input) {
#define F(n,start,shift,mask) \
    output[n] = ((((int64_t) input[start + 0]) | \
                    ((int64_t) input[start + 1]) << 8 | \
                    ((int64_t) input[start + 2]) << 16 | \
                    ((int64_t) input[start + 3]) << 24) >> shift) & mask;
    F(0, 0, 0, 0x3ffffff);
    F(1, 3, 2, 0x1ffffff);
    F(2, 6, 3, 0x3ffffff);
    F(3, 9, 5, 0x1ffffff);
    F(4, 12, 6, 0x3ffffff);
    F(5, 16, 0, 0x1ffffff);
    F(6, 19, 1, 0x3ffffff);
    F(7, 22, 3, 0x1ffffff);
    F(8, 25, 4, 0x3ffffff);
    F(9, 28, 6, 0x1ffffff);
#undef F
}

#if (-32 >> 1) != -16
#error "This code only works when >> does sign-extension on negative numbers"
#endif

/* s32_eq returns 0xffffffff iff a == b and zero otherwise. */
static constexpr int32_t s32_eq(int32_t a, int32_t b) {
    a = ~(a ^ b);
    a &= a << 16;
    a &= a << 8;
    a &= a << 4;
    a &= a << 2;
    a &= a << 1;
    return a >> 31;
}

/* s32_gte returns 0xffffffff if a >= b and zero otherwise, where a and b are
 * both non-negative. */
static constexpr int32_t s32_gte(int32_t a, int32_t b) {
    a -= b;
    /* a >= 0 iff a >= b. */
    return ~(a >> 31);
}

/* Take a fully reduced polynomial form number and contract it into a
 * little-endian, 32-byte array.
 *
 * On entry: |input_limbs[i]| < 2^26 */
static constexpr void
fcontract(uint8_t *output, int64_t *input_limbs) {
    int32_t input[10];
    int32_t mask;

    /* |input_limbs[i]| < 2^26, so it's valid to convert to an s32. */
    for (size_t i = 0; i < 10; i++) {
        input[i] = (int32_t)input_limbs[i];
    }

    for (size_t j = 0; j < 2; ++j) {
        for (size_t i = 0; i < 9; ++i) {
        if ((i & 1) == 1) {
            /* This calculation is a time-invariant way to make input[i]
            * non-negative by borrowing from the next-larger int64_t. */
            const int32_t i_mask = input[i] >> 31;
            const int32_t carry = -((input[i] & i_mask) >> 25);
            input[i] = input[i] + (carry << 25);
            input[i+1] = input[i+1] - carry;
        } else {
            const int32_t i_mask = input[i] >> 31;
            const int32_t carry = -((input[i] & i_mask) >> 26);
            input[i] = input[i] + (carry << 26);
            input[i+1] = input[i+1] - carry;
        }
        }

        /* There's no greater int64_t for input[9] to borrow from, but we can multiply
        * by 19 and borrow from input[0], which is valid mod 2^255-19. */
        {
            const int32_t i_mask = input[9] >> 31;
            const int32_t carry = -((input[9] & i_mask) >> 25);
            input[9] = input[9] + (carry << 25);
            input[0] = input[0] - (carry * 19);
        }

        /* After the first iteration, input[1..9] are non-negative and fit within
        * 25 or 26 bits, depending on position. However, input[0] may be
        * negative. */
    }

    /* The first borrow-propagation pass above ended with every int64_t
        except (possibly) input[0] non-negative.

        If input[0] was negative after the first pass, then it was because of a
        carry from input[9]. On entry, input[9] < 2^26 so the carry was, at most,
        one, since (2**26-1) >> 25 = 1. Thus input[0] >= -19.

        In the second pass, each int64_t is decreased by at most one. Thus the second
        borrow-propagation pass could only have wrapped around to decrease
        input[0] again if the first pass left input[0] negative *and* input[1]
        through input[9] were all zero.  In that case, input[1] is now 2^25 - 1,
        and this last borrow-propagation step will leave input[1] non-negative. */
    {
        const int32_t i_mask = input[0] >> 31;
        const int32_t carry = -((input[0] & i_mask) >> 26);
        input[0] = input[0] + (carry << 26);
        input[1] = input[1] - carry;
    }

    /* All input[i] are now non-negative. However, there might be values between
    * 2^25 and 2^26 in a int64_t which is, nominally, 25 bits wide. */
    for (size_t j = 0; j < 2; j++) {
        for (size_t i = 0; i < 9; i++) {
        if ((i & 1) == 1) {
            const int32_t carry = input[i] >> 25;
            input[i] &= 0x1ffffff;
            input[i+1] += carry;
        } else {
            const int32_t carry = input[i] >> 26;
            input[i] &= 0x3ffffff;
            input[i+1] += carry;
        }
        }

        {
        const int32_t carry = input[9] >> 25;
        input[9] &= 0x1ffffff;
        input[0] += 19*carry;
        }
    }

    /* If the first carry-chain pass, just above, ended up with a carry from
    * input[9], and that caused input[0] to be out-of-bounds, then input[0] was
    * < 2^26 + 2*19, because the carry was, at most, two.
    *
    * If the second pass carried from input[9] again then input[0] is < 2*19 and
    * the input[9] -> input[0] carry didn't push input[0] out of bounds. */

    /* It still remains the case that input might be between 2^255-19 and 2^255.
    * In this case, input[1..9] must take their maximum value and input[0] must
    * be >= (2^255-19) & 0x3ffffff, which is 0x3ffffed. */
    mask = s32_gte(input[0], 0x3ffffed);
    for (size_t i = 1; i < 10; i++) {
        if ((i & 1) == 1) {
        mask &= s32_eq(input[i], 0x1ffffff);
        } else {
        mask &= s32_eq(input[i], 0x3ffffff);
        }
    }

    /* mask is either 0xffffffff (if input >= 2^255-19) and zero otherwise. Thus
    * this conditionally subtracts 2^255-19. */
    input[0] -= mask & 0x3ffffed;

    for (size_t i = 1; i < 10; i++) {
        if ((i & 1) == 1) {
        input[i] -= mask & 0x1ffffff;
        } else {
        input[i] -= mask & 0x3ffffff;
        }
    }

    input[1] <<= 2;
    input[2] <<= 3;
    input[3] <<= 5;
    input[4] <<= 6;
    input[6] <<= 1;
    input[7] <<= 3;
    input[8] <<= 4;
    input[9] <<= 6;
    #define F(i, s) \
    output[s+0] |=  input[i] & 0xff; \
    output[s+1]  = (input[i] >> 8) & 0xff; \
    output[s+2]  = (input[i] >> 16) & 0xff; \
    output[s+3]  = (input[i] >> 24) & 0xff;
    output[0] = 0;
    output[16] = 0;
    F(0,0);
    F(1,3);
    F(2,6);
    F(3,9);
    F(4,12);
    F(5,16);
    F(6,19);
    F(7,22);
    F(8,25);
    F(9,28);
    #undef F
}

/* Input: Q, Q', Q-Q'
 * Output: 2Q, Q+Q'
 *
 *   x2 z3: long form
 *   x3 z3: long form
 *   x z: short form, destroyed
 *   xprime zprime: short form, destroyed
 *   qmqp: short form, preserved
 *
 * On entry and exit, the absolute value of the limbs of all inputs and outputs
 * are < 2^26. */
static constexpr void fmonty(int64_t *x2, int64_t *z2,  /* output 2Q */
                   int64_t *x3, int64_t *z3,  /* output Q + Q' */
                   int64_t *x, int64_t *z,    /* input Q */
                   int64_t *xprime, int64_t *zprime,  /* input Q' */
                   const int64_t *qmqp /* input Q - Q' */) {
    int64_t origx[10], origxprime[10], zzz[19], xx[19], zz[19], xxprime[19],
            zzprime[19], zzzprime[19], xxxprime[19];


    COPY_X10(origx, x)
    fsum(x, z);
    /* |x[i]| < 2^27 */
    fdifference(z, origx);  /* does x - z */
    /* |z[i]| < 2^27 */

    COPY_X10(origxprime, xprime);
    fsum(xprime, zprime);
    /* |xprime[i]| < 2^27 */
    fdifference(zprime, origxprime);
    /* |zprime[i]| < 2^27 */
    fproduct(xxprime, xprime, z);
    /* |xxprime[i]| < 14*2^54: the largest product of two limbs will be <
    * 2^(27+27) and fproduct adds together, at most, 14 of those products.
    * (Approximating that to 2^58 doesn't work out.) */
    fproduct(zzprime, x, zprime);
    /* |zzprime[i]| < 14*2^54 */
    freduce_degree(xxprime);
    freduce_coefficients(xxprime);
    /* |xxprime[i]| < 2^26 */
    freduce_degree(zzprime);
    freduce_coefficients(zzprime);
    /* |zzprime[i]| < 2^26 */
    COPY_X10(origxprime, xxprime);
    fsum(xxprime, zzprime);
    /* |xxprime[i]| < 2^27 */
    fdifference(zzprime, origxprime);
    /* |zzprime[i]| < 2^27 */
    fsquare(xxxprime, xxprime);
    /* |xxxprime[i]| < 2^26 */
    fsquare(zzzprime, zzprime);
    /* |zzzprime[i]| < 2^26 */
    fproduct(zzprime, zzzprime, qmqp);
    /* |zzprime[i]| < 14*2^52 */
    freduce_degree(zzprime);
    freduce_coefficients(zzprime);
    /* |zzprime[i]| < 2^26 */
    COPY_X10(x3, xxxprime);
    COPY_X10(z3, zzprime);

    fsquare(xx, x);
    /* |xx[i]| < 2^26 */
    fsquare(zz, z);
    /* |zz[i]| < 2^26 */
    fproduct(x2, xx, zz);
    /* |x2[i]| < 14*2^52 */
    freduce_degree(x2);
    freduce_coefficients(x2);
    /* |x2[i]| < 2^26 */
    fdifference(zz, xx);  // does zz = xx - zz
    /* |zz[i]| < 2^27 */
    for(size_t i = 0; i < 9; i++) *(zzz + 10 + i) = 0;
    fscalar_product(zzz, zz, 121665);
    /* |zzz[i]| < 2^(27+17) */
    /* No need to call freduce_degree here:
        fscalar_product doesn't increase the degree of its input. */
    freduce_coefficients(zzz);
    /* |zzz[i]| < 2^26 */
    fsum(zzz, xx);
    /* |zzz[i]| < 2^27 */
    fproduct(z2, zz, zzz);
    /* |z2[i]| < 14*2^(26+27) */
    freduce_degree(z2);
    freduce_coefficients(z2);
    /* |z2|i| < 2^26 */
}

/* Conditionally swap two reduced-form int64_t arrays if 'iswap' is 1, but leave
 * them unchanged if 'iswap' is 0.  Runs in data-invariant time to avoid
 * side-channel attacks.
 *
 * NOTE that this function requires that 'iswap' be 1 or 0; other values give
 * wrong results.  Also, the two int64_t arrays must be in reduced-coefficient,
 * reduced-degree form: the values in a[10..19] or b[10..19] aren't swapped,
 * and all all values in a[0..9],b[0..9] must have magnitude less than
 * INT32_MAX. */
static constexpr void
swap_conditional(int64_t a[19], int64_t b[19], int64_t iswap) {
    const int32_t swap = (int32_t) -iswap;

    for(size_t i = 0; i < 10; ++i) {
        const int32_t x = swap & ( ((int32_t)a[i]) ^ ((int32_t)b[i]) );
        a[i] = ((int32_t)a[i]) ^ x;
        b[i] = ((int32_t)b[i]) ^ x;
    }
}

/* Calculates nQ where Q is the x-coordinate of a point on the curve
 *
 *   resultx/resultz: the x coordinate of the resulting curve point (short form)
 *   n: a little endian, 32-byte number
 *   q: a point of the curve (short form) */
static constexpr void
    cmult(int64_t *resultx, int64_t *resultz, const uint8_t *n, const int64_t *q) {
    int64_t a[19] = {0}, b[19] = {1}, c[19] = {1}, d[19] = {0};
    int64_t *nqpqx = a, *nqpqz = b, *nqx = c, *nqz = d, *t;
    int64_t e[19] = {0}, f[19] = {1}, g[19] = {0}, h[19] = {1};
    int64_t *nqpqx2 = e, *nqpqz2 = f, *nqx2 = g, *nqz2 = h;


        COPY_X10(nqpqx, q)

    for(size_t i = 0; i < 32; ++i) {
        uint8_t byte = n[31 - i];
        for (size_t j = 0; j < 8; ++j) {
        const int64_t bit = byte >> 7;

        swap_conditional(nqx, nqpqx, bit);
        swap_conditional(nqz, nqpqz, bit);
        fmonty(nqx2, nqz2,
                nqpqx2, nqpqz2,
                nqx, nqz,
                nqpqx, nqpqz,
                q);
        swap_conditional(nqx2, nqpqx2, bit);
        swap_conditional(nqz2, nqpqz2, bit);

        t = nqx;
        nqx = nqx2;
        nqx2 = t;
        t = nqz;
        nqz = nqz2;
        nqz2 = t;
        t = nqpqx;
        nqpqx = nqpqx2;
        nqpqx2 = t;
        t = nqpqz;
        nqpqz = nqpqz2;
        nqpqz2 = t;

        byte <<= 1;
        }
    }

    COPY_X10(resultx, nqx);
    COPY_X10(resultz, nqz);
}

// -----------------------------------------------------------------------------
// Shamelessly copied from djb's code
// -----------------------------------------------------------------------------
static constexpr void
crecip(int64_t *out, const int64_t *z) {
    int64_t z2[10];
    int64_t z9[10];
    int64_t z11[10];
    int64_t z2_5_0[10];
    int64_t z2_10_0[10];
    int64_t z2_20_0[10];
    int64_t z2_50_0[10];
    int64_t z2_100_0[10];
    int64_t t0[10];
    int64_t t1[10];

  /* 2 */ fsquare(z2,z);
  /* 4 */ fsquare(t1,z2);
  /* 8 */ fsquare(t0,t1);
  /* 9 */ fmul(z9,t0,z);
  /* 11 */ fmul(z11,z9,z2);
  /* 22 */ fsquare(t0,z11);
  /* 2^5 - 2^0 = 31 */ fmul(z2_5_0,t0,z9);

  /* 2^6 - 2^1 */ fsquare(t0,z2_5_0);
  /* 2^7 - 2^2 */ fsquare(t1,t0);
  /* 2^8 - 2^3 */ fsquare(t0,t1);
  /* 2^9 - 2^4 */ fsquare(t1,t0);
  /* 2^10 - 2^5 */ fsquare(t0,t1);
  /* 2^10 - 2^0 */ fmul(z2_10_0,t0,z2_5_0);

  /* 2^11 - 2^1 */ fsquare(t0,z2_10_0);
  /* 2^12 - 2^2 */ fsquare(t1,t0);
  /* 2^20 - 2^10 */ for(size_t i = 2;i < 10;i += 2) { fsquare(t0,t1); fsquare(t1,t0); }
  /* 2^20 - 2^0 */ fmul(z2_20_0,t1,z2_10_0);

  /* 2^21 - 2^1 */ fsquare(t0,z2_20_0);
  /* 2^22 - 2^2 */ fsquare(t1,t0);
  /* 2^40 - 2^20 */ for(size_t i = 2;i < 20;i += 2) { fsquare(t0,t1); fsquare(t1,t0); }
  /* 2^40 - 2^0 */ fmul(t0,t1,z2_20_0);

  /* 2^41 - 2^1 */ fsquare(t1,t0);
  /* 2^42 - 2^2 */ fsquare(t0,t1);
  /* 2^50 - 2^10 */ for(size_t i = 2;i < 10;i += 2) { fsquare(t1,t0); fsquare(t0,t1); }
  /* 2^50 - 2^0 */ fmul(z2_50_0,t0,z2_10_0);

  /* 2^51 - 2^1 */ fsquare(t0,z2_50_0);
  /* 2^52 - 2^2 */ fsquare(t1,t0);
  /* 2^100 - 2^50 */ for(size_t i = 2;i < 50;i += 2) { fsquare(t0,t1); fsquare(t1,t0); }
  /* 2^100 - 2^0 */ fmul(z2_100_0,t1,z2_50_0);

  /* 2^101 - 2^1 */ fsquare(t1,z2_100_0);
  /* 2^102 - 2^2 */ fsquare(t0,t1);
  /* 2^200 - 2^100 */ for(size_t i = 2;i < 100;i += 2) { fsquare(t1,t0); fsquare(t0,t1); }
  /* 2^200 - 2^0 */ fmul(t1,t0,z2_100_0);

  /* 2^201 - 2^1 */ fsquare(t0,t1);
  /* 2^202 - 2^2 */ fsquare(t1,t0);
  /* 2^250 - 2^50 */ for(size_t i = 2;i < 50;i += 2) { fsquare(t0,t1); fsquare(t1,t0); }
  /* 2^250 - 2^0 */ fmul(t0,t1,z2_50_0);

  /* 2^251 - 2^1 */ fsquare(t1,t0);
  /* 2^252 - 2^2 */ fsquare(t0,t1);
  /* 2^253 - 2^3 */ fsquare(t1,t0);
  /* 2^254 - 2^4 */ fsquare(t0,t1);
  /* 2^255 - 2^5 */ fsquare(t1,t0);
  /* 2^255 - 21 */ fmul(out,t1,z11);
}

}
#undef COPY_X10