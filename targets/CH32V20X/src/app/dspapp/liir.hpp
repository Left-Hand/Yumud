/*
 *                            COPYRIGHT
 *
 *  liir - Recursive digital filter functions
 *  Copyright (C) 2007 Exstrom Laboratories LLC
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  A copy of the GNU General Public License is available on the internet at:
 *
 *  http://www.gnu.org/copyleft/gpl.html
 *
 *  or you can write to:
 *
 *  The Free Software Foundation, Inc.
 *  675 Mass Ave
 *  Cambridge, MA 02139, USA
 *
 *  You can contact Exstrom Laboratories LLC via Email at:
 *
 *  stefan(AT)exstrom.com
 *
 *  or you can write to:
 *
 *  Exstrom Laboratories LLC
 *  P.O. Box 7651
 *  Longmont, CO 80501, USA
 *
 */

#pragma once

#include "core/math/realmath.hpp"


namespace ymd::dsp{

/**********************************************************************
 binomial_mult - multiplies a series of binomials together and returns
the coefficients of the resulting polynomial.

The multiplication has the following form:

(x+p[0])*(x+p[1])*...*(x+p[N-1])

The p[i] coefficients are assumed to be complex and are passed to the 
function as a pointer to an array of Ts of length 2n.

The resulting polynomial has the following form:

x^N + a[0]*x^N-1 + a[1]*x^N-2 + ... +a[N-2]*x + a[N-1]

The a[i] coefficients can in general be complex but should in most
cases turn out to be real. The a[i] coefficients are returned by the
function as a pointer to an array of Ts of length 2n. Storage
for the array is allocated by the function and should be freed by the
calling program when no longer needed.

Function arguments:

N  -  The number of binomials to multiply
p  -  Pointer to an array of Ts where p[2i] (i=0...N-1) is
        assumed to be the real part of the coefficient of the ith binomial
        and p[2i+1] is assumed to be the imaginary part. The overall size
        of the array is then 2n.
*/

template<
    arithmetic T, 
    size_t N, 
    typename Ret = std::array<T, 2 * N>>
constexpr Ret binomial_mult(const std::span<T, 2 * N> p)
{
    size_t i, j;

    Ret a;

    for( i = 0; i < N; ++i ){
        for( j = i; j > 0; --j ){
            a[2*j] += p[2*i] * a[2*(j-1)] - p[2*i+1] * a[2*(j-1)+1];
            a[2*j+1] += p[2*i] * a[2*(j-1)+1] + p[2*i+1] * a[2*(j-1)];
        }
        a[0] += p[2*i];
        a[1] += p[2*i+1];
    }
    return( a );
}


/**********************************************************************
 trinomial_mult - multiplies a series of trinomials together and returns
the coefficients of the resulting polynomial.

The multiplication has the following form:

(x^2 + b[0]x + c[0])*(x^2 + b[1]x + c[1])*...*(x^2 + b[N-1]x + c[N-1])

The b[i] and c[i] coefficients are assumed to be complex and are passed
to the function as a pointers to arrays of Ts of length 2n. The real
part of the coefficients are stored in the even numbered elements of the
array and the imaginary parts are stored in the odd numbered elements.

The resulting polynomial has the following form:

x^2n + a[0]*x^2n-1 + a[1]*x^2n-2 + ... +a[2n-2]*x + a[2n-1]

The a[i] coefficients can in general be complex but should in most cases
turn out to be real. The a[i] coefficients are returned by the function as
a pointer to an array of Ts of length 4n. The real and imaginary
parts are stored, respectively, in the even and odd elements of the array.
Storage for the array is allocated by the function and should be freed by
the calling program when no longer needed.

Function arguments:

N  -  The number of trinomials to multiply
b  -  Pointer to an array of Ts of length 2n.
c  -  Pointer to an array of Ts of length 2n.
*/

template<
    arithmetic T, 
    size_t N, 
    typename Ret = std::array<T, 4 * N>>
constexpr  Ret trinomial_mult(
    const std::span<const T, 2 * N> b, 
    const std::span<const T, 2 * N> c
){

    Ret a;

    a[2] = c[0];
    a[3] = c[1];
    a[0] = b[0];
    a[1] = b[1];

    for(size_t i = 1; i < N; ++i ){
        a[2*(2*i+1)]   += c[2*i]*a[2*(2*i-1)]   - c[2*i+1]*a[2*(2*i-1)+1];
        a[2*(2*i+1)+1] += c[2*i]*a[2*(2*i-1)+1] + c[2*i+1]*a[2*(2*i-1)];

        for(size_t  j = 2*i; j > 1; --j ){
            a[2*j]   += b[2*i] * a[2*(j-1)]   - b[2*i+1] * a[2*(j-1)+1] + 
            c[2*i] * a[2*(j-2)]   - c[2*i+1] * a[2*(j-2)+1];
            a[2*j+1] += b[2*i] * a[2*(j-1)+1] + b[2*i+1] * a[2*(j-1)] +
            c[2*i] * a[2*(j-2)+1] + c[2*i+1] * a[2*(j-2)];
        }

        a[2] += b[2*i] * a[0] - b[2*i+1] * a[1] + c[2*i];
        a[3] += b[2*i] * a[1] + b[2*i+1] * a[0] + c[2*i+1];
        a[0] += b[2*i];
        a[1] += b[2*i+1];
    }

    return( a );
}


/**********************************************************************
 dcof_bwlp - calculates the d coefficients for a butterworth lowpass 
filter. The coefficients are returned as an array of Ts.

*/

template<
    arithmetic T, 
    size_t N, 
    typename Ret = std::array<T, 2 * N>>
constexpr  Ret dcof_bwlp(T fcf ){
    Ret rcof;

    const auto theta = T(PI) * fcf;
    const auto st = std::sin(theta);
    const auto ct = std::cos(theta);

    for(size_t  k = 0; k < N; ++k ){
        const auto parg = T(PI) * (T)(2*k+1)/(T)(2*N);
        const auto sparg = std::sin(parg);
        const auto cparg = std::cos(parg);
        const auto a = 1 + st*sparg;
        rcof[2*k] = -ct/a;
        rcof[2*k+1] = -st*cparg/a;
    }

    auto dcof = binomial_mult(rcof);

    dcof[1] = dcof[0];
    dcof[0] = 1;
    for(size_t  k = 3; k <= N; ++k )
        dcof[k] = dcof[2*k-2];
    return( dcof );
}

/**********************************************************************
 dcof_bwhp - calculates the d coefficients for a butterworth highpass 
filter. The coefficients are returned as an array of Ts.

*/

template<
    arithmetic T, 
    size_t N, 
    typename Ret = std::array<T, 2 * N>>
constexpr Ret dcof_bwhp(T fcf ){
    return( dcof_bwlp( N, fcf ) );
}


/**********************************************************************
 dcof_bwbp - calculates the d coefficients for a butterworth bandpass 
filter. The coefficients are returned as an array of Ts.

*/

template<arithmetic T, size_t N> 
constexpr std::array<T, 4 * N> dcof_bwbp(T f1f, T f2f ){
    const auto cp = std::cos(T(PI) * (f2f + f1f) / 2);
    const auto theta = T(PI) * (f2f - f1f) / 2;
    const auto [st, ct] = sincos(theta);
    const auto s2t = 2*st*ct;        // sine of 2*theta
    const auto c2t = 2*ct*ct - 1;  // cosine of 2*theta

    std::array<T, 2 * N> rcof;
    std::array<T, 2 * N> tcof;

    for(size_t  k = 0; k < N; ++k ){
        const auto parg = T(PI) * (T)(2*k+1)/(T)(2*N);
        const auto sparg = std::sin(parg);
        const auto cparg = std::cos(parg);
        const auto a = 1 + s2t*sparg;
        rcof[2*k] = c2t/a;
        rcof[2*k+1] = s2t*cparg/a;
        tcof[2*k] = -2*cp*(ct+st*sparg)/a;
        tcof[2*k+1] = -2*cp*st*cparg/a;
    }

    auto dcof = trinomial_mult<T, N>(tcof, rcof);

    dcof[1] = dcof[0];
    dcof[0] = 1;
    for(size_t k = 3; k <= 2*N; ++k )
        dcof[k] = dcof[2*k-2];
    return dcof;
}

/**********************************************************************
 dcof_bwbs - calculates the d coefficients for a butterworth bandstop 
filter. The coefficients are returned as an array of Ts.

*/

template<arithmetic T, size_t N>
constexpr std::array<T, 2*N> dcof_bwbs(T f1f, T f2f ){

    const auto cp = std::cos(T(PI) * (f2f + f1f) / 2);
    const auto theta = T(PI) * (f2f - f1f) / 2;
    const auto [st, ct] = sincos(theta);
    const auto s2t = 2*st*ct;        // sine of 2*theta
    const auto c2t = 2*ct*ct - 1;  // cosine 0f 2*theta

    std::array<T, 2*N> rcof;
    std::array<T, 2*N> tcof;  

    for(size_t  k = 0; k < N; ++k ){
        const auto parg = T(PI) * (T)(2*k+1)/(T)(2*N);
        const auto sparg = std::sin(parg);
        const auto cparg = std::cos(parg);
        const auto a = 1 + s2t*sparg;
        rcof[2*k] = c2t/a;
        rcof[2*k+1] = -s2t*cparg/a;
        tcof[2*k] = -2*cp*(ct+st*sparg)/a;
        tcof[2*k+1] = 2*cp*st*cparg/a;
    }

    auto dcof = trinomial_mult( N, tcof, rcof );

    dcof[1] = dcof[0];
    dcof[0] = 1;
    for(size_t  k = 3; k <= 2*N; ++k )
        dcof[k] = dcof[2*k-2];
    return( dcof );
}

/**********************************************************************
 ccof_bwlp - calculates the c coefficients for a butterworth lowpass 
filter. The coefficients are returned as an array of integers.

*/

template<size_t N, typename Ret = std::array<size_t, N+1>>
constexpr Ret ccof_bwlp(){

    Ret ccof;

    ccof[0] = 1;
    ccof[1] = N;
    static constexpr size_t m = N/2;

    for(size_t i=2; i <= m; ++i){
        ccof[i] = (N-i+1)*ccof[i-1]/i;
        ccof[N-i]= ccof[i];
    }

    ccof[N-1] = N;
    ccof[N] = 1;

    return( ccof );
}

/**********************************************************************
 ccof_bwhp - calculates the c coefficients for a butterworth highpass 
filter. The coefficients are returned as an array of integers.

*/

template<size_t N>
constexpr std::array<size_t, N + 1> ccof_bwhp(){

    auto ccof = ccof_bwlp<N>();

    for(size_t  i = 0; i <= N; ++i)
        if( i % 2 ) ccof[i] = -ccof[i];

    return( ccof );
}

/**********************************************************************
 ccof_bwbp - calculates the c coefficients for a butterworth bandpass 
filter. The coefficients are returned as an array of integers.

*/

template<size_t N, typename Ret = std::array<size_t, 2 * N + 1>>
constexpr Ret ccof_bwbp(){

    Ret ccof;

    const auto tcof = ccof_bwhp<N>();

    for(size_t i = 0; i < N; ++i){
        ccof[2*i] = tcof[i];
        ccof[2*i+1] = (0);
    }
    ccof[2*N] = tcof[N];

    return( ccof );
}

/**********************************************************************
 ccof_bwbs - calculates the c coefficients for a butterworth bandstop 
filter. The coefficients are returned as an array of integers.

*/


template<arithmetic T, size_t N, typename Ret = std::array<T, 2 * N + 1>>
constexpr Ret ccof_bwbs(T f1f, T f2f )
{
    size_t i, j;

    const auto alpha = -2 * std::cos(T(PI) * (f2f + f1f) / 2) / std::cos(T(PI) * (f2f - f1f) / 2);

    Ret ccof;

    ccof[0] = 1;

    ccof[2] = 1;
    ccof[1] = alpha;

    for( i = 1; i < N; ++i ){
        ccof[2*i+2] += ccof[2*i];
        for( j = 2*i; j > 1; --j )
            ccof[j+1] += alpha * ccof[j] + ccof[j-1];

        ccof[2] += alpha * ccof[1] + 1;
        ccof[1] += alpha;
    }

    return( ccof );
}

/**********************************************************************
 sf_bwlp - calculates the scaling factor for a butterworth lowpass filter.
The scaling factor is what the c coefficients must be multiplied by so
that the filter response has a maximum value of 1.

*/

template<arithmetic T>
constexpr T sf_bwlp( size_t N, T fcf )
{
    size_t m, k;         // loop variables
    T omega;     // T(PI) * fcf
    T fomega;    // function of omega
    T parg0;     // zeroth pole angle
    T sf;        // scaling factor

    omega = T(PI) * fcf;
    fomega = std::sin(omega);
    parg0 = T(PI) / (2*N);

    m = N / 2;
    sf = 1;
    for( k = 0; k < N/2; ++k )
        sf *= 1 + fomega * std::sin((T)(2*k+1)*parg0);

    fomega = std::sin(omega / 2);

    if( N % 2 ) sf *= fomega + std::cos(omega / 2);
    sf = pow( fomega, N ) / sf;

    return(sf);
}

/**********************************************************************
 sf_bwhp - calculates the scaling factor for a butterworth highpass filter.
The scaling factor is what the c coefficients must be multiplied by so
that the filter response has a maximum value of 1.

*/

template<arithmetic T>
constexpr T sf_bwhp( size_t N, T fcf )
{
    size_t m, k;         // loop variables
    T omega;     // T(PI) * fcf
    T fomega;    // function of omega
    T parg0;     // zeroth pole angle
    T sf;        // scaling factor

    omega = T(PI) * fcf;
    fomega = std::sin(omega);
    parg0 = T(PI) / (2*N);

    m = N / 2;
    sf = 1;
    for( k = 0; k < N/2; ++k )
        sf *= 1 + fomega * std::sin((T)(2*k+1)*parg0);

    fomega = std::cos(omega / 2);

    if( N % 2 ) sf *= fomega + std::sin(omega / 2);
    sf = pow( fomega, N ) / sf;

    return(sf);
}

/**********************************************************************
 sf_bwbp - calculates the scaling factor for a butterworth bandpass filter.
The scaling factor is what the c coefficients must be multiplied by so
that the filter response has a maximum value of 1.

*/

template<arithmetic T, size_t N>
constexpr T sf_bwbp(T f1f, T f2f )
{
    T ctt;       // cotangent of theta
    T sfr, sfi;  // real and imaginary parts of the scaling factor
    T parg;      // pole angle
    T sparg;     // sine of pole angle
    T cparg;     // cosine of pole angle
    T a, b, c;   // workspace variables

    ctt = 1 / std::tan(T(PI) * (f2f - f1f) / 2);
    sfr = 1;
    sfi = T(0);

    for(size_t  k = 0; k < N; ++k )
    {
        parg = T(PI) * (T)(2*k+1)/(T)(2*N);
        sparg = ctt + std::sin(parg);
        cparg = std::cos(parg);
        a = (sfr + sfi)*(sparg - cparg);
        b = sfr * sparg;
        c = -sfi * cparg;
        sfr = b - c;
        sfi = a - b - c;
    }

    return( 1 / sfr );
}

/**********************************************************************
 sf_bwbs - calculates the scaling factor for a butterworth bandstop filter.
The scaling factor is what the c coefficients must be multiplied by so
that the filter response has a maximum value of 1.

*/

template<arithmetic T>
constexpr T sf_bwbs( size_t N, T f1f, T f2f )
{
    T tt;        // tangent of theta
    T sfr, sfi;  // real and imaginary parts of the scaling factor
    T parg;      // pole angle
    T sparg;     // sine of pole angle
    T cparg;     // cosine of pole angle
    T a, b, c;   // workspace variables

    tt = tan(T(PI) * (f2f - f1f) / 2);
    sfr = 1;
    sfi = T(0);

    for(size_t  k = 0; k < N; ++k ){
        parg = T(PI) * (T)(2*k+1)/(T)(2*N);
        sparg = tt + std::sin(parg);
        cparg = std::cos(parg);
        a = (sfr + sfi)*(sparg - cparg);
        b = sfr * sparg;
        c = -sfi * cparg;
        sfr = b - c;
        sfi = a - b - c;
    }

    return( 1 / sfr );
}

}