// /*

// 	FFT library
// 	Copyright (C) 2010 Didier Longueville
// 	Copyright (C) 2014 Enrique Condes

// 	This program is free software: you can redistribute it and/or modify
// 	it under the terms of the GNU General Public License as published by
// 	the Free Software Foundation, either version 3 of the License, or
// 	(at your option) any later version.

// 	This program is distributed in the hope that it will be useful,
// 	but WITHOUT ANY WARRANTY; without even the implied warranty of
// 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// 	GNU General Public License for more details.

// 	You should have received a copy of the GNU General Public License
// 	along with this program.  If not, see <http://www.gnu.org/licenses/>.

// */

// #ifndef ArduinoFFT_h /* Prevent loading library twice */
// #define ArduinoFFT_h

// #include "core/platform.hpp"
// // #include <math.h>
// #include <cstdint>

// // Define this to use a low-precision square root approximation instead of the
// // regular sqrt() call
// // This might only work for specific use cases, but is significantly faster.
// // Only works for ArduinoFFT<float>.
// // #define FFT_SQRT_APPROXIMATION

// #ifdef FFT_SQRT_APPROXIMATION
// #include <type_traits>
// #else
// #define sqrt_internal sqrt
// #endif



// class FFT {
// public:
//     enum class Direction { Reverse, Forward };

//     enum class Window {
//     Rectangle,        // rectangle (Box car)
//     Hamming,          // hamming
//     Hann,             // hann
//     Triangle,         // triangle (Bartlett)
//     Nuttall,          // nuttall
//     Blackman,         // blackman
//     Blackman_Nuttall, // blackman nuttall
//     Blackman_Harris,  // blackman harris
//     Flat_top,         // flat top
//     Welch             // welch
//     };
//   /* Constructor */
//   FFT(real_t *vReal, real_t *vImag, uint16_t samples,
//              real_t samplingFrequency);
//   /* Destructor */
//   ~FFT(void);
//   /* Functions */

//   uint8_t Exponent(uint16_t value);

//   void ComplexToMagnitude();
//   void Compute(Direction dir);
//   void DCRemoval();
//   real_t MajorPeak();
//   void MajorPeak(real_t *f, real_t *v);
//   void Windowing(Window windowType, Direction dir);

//   real_t MajorPeakParabola();

// private:
//     static constexpr real_t twoPi = 6.28318531;
//     static constexpr real_t fourPi = 12.56637061;
//     static constexpr real_t sixPi = 18.84955593;
//   /* Variables */
//   uint16_t _samples;
//   real_t _samplingFrequency;
//   real_t *_vReal;
//   real_t *_vImag;
//   uint8_t _power;
//   /* Functions */
//   real_t sq(const real_t & x){return x*x;}
//   void Swap(real_t *x, real_t *y);
//   void Parabola(real_t x1, real_t y1, real_t x2, real_t y2, real_t x3,
//                 real_t y3, real_t *a, real_t *b, real_t *c);
// };

// #endif
