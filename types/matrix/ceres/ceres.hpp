#pragma once

#include "types/matrix/matrix_static.hpp"


namespace ymd{
template <arithmetic T, size_t N>
struct Jet{
  Matrix<T, N, 1> v;
  T a;
  Jet() : a(0) {}
  Jet(const T value) : a(value) { v.setZero(); }

  __fast_inline Jet(const T value, const Matrix<T, N, 1>& v_)
      : a(value), v(v_){}

  __fast_inline constexpr T & operator [](const size_t n){
    return v.at(n, 0);
  }

  __fast_inline constexpr const T & operator [](const size_t n) const {
    return v.at(n, 0);
  } 

  Jet(const T value, const int index){
    v.setZero();
    a = value;
    v(index, 0) = T(1);
  }

  void init(const T value, const int index){
    v.setZero();
    a = value;
    v(index, 0) = T(1);
  }
};
/****************jet overload******************/
// for the camera BA,the autodiff only need overload the operator :jet+jet
// number+jet -jet jet-number jet*jet number/jet jet/jet sqrt(jet) cos(jet)
// sin(jet)  +=(jet) overload jet + jet
template <arithmetic T, size_t N>
inline Jet<T, N> operator+(const Jet<T, N>& A, const Jet<T, N>& B){
  return Jet<T, N>(A.a + B.a, A.v + B.v);
}  // end jet+jet

// overload number + jet
template <arithmetic T, size_t N>
inline Jet<T, N> operator+(T A, const Jet<T, N>& B)
{
  return Jet<T, N>(A + B.a, B.v);
}  // end number+jet

template <arithmetic T, size_t N>
inline Jet<T, N> operator+(const Jet<T, N>& B, T A)
{
  return Jet<T, N>(A + B.a, B.v);
}  // end number+jet

// overload jet-number
template <arithmetic T, size_t N>
inline Jet<T, N> operator-(const Jet<T, N>& A, T B)
{
  return Jet<T, N>(A.a - B, A.v);
}
// overload number * jet because jet *jet need A.a *B.v+B.a*A.v.So the number
// *jet is required
template <arithmetic T, size_t N>
inline Jet<T, N> operator*(T A, const Jet<T, N>& B)
{
  return Jet<T, N>(A * B.a, A * B.v);
}
template <arithmetic T, size_t N>
inline Jet<T, N> operator*(const Jet<T, N>& A, T B)
{
  return Jet<T, N>(B * A.a, B * A.v);
}
// overload -jet
template <arithmetic T, size_t N>
inline Jet<T, N> operator-(const Jet<T, N>& A)
{
  return Jet<T, N>(-A.a, -A.v);
}
template <arithmetic T, size_t N>
inline Jet<T, N> operator-(T A, const Jet<T, N>& B)
{
  return Jet<T, N>(A - B.a, -B.v);
}
template <arithmetic T, size_t N>
inline Jet<T, N> operator-(const Jet<T, N>& A, const Jet<T, N>& B)
{
  return Jet<T, N>(A.a - B.a, A.v - B.v);
}
// overload jet*jet
template <arithmetic T, size_t N>
inline Jet<T, N> operator*(const Jet<T, N>& A, const Jet<T, N>& B)
{
  return Jet<T, N>(A.a * B.a, B.a * A.v + A.a * B.v);
}
// overload number/jet
template <arithmetic T, size_t N>
inline Jet<T, N> operator/(T A, const Jet<T, N>& B)
{
  return Jet<T, N>(A / B.a, -A * B.v / (B.a * B.a));
}
// overload jet/jet
template <arithmetic T, size_t N>
inline Jet<T, N> operator/(const Jet<T, N>& A, const Jet<T, N>& B)
{
  // This uses:
  //
  //   a + u   (a + u)(b - v)   (a + u)(b - v)
  //   ----- = -------------- = --------------
  //   b + v   (b + v)(b - v)        b^2
  //
  // which holds because v*v = 0.
  const T a_inverse = T(1) / B.a;
  const T abyb = A.a * a_inverse;
  return Jet<T, N>(abyb, (A.v - abyb * B.v) * a_inverse);
}
// sqrt(jet)
template <arithmetic T, size_t N>
inline Jet<T, N> sqrt(const Jet<T, N>& A){
  T temp = sqrt(A.a);

  return Jet<T, N>(temp, T(1) / (T(2) * temp) * A.v);
}
// cos(jet)
template <arithmetic T, size_t N>
inline Jet<T, N> cos(const Jet<T, N>& A){
  return Jet<T, N>(cos(A.a), -sin(A.a) * A.v);
}
template <arithmetic T, size_t N>
inline Jet<T, N> sin(const Jet<T, N>& A)
{
  return Jet<T, N>(sin(A.a), cos(A.a) * A.v);
}
template <arithmetic T, size_t N>
inline bool operator>(const Jet<T, N>& f, const Jet<T, N>& g)
{
  return f.a > g.a;
}

template<arithmetic T, size_t N>
__inline OutputStream & operator<<(OutputStream & os, const Jet<T, N> & jet){
    os << os.brackets<'['>();
    for (size_t _j = 0; _j < N; _j++) {
        os << jet[_j];
        if(_j == N - 1) break;
        os << os.splitter();
    }
    os << os.brackets<']'>();
    return os;
}

}