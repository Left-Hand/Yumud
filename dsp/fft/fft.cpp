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

// #include "fft.hpp"

// FFT::FFT(real_t *vReal, real_t *vImag, uint16_t samples,
//                        real_t samplingFrequency) { // Constructor
//   this->_vReal = vReal;
//   this->_vImag = vImag;
//   this->_samples = samples;
//   this->_samplingFrequency = samplingFrequency;
//   this->_power = Exponent(samples);
// }

// FFT::~FFT(void) {
//   // Destructor
// }

// uint8_t FFT::Exponent(uint16_t value){
//   // Calculates the base 2 logarithm of a value
//   uint8_t result = 0;
//   while (value >>= 1)
//     result++;
//   return (result);
// }

// void FFT::Compute(Direction dir) {
//   // Computes in-place complex-to-complex FFT /
//   // Reverse bits /
//   uint16_t j = 0;
//   for (uint16_t i = 0; i < (this->_samples - 1); i++) {
//     if (i < j) {
//       Swap(&this->_vReal[i], &this->_vReal[j]);
//       if (dir == Direction::Reverse)
//         Swap(&this->_vImag[i], &this->_vImag[j]);
//     }
//     uint16_t k = (this->_samples >> 1);
//     while (k <= j) {
//       j -= k;
//       k >>= 1;
//     }
//     j += k;
//   }
// // Compute the FFT
//   real_t c1 = -1.0;
//   real_t c2 = 0.0;
//   uint16_t l2 = 1;
//   for (uint8_t l = 0; (l < this->_power); l++) {
//     uint16_t l1 = l2;
//     l2 <<= 1;
//     real_t u1 = 1.0;
//     real_t u2 = 0.0;
//     for (j = 0; j < l1; j++) {
//       for (uint16_t i = j; i < this->_samples; i += l2) {
//         uint16_t i1 = i + l1;
//         real_t t1 = u1 * this->_vReal[i1] - u2 * this->_vImag[i1];
//         real_t t2 = u1 * this->_vImag[i1] + u2 * this->_vReal[i1];
//         this->_vReal[i1] = this->_vReal[i] - t1;
//         this->_vImag[i1] = this->_vImag[i] - t2;
//         this->_vReal[i] += t1;
//         this->_vImag[i] += t2;
//       }
//       real_t z = ((u1 * c1) - (u2 * c2));
//       u2 = ((u1 * c2) + (u2 * c1));
//       u1 = z;
//     }
// #ifdef __AVR__
//     c2 = pgm_read_float_near(&(_c2[index]));
//     c1 = pgm_read_float_near(&(_c1[index]));
//     index++;
// #else
//     c2 = sqrt((1.0 - c1) / 2.0);
//     c1 = sqrt((1.0 + c1) / 2.0);
// #endif
//     if (dir == Direction::Forward) {
//       c2 = -c2;
//     }
//   }
//   // Scaling for reverse transform /
//   if (dir != Direction::Forward) {
//     real_t reciprocal = 1.0 / this->_samples;
//     for (uint16_t i = 0; i < this->_samples; i++) {
//       this->_vReal[i] *= reciprocal;
//       this->_vImag[i] *= reciprocal;
//     }
//   }
// }


// void FFT::ComplexToMagnitude() {
//   // vM is half the size of vReal and vImag
//   for (uint16_t i = 0; i < this->_samples; i++) {
//     this->_vReal[i] = sqrt(sq(this->_vReal[i]) + sq(this->_vImag[i]));
//   }
// }



// void FFT::DCRemoval() {
//   // calculate the mean of vData
//   real_t mean = 0;
//   for (uint16_t i = 0; i < this->_samples; i++) {
//     mean += this->_vReal[i];
//   }
//   mean /= this->_samples;
//   // Subtract the mean from vData
//   for (uint16_t i = 0; i < this->_samples; i++) {
//     this->_vReal[i] -= mean;
//   }
// }

