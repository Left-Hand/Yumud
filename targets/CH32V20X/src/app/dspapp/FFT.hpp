#pragma once

#include <span>
#include "sys/math/real.hpp"

namespace ymd::dsp{
/**********************************************************************
  FFT - calculates the discrete fourier transform of an array of T
  precision complex numbers using the FFT algorithm.

  c = pointer to an array of size 2*N that contains the real and
    imaginary parts of the complex numbers. The even numbered indices contain
    the real parts and the odd numbered indices contain the imaginary parts.
      c[2*k] = real part of kth data point.
      c[2*k+1] = imaginary part of kth data point.
  N = number of data points. The array, c, should contain 2*N elements
  isign = 1 for forward FFT, -1 for inverse FFT.
*/

namespace details{
template<arithmetic T>
constexpr void _FFT(std::span<T> c, int isign){
    int n, n2, nb, j, k, i0, i1;
    T wr, wi, wrk, wik;
    T d, dr, di, d0r, d0i, d1r, d1i;
    T *cp;

    j = 0;
    const auto N = c.size();
    n2 = N / 2;
    for( k = 0; k < N; ++k ){
        if( k < j )
        {
        i0 = k << 1;
        i1 = j << 1;
        dr = c[i0];
        di = c[i0+1];
        c[i0] = c[i1];
        c[i0+1] = c[i1+1];
        c[i1] = dr;
        c[i1+1] = di;
        }
        n = N >> 1;
        while( (n >= 2) && (j >= n) )
        {
        j -= n;
        n = n >> 1;
        }
        j += n;
    }

    for( n = 2; n <= N; n = n << 1 ){
        wr = std::cos(T(TAU) / n );
        wi = std::sin(T(TAU) / n );
        if( isign == 1 ) wi = -wi;
        cp = c;
        nb = N / n;
        n2 = n >> 1;
        for( j = 0; j < nb; ++j ){
            wrk = 1;
            wik = 0;
            for( k = 0; k < n2; ++k ){
                i0 = k << 1;
                i1 = i0 + n;
                d0r = cp[i0];
                d0i = cp[i0+1];
                d1r = cp[i1];
                d1i = cp[i1+1];
                dr = wrk * d1r - wik * d1i;
                di = wrk * d1i + wik * d1r;
                cp[i0] = d0r + dr;
                cp[i0+1] = d0i + di;
                cp[i1] = d0r - dr;
                cp[i1+1] = d0i - di;
                d = wrk;
                wrk = wr * wrk - wi * wik;
                wik = wr * wik + wi * d;
            }
            cp += n << 1;
        }
    }
}
}

template<arithmetic T>
constexpr void FFT(std::span<T> c){
    details::_FFT(c, 1);
}

template<arithmetic T>
constexpr void IFFT(std::span<T> c){
    details::_FFT(c, -1);
}

}