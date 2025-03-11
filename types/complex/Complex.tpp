#define COMPLEX_ARITHMETIC_OPERATOR(op)\
\
template <typename T, typename U>\
__fast_inline Complex_t<T> operator op (Complex_t<T> lhs, const Complex_t<U>& rhs) {\
    lhs op##= rhs;\
    return lhs;\
}\
\
template <typename T, typename U>\
__fast_inline Complex_t<T> operator op (Complex_t<T> lhs, const U & rhs) {\
    lhs op##= static_cast<T>(rhs);\
    return lhs;\
}\
\
template <typename T, typename U>\
__fast_inline Complex_t<T> operator op (const U & lhs, Complex_t<T> rhs) {\
    Complex_t<T>(static_cast<T>(lhs), T(0)) op##= rhs;\
    return lhs;\
}

COMPLEX_ARITHMETIC_OPERATOR(+)
COMPLEX_ARITHMETIC_OPERATOR(-)
COMPLEX_ARITHMETIC_OPERATOR(*)
COMPLEX_ARITHMETIC_OPERATOR(/)


template <typename T, typename U>
__fast_inline bool operator==(const Complex_t<T>& lhs, const Complex_t<U>& rhs) {
    return lhs.real == static_cast<T>(rhs.real) && lhs.imag == static_cast<T>(rhs.imag);
}

template <typename T>
__fast_inline bool operator!=(const Complex_t<T>& lhs, const Complex_t<T>& rhs) {
    return !(lhs == rhs);
}

#define COMPLEX_COMPARE_IM_OPERATOR(op) \
\
template <typename T, typename U> \
__fast_inline bool operator op (const Complex_t<T>& lhs, const U& rhs) { \
    T absrhs = static_cast<T>(abs(rhs)); \
    return lhs.norm() op (absrhs * absrhs); \
} \
\
template <typename T, typename U> \
__fast_inline bool operator op (const U& lhs, const Complex_t<T>& rhs) { \
    T abslhs = static_cast<T>(abs(lhs)); \
    return (abslhs * abslhs) op (rhs.norm()); \
}

COMPLEX_COMPARE_IM_OPERATOR(<)
COMPLEX_COMPARE_IM_OPERATOR(<=)
COMPLEX_COMPARE_IM_OPERATOR(>)
COMPLEX_COMPARE_IM_OPERATOR(>=)
COMPLEX_COMPARE_IM_OPERATOR(==)
COMPLEX_COMPARE_IM_OPERATOR(!=)

#undef COMPLEX_COMPARE_IM_OPERATOR

template <typename T>
__fast_inline Complex_t<T> proj(const Complex_t<T> & m){
    if (std::isinf(m.real) || std::isinf(m.imag)) {
        return Complex_t<T>(std::numeric_limits<T>::infinity(), T(0));
    }else return *m;
}

template <typename T>
__fast_inline Complex_t<T> conj(const Complex_t<T> & m){
    return Complex_t(m.real, -m.imag);
}