// void FFT::Windowing(Window windowType, Direction dir) {
//   // Weighing factors are computed once before multiple use of FFT
//   // The weighing function is symmetric; half the weighs are recorded
//   real_t samplesMinusOne = (real_t(this->_samples) - 1.0);
//   for (uint16_t i = 0; i < (this->_samples >> 1); i++) {
//     real_t indexMinusOne = real_t(i);
//     real_t ratio = (indexMinusOne / samplesMinusOne);
//     real_t weighingFactor = 1.0;
//     // Compute and record weighting factor
//     switch (windowType) {
//     case Window::Rectangle: // rectangle (box car)
//       weighingFactor = 1.0;
//       break;
//     case Window::Hamming: // hamming
//       weighingFactor = 0.54 - (0.46 * cos(twoPi * ratio));
//       break;
//     case Window::Hann: // hann
//       weighingFactor = 0.54 * (1.0 - cos(twoPi * ratio));
//       break;
//     case Window::Triangle: // triangle (Bartlett)
//       weighingFactor =
//           1.0 - ((2.0 * abs(indexMinusOne - (samplesMinusOne / 2.0))) /
//                  samplesMinusOne);
//       break;
//     case Window::Nuttall: // nuttall
//       weighingFactor = 0.355768 - (0.487396 * (cos(twoPi * ratio))) +
//                        (0.144232 * (cos(fourPi * ratio))) -
//                        (0.012604 * (cos(sixPi * ratio)));
//       break;
//     case Window::Blackman: // blackman
//       weighingFactor = 0.42323 - (0.49755 * (cos(twoPi * ratio))) +
//                        (0.07922 * (cos(fourPi * ratio)));
//       break;
//     case Window::Blackman_Nuttall: // blackman nuttall
//       weighingFactor = 0.3635819 - (0.4891775 * (cos(twoPi * ratio))) +
//                        (0.1365995 * (cos(fourPi * ratio))) -
//                        (0.0106411 * (cos(sixPi * ratio)));
//       break;
//     case Window::Blackman_Harris: // blackman harris
//       weighingFactor = 0.35875 - (0.48829 * (cos(twoPi * ratio))) +
//                        (0.14128 * (cos(fourPi * ratio))) -
//                        (0.01168 * (cos(sixPi * ratio)));
//       break;
//     case Window::Flat_top: // flat top
//       weighingFactor = 0.2810639 - (0.5208972 * cos(twoPi * ratio)) +
//                        (0.1980399 * cos(fourPi * ratio));
//       break;
//     case Window::Welch: // welch
//       weighingFactor = 1.0 - sq((indexMinusOne - samplesMinusOne / 2.0) /
//                                 (samplesMinusOne / 2.0));
//       break;
//     }
//     if (dir == Direction::Forward) {
//       this->_vReal[i] *= weighingFactor;
//       this->_vReal[this->_samples - (i + 1)] *= weighingFactor;
//     } else {
//       this->_vReal[i] /= weighingFactor;
//       this->_vReal[this->_samples - (i + 1)] /= weighingFactor;
//     }
//   }
// }

// real_t FFT::MajorPeak() {
//   real_t maxY = 0;
//   uint16_t IndexOfMaxY = 0;
//   // If sampling_frequency = 2 * max_frequency in signal,
//   // value would be stored at position samples/2
//   for (uint16_t i = 1; i < ((this->_samples >> 1) + 1); i++) {
//     if ((this->_vReal[i - 1] < this->_vReal[i]) &&
//         (this->_vReal[i] > this->_vReal[i + 1])) {
//       if (this->_vReal[i] > maxY) {
//         maxY = this->_vReal[i];
//         IndexOfMaxY = i;
//       }
//     }
//   }
//   real_t delta =
//       0.5 *
//       ((this->_vReal[IndexOfMaxY - 1] - this->_vReal[IndexOfMaxY + 1]) /
//        (this->_vReal[IndexOfMaxY - 1] - (2.0 * this->_vReal[IndexOfMaxY]) +
//         this->_vReal[IndexOfMaxY + 1]));
//   real_t interpolatedX =
//       ((IndexOfMaxY + delta) * this->_samplingFrequency) / (this->_samples - 1);
//   if (IndexOfMaxY ==
//       (this->_samples >> 1)) // To improve calculation on edge values
//     interpolatedX =
//         ((IndexOfMaxY + delta) * this->_samplingFrequency) / (this->_samples);
//   // returned value: interpolated frequency peak apex
//   return (interpolatedX);
// }

