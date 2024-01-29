template <typename T, typename U>
Complex_t<T> operator+(Complex_t<T> lhs, const Complex_t<U>& rhs) {
    lhs += rhs;
    return lhs;
}

template <typename T, typename U>
Complex_t<T> operator+(Complex_t<T> lhs, const U & rhs) {
    lhs += Complex_t<T>(rhs, T());
    return lhs;
}

template <typename T, typename U>
Complex_t<T> operator-(Complex_t<T> lhs, const Complex_t<T>& rhs) {
    lhs -= rhs;
    return lhs;
}

template <typename T>
Complex_t<T> operator*(Complex_t<T> lhs, const Complex_t<T>& rhs) {
    lhs *= rhs;
    return lhs;
}

template <typename T>
Complex_t<T> operator/(Complex_t<T> lhs, const Complex_t<T>& rhs) {
    lhs /= rhs;
    return lhs;
}

template <typename T>
bool operator==(const Complex_t<T>& lhs, const Complex_t<T>& rhs) {
    return lhs.real() == rhs.real() && lhs.imag() == rhs.imag();
}

template <typename T>
bool operator!=(const Complex_t<T>& lhs, const Complex_t<T>& rhs) {
    return !(lhs == rhs);
}


template <typename T>
Complex_t<T> operator/(const Complex_t<T>& lhs, const Complex_t<T>& rhs) {
    T denominator = rhs.real() * rhs.real() + rhs.imag() * rhs.imag();
    T realPart = (lhs.real() * rhs.real() + lhs.imag() * rhs.imag()) / denominator;
    T imagPart = (lhs.imag() * rhs.real() - lhs.real() * rhs.imag()) / denominator;
    return Complex_t<T>(realPart, imagPart);
}

template <typename T, typename U>
bool operator<(const Complex_t<T>& lhs, const Complex_t<U>& rhs) {
    using CommonType = typename std::common_type<T, U>::type;
    return static_cast<CommonType>(lhs.norm()) < static_cast<CommonType>(rhs.norm());
}


template <typename T, typename U>
bool operator<(const Complex_t<T>& lhs, const U& rhs) {
    using CommonType = typename std::common_type<T, U>::type;
    U absrhs = std::abs(rhs);
    return static_cast<CommonType>(lhs.norm()) < static_cast<CommonType>(absrhs * absrhs);
}

template <typename T>
__fast_inline Complex_t<T> Complex_t<T>::proj() {
    if (std::isinf(real) || std::isinf(imag)) {
        return Complex_t<T>(std::numeric_limits<T>::infinity(), T(0));
    }else return *this;
}