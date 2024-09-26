#pragma once

#include "types/matrix/matrix.hpp"

template <arithmetic T, size_t N>
struct Jet_t
{
  Matrix_t<T> v{N, 1};
  T a;
  Jet_t() : a(0.0) {}
  Jet_t(const T value) : a(value) { v.setZero(); }
  __fast_inline Jet_t(const T value, const Matrix_t<T>& v_)
      : a(value), v(v_)
  {
  }

  __fast_inline constexpr T & operator [](const size_t n){
    return v(n, 0);
  }

  __fast_inline constexpr const T & operator [](const size_t n) const {
    return v(n, 0);
  } 
  Jet_t(const T value, const int index)
  {
    v.setZero();
    a = value;
    v(index, 0) = 1.0;
  }
  void init(const T value, const int index)
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
template <arithmetic T, size_t N>
inline Jet_t<T, N> operator+(const Jet_t<T, N>& A, const Jet_t<T, N>& B)
{
  return Jet_t<T, N>(A.a + B.a, A.v + B.v);
}  // end jet+jet

// overload number + jet
template <arithmetic T, size_t N>
inline Jet_t<T, N> operator+(T A, const Jet_t<T, N>& B)
{
  return Jet_t<T, N>(A + B.a, B.v);
}  // end number+jet

template <arithmetic T, size_t N>
inline Jet_t<T, N> operator+(const Jet_t<T, N>& B, T A)
{
  return Jet_t<T, N>(A + B.a, B.v);
}  // end number+jet

// overload jet-number
template <arithmetic T, size_t N>
inline Jet_t<T, N> operator-(const Jet_t<T, N>& A, T B)
{
  return Jet_t<T, N>(A.a - B, A.v);
}
// overload number * jet because jet *jet need A.a *B.v+B.a*A.v.So the number
// *jet is required
template <arithmetic T, size_t N>
inline Jet_t<T, N> operator*(T A, const Jet_t<T, N>& B)
{
  return Jet_t<T, N>(A * B.a, A * B.v);
}
template <arithmetic T, size_t N>
inline Jet_t<T, N> operator*(const Jet_t<T, N>& A, T B)
{
  return Jet_t<T, N>(B * A.a, B * A.v);
}
// overload -jet
template <arithmetic T, size_t N>
inline Jet_t<T, N> operator-(const Jet_t<T, N>& A)
{
  return Jet_t<T, N>(-A.a, -A.v);
}
template <arithmetic T, size_t N>
inline Jet_t<T, N> operator-(T A, const Jet_t<T, N>& B)
{
  return Jet_t<T, N>(A - B.a, -B.v);
}
template <arithmetic T, size_t N>
inline Jet_t<T, N> operator-(const Jet_t<T, N>& A, const Jet_t<T, N>& B)
{
  return Jet_t<T, N>(A.a - B.a, A.v - B.v);
}
// overload jet*jet
template <arithmetic T, size_t N>
inline Jet_t<T, N> operator*(const Jet_t<T, N>& A, const Jet_t<T, N>& B)
{
  return Jet_t<T, N>(A.a * B.a, B.a * A.v + A.a * B.v);
}
// overload number/jet
template <arithmetic T, size_t N>
inline Jet_t<T, N> operator/(T A, const Jet_t<T, N>& B)
{
  return Jet_t<T, N>(A / B.a, -A * B.v / (B.a * B.a));
}
// overload jet/jet
template <arithmetic T, size_t N>
inline Jet_t<T, N> operator/(const Jet_t<T, N>& A, const Jet_t<T, N>& B)
{
  // This uses:
  //
  //   a + u   (a + u)(b - v)   (a + u)(b - v)
  //   ----- = -------------- = --------------
  //   b + v   (b + v)(b - v)        b^2
  //
  // which holds because v*v = 0.
  const T a_inverse = 1.0 / B.a;
  const T abyb = A.a * a_inverse;
  return Jet_t<T, N>(abyb, (A.v - abyb * B.v) * a_inverse);
}
// sqrt(jet)
template <arithmetic T, size_t N>
inline Jet_t<T, N> sqrt(const Jet_t<T, N>& A)
{
  T t = std::sqrt(A.a);

  return Jet_t<T, N>(t, T(1) / (T(2) * t) * A.v);
}
// cos(jet)
template <arithmetic T, size_t N>
inline Jet_t<T, N> cos(const Jet_t<T, N>& A)
{
  return Jet_t<T, N>(std::cos(A.a), -std::sin(A.a) * A.v);
}
template <arithmetic T, size_t N>
inline Jet_t<T, N> sin(const Jet_t<T, N>& A)
{
  return Jet_t<T, N>(std::sin(A.a), std::cos(A.a) * A.v);
}
template <arithmetic T, size_t N>
inline bool operator>(const Jet_t<T, N>& f, const Jet_t<T, N>& g)
{
  return f.a > g.a;
}

template<arithmetic T, size_t N>
__inline OutputStream & operator<<(OutputStream & os, const Jet_t<T, N> & jet){
    os << "[ ";
    for (size_t _j = 0; _j < N; _j++) {
        os << jet[_j];
        if(_j == N - 1) break;
        os << ", ";
    }
    os << " ]";
    return os;
}

// template<size_t N>
// using Jet<N> = Jet_t<real_t, N>;

// template<size_t N>
// using JetF<N> = Jet_t<float, N>;

// template<size_t N>
// using JetD<N> = Jet_t<double, N>;