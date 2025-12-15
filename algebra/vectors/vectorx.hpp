#include "core/platform.hpp"


namespace ymd{
template<typename T, size_t N>
struct [[nodiscard]] VectorX final{
    using Storage = std::array<T, N>;
private:
    Storage sto_ {};
    explicit constexpr VectorX() = default;
public:
    static constexpr VectorX from_uninitialized() {
        return VectorX();
    }

    explicit constexpr VectorX(const std::initializer_list<T>& init) {
        size_t i = 0;
        for (const auto& value : init) {
            if (i >= N) break;
            sto_[i++] = value;
        }
    }

    explicit constexpr VectorX(const std::span<const T, N>& init) {
        size_t i = 0;
        for (const auto& value : init) {
            if (i >= N) break;
            sto_[i++] = value;
        }
    }
    
    constexpr T & operator[](size_t i){
        return sto_[i];
    }

    constexpr const T & operator[](size_t i) const {
        return sto_[i];
    }

    constexpr size_t size() const {
        return N;
    }

    constexpr const T * begin() const {
        return sto_.data();
    }

    constexpr const T * end() const {
        return sto_.data() + N;
    }

    constexpr T * begin() {
        return sto_.data();
    }

    constexpr T * end() {
        return sto_.data() + N;
    }

    constexpr std::span<const T> iter() const {
        return std::span<const T>(sto_.data(), N);
    }
    
    // Addition operator
    constexpr VectorX operator+(const VectorX& other) const {
        VectorX result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = sto_[i] + other[i];
        }
        return result;
    }
    
    // Subtraction operator
    constexpr VectorX operator-(const VectorX& other) const {
        VectorX result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = sto_[i] - other[i];
        }
        return result;
    }

    constexpr VectorX substract_bounded(const VectorX& other) const {
        VectorX result;
        for (size_t i = 0; i < N; ++i) {
            if constexpr (std::is_integral_v<T>){
                result[i] = CLAMP(
                    int(sto_[i] - other[i]), 
                    int(std::numeric_limits<T>::min()), 
                    int(std::numeric_limits<T>::max())
                );
            }else{
                result[i] = CLAMP(
                    sto_[i] - other[i], 
                    std::numeric_limits<T>::min(), 
                    std::numeric_limits<T>::max()
                );
            }
        }
        return result;
    }

    constexpr VectorX addition_bounded(const VectorX& other) const { 
        VectorX result;
        for (size_t i = 0; i < size(); ++i) {
            if constexpr(std::is_integral<T>::value) {
                result[i] = CLAMP(
                    int(sto_[i] + other[i]), 
                    int(std::numeric_limits<T>::min()), 
                    int(std::numeric_limits<T>::max())
                );
            }else{
                result[i] = CLAMP(
                    sto_[i] + other[i], 
                    std::numeric_limits<T>::min(), 
                    std::numeric_limits<T>::max()
                );
            }
        }
    }

    // Subtraction operator
    constexpr VectorX operator-() const {
        VectorX result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = -sto_[i];
        }
        return result;
    }
    
    // In-place addition
    constexpr VectorX& operator+=(const VectorX& other) {
        for (size_t i = 0; i < N; ++i) {
            sto_[i] += other[i];
        }
        return *this;
    }
    
    // In-place subtraction
    constexpr VectorX& operator-=(const VectorX& other) {
        for (size_t i = 0; i < N; ++i) {
            sto_[i] -= other[i];
        }
        return *this;
    }

    #define DEF_BINARY_OPERATOR(op)\
    constexpr VectorX<bool, N >operator op (const T value) const{\
        VectorX<bool, N> result = VectorX<bool, N>::from_uninitialized();\
        for (size_t i = 0; i < N; ++i) {\
            result[i] = (sto_[i] op value);\
        }\
        return result;\
    }\

    DEF_BINARY_OPERATOR(==)
    DEF_BINARY_OPERATOR(!=)
    DEF_BINARY_OPERATOR(>=)
    DEF_BINARY_OPERATOR(<=)
    DEF_BINARY_OPERATOR(>)
    DEF_BINARY_OPERATOR(<)

    #undef DEF_BINARY_OPERATOR

    friend OutputStream &operator<<(OutputStream &os, const VectorX &vec) {
        return os << vec.iter();
    }

};
}