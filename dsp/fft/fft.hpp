/*

	FFT library
	Copyright (C) 2010 Didier Longueville
	Copyright (C) 2014 Enrique Condes

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef ArduinoFFT_h /* Prevent loading library twice */
#define ArduinoFFT_h

#include "sys/core/platform.h"
#include <math.h>
#include <cstdint>

// Define this to use a low-precision square root approximation instead of the
// regular sqrt() call
// This might only work for specific use cases, but is significantly faster.
// Only works for ArduinoFFT<float>.
// #define FFT_SQRT_APPROXIMATION

#ifdef FFT_SQRT_APPROXIMATION
#include <type_traits>
#else
#define sqrt_internal sqrt
#endif



class FFT {
public:
    enum class Direction { Reverse, Forward };

    enum class Window {
    Rectangle,        // rectangle (Box car)
    Hamming,          // hamming
    Hann,             // hann
    Triangle,         // triangle (Bartlett)
    Nuttall,          // nuttall
    Blackman,         // blackman
    Blackman_Nuttall, // blackman nuttall
    Blackman_Harris,  // blackman harris
    Flat_top,         // flat top
    Welch             // welch
    };
  /* Constructor */
  FFT(double *vReal, double *vImag, uint16_t samples,
             double samplingFrequency);
  /* Destructor */
  ~FFT(void);
  /* Functions */

  uint8_t Exponent(uint16_t value);

  void ComplexToMagnitude();
  void Compute(Direction dir);
  void DCRemoval();
  double MajorPeak();
  void MajorPeak(double *f, double *v);
  void Windowing(Window windowType, Direction dir);

  double MajorPeakParabola();

private:
    scexpr double twoPi = 6.28318531;
    scexpr double fourPi = 12.56637061;
    scexpr double sixPi = 18.84955593;
  /* Variables */
  uint16_t _samples;
  double _samplingFrequency;
  double *_vReal;
  double *_vImag;
  uint8_t _power;
  /* Functions */
  double sq(const double & x){return x*x;}
  void Swap(double *x, double *y);
  void Parabola(double x1, double y1, double x2, double y2, double x3,
                double y3, double *a, double *b, double *c);
};

#endif
