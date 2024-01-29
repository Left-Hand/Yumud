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
    using CommonType = typename std::common_type<T, U>::type; \
    U absrhs = std::abs(rhs); \
    return static_cast<CommonType>(lhs.norm()) op static_cast<CommonType>(absrhs * absrhs); \
} \
\
template <typename T, typename U> \
__fast_inline bool operator op (const U& lhs, const Complex_t<T>& rhs) { \
    using CommonType = typename std::common_type<T, U>::type; \
    U abslhs = std::abs(lhs); \
    return static_cast<CommonType>(abslhs * abslhs) op static_cast<CommonType>(rhs.norm()); \
}

COMPLEX_COMPARE_IM_OPERATOR(<)
COMPLEX_COMPARE_IM_OPERATOR(<=)
COMPLEX_COMPARE_IM_OPERATOR(>)
COMPLEX_COMPARE_IM_OPERATOR(>=)
COMPLEX_COMPARE_IM_OPERATOR(==)
COMPLEX_COMPARE_IM_OPERATOR(!=)

template <typename T>
__fast_inline Complex_t<T> Complex_t<T>::proj() {
    if (std::isinf(real) || std::isinf(imag)) {
        return Complex_t<T>(std::numeric_limits<T>::infinity(), T(0));
    }else return *this;
}