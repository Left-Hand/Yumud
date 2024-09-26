#pragma once

#include "types/matrix/matrix.hpp"

template <int N>
struct jet
{
  Matrix_t<real_t> v;
  real_t a;
  jet() : a(0.0) {}
  jet(const real_t value) : a(value) { v.setZero(); }
  __fast_inline jet(const real_t value, const Matrix_t<real_t>& v_)
      : a(value), v(v_)
  {
  }
  jet(const real_t value, const int index)
  {
    v.setZero();
    a = value;
    v(index, 0) = 1.0;
  }
  void init(const real_t value, const int index)
  {
    v.setZero();
    a = value;
    v(index, 0) = 1.0;
  }
};
/****************jet overload******************/
// for the camera BA,the autodiff only need overload the operator :jet+jet
// number+jet -jet jet-number jet*jet number/jet jet/jet sqrt(jet) cos(jet)
// sin(jet)  +=(jet) overload jet + jet
template <int N>
inline jet<N> operator+(const jet<N>& A, const jet<N>& B)
{
  return jet<N>(A.a + B.a, A.v + B.v);
}  // end jet+jet

// overload number + jet
template <int N>
inline jet<N> operator+(real_t A, const jet<N>& B)
{
  return jet<N>(A + B.a, B.v);
}  // end number+jet

template <int N>
inline jet<N> operator+(const jet<N>& B, real_t A)
{
  return jet<N>(A + B.a, B.v);
}  // end number+jet

// overload jet-number
template <int N>
inline jet<N> operator-(const jet<N>& A, real_t B)
{
  return jet<N>(A.a - B, A.v);
}
// overload number * jet because jet *jet need A.a *B.v+B.a*A.v.So the number
// *jet is required
template <int N>
inline jet<N> operator*(real_t A, const jet<N>& B)
{
  return jet<N>(A * B.a, A * B.v);
}
template <int N>
inline jet<N> operator*(const jet<N>& A, real_t B)
{
  return jet<N>(B * A.a, B * A.v);
}
// overload -jet
template <int N>
inline jet<N> operator-(const jet<N>& A)
{
  return jet<N>(-A.a, -A.v);
}
template <int N>
inline jet<N> operator-(real_t A, const jet<N>& B)
{
  return jet<N>(A - B.a, -B.v);
}
template <int N>
inline jet<N> operator-(const jet<N>& A, const jet<N>& B)
{
  return jet<N>(A.a - B.a, A.v - B.v);
}
// overload jet*jet
template <int N>
inline jet<N> operator*(const jet<N>& A, const jet<N>& B)
{
  return jet<N>(A.a * B.a, B.a * A.v + A.a * B.v);
}
// overload number/jet
template <int N>
inline jet<N> operator/(real_t A, const jet<N>& B)
{
  return jet<N>(A / B.a, -A * B.v / (B.a * B.a));
}
// overload jet/jet
template <int N>
inline jet<N> operator/(const jet<N>& A, const jet<N>& B)
{
  // This uses:
  //
  //   a + u   (a + u)(b - v)   (a + u)(b - v)
  //   ----- = -------------- = --------------
  //   b + v   (b + v)(b - v)        b^2
  //
  // which holds because v*v = 0.
  const real_t a_inverse = 1.0 / B.a;
  const real_t abyb = A.a * a_inverse;
  return jet<N>(abyb, (A.v - abyb * B.v) * a_inverse);
}
// sqrt(jet)
template <int N>
inline jet<N> sqrt(const jet<N>& A)
{
  real_t t = std::sqrt(A.a);

  return jet<N>(t, real_t(1) / (real_t(2) * t) * A.v);
}
// cos(jet)
template <int N>
inline jet<N> cos(const jet<N>& A)
{
  return jet<N>(std::cos(A.a), -std::sin(A.a) * A.v);
}
template <int N>
inline jet<N> sin(const jet<N>& A)
{
  return jet<N>(std::sin(A.a), std::cos(A.a) * A.v);
}
template <int N>
inline bool operator>(const jet<N>& f, const jet<N>& g)
{
  return f.a > g.a;
}