// void FFT::MajorPeak(real_t *f, real_t *v) {
//   real_t maxY = 0;
//   uint16_t IndexOfMaxY = 0;
//   // If sampling_frequency = 2 * max_frequency in signal,
//   // value would be stored at position samples/2
//   for (uint16_t i = 1; i < ((this->_samples >> 1) + 1); i++) {
//     if ((this->_vReal[i - 1] < this->_vReal[i]) &&
//         (this->_vReal[i] > this->_vReal[i + 1])) {
//       if (this->_vReal[i] > maxY) {
//         maxY = this->_vReal[i];
//         IndexOfMaxY = i;
//       }
//     }
//   }
//   real_t delta =
//       0.5 *
//       ((this->_vReal[IndexOfMaxY - 1] - this->_vReal[IndexOfMaxY + 1]) /
//        (this->_vReal[IndexOfMaxY - 1] - (2.0 * this->_vReal[IndexOfMaxY]) +
//         this->_vReal[IndexOfMaxY + 1]));
//   real_t interpolatedX =
//       ((IndexOfMaxY + delta) * this->_samplingFrequency) / (this->_samples - 1);
//   if (IndexOfMaxY ==
//       (this->_samples >> 1)) // To improve calculation on edge values
//     interpolatedX =
//         ((IndexOfMaxY + delta) * this->_samplingFrequency) / (this->_samples);
//   // returned value: interpolated frequency peak apex
//   *f = interpolatedX;
// #if defined(ESP8266) || defined(ESP32)
//   *v = fabs(this->_vReal[IndexOfMaxY - 1] - (2.0 * this->_vReal[IndexOfMaxY]) +
//             this->_vReal[IndexOfMaxY + 1]);
// #else
//   *v = abs(this->_vReal[IndexOfMaxY - 1] - (2.0 * this->_vReal[IndexOfMaxY]) +
//            this->_vReal[IndexOfMaxY + 1]);
// #endif
// }

// real_t FFT::MajorPeakParabola() {
//   real_t maxY = 0;
//   uint16_t IndexOfMaxY = 0;
//   // If sampling_frequency = 2 * max_frequency in signal,
//   // value would be stored at position samples/2
//   for (uint16_t i = 1; i < ((this->_samples >> 1) + 1); i++) {
//     if ((this->_vReal[i - 1] < this->_vReal[i]) &&
//         (this->_vReal[i] > this->_vReal[i + 1])) {
//       if (this->_vReal[i] > maxY) {
//         maxY = this->_vReal[i];
//         IndexOfMaxY = i;
//       }
//     }
//   }

//   real_t freq = 0;
//   if (IndexOfMaxY > 0) {
//     // Assume the three points to be on a parabola
//     real_t a, b, c;
//     Parabola(IndexOfMaxY - 1, this->_vReal[IndexOfMaxY - 1], IndexOfMaxY,
//              this->_vReal[IndexOfMaxY], IndexOfMaxY + 1,
//              this->_vReal[IndexOfMaxY + 1], &a, &b, &c);

//     // Peak is at the middle of the parabola
//     real_t x = -b / (2 * a);

//     // And magnitude is at the extrema of the parabola if you want It...
//     // real_t y = a*x*x+b*x+c;

//     // Convert to frequency
//     freq = (x * this->_samplingFrequency) / (this->_samples);
//   }

//   return freq;
// }

// void FFT::Parabola(real_t x1, real_t y1, real_t x2, real_t y2, real_t x3,
//                           real_t y3, real_t *a, real_t *b, real_t *c) {
//   real_t reversed_denom = 1 / ((x1 - x2) * (x1 - x3) * (x2 - x3));

//   *a = (x3 * (y2 - y1) + x2 * (y1 - y3) + x1 * (y3 - y2)) * reversed_denom;
//   *b = (x3 * x3 * (y1 - y2) + x2 * x2 * (y3 - y1) + x1 * x1 * (y2 - y3)) *
//        reversed_denom;
//   *c = (x2 * x3 * (x2 - x3) * y1 + x3 * x1 * (x3 - x1) * y2 +
//         x1 * x2 * (x1 - x2) * y3) *
//        reversed_denom;
// }

// // Private functions

// void FFT::Swap(real_t *x, real_t *y) {
//   real_t temp = *x;
//   *x = *y;
//   *y = temp;
// }
