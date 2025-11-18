#pragma once

#include <cstdint>
#include <type_traits>


template<typename T>
requires std::is_integral_v<T>
class [[nodiscard]] int_t{
private:
    T value;
public:
    explicit constexpr int_t(const T v) : value(v){
        static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8, "int_t only support 1, 2, 4, 8 bytes");
        // if constexpr(std::is_signed_v<T>) static_assert();
    }

    constexpr int_t(const int_t & v) : int_t(v.value){}
    constexpr int_t(int_t && v) : int_t(v.value){}

    __fast_inline constexpr int_t & operator=(const int_t & v){ value = v.value; return *this; }
    __fast_inline constexpr int_t & operator=(const T v){ value = v; return *this; }
    [[nodiscard]] explicit constexpr operator T(){ return value; }
    [[nodiscard]] explicit constexpr operator bool(){ return value; }

    __fast_inline constexpr int_t & operator++(){ ++value; return *this; }
    __fast_inline constexpr int_t & operator--(){ --value; return *this;}

    [[nodiscard]] __always_inline constexpr int_t operator+() const {return *this; }
    [[nodiscard]] __always_inline constexpr int_t operator-(){ value = -value; return *this;}

    [[nodiscard]] __always_inline constexpr int_t operator<<(const size_t bits){ return int_t(value << bits);}
    [[nodiscard]] __always_inline constexpr int_t operator>>(const size_t bits){ return int_t(value >> bits);}


    //#region compare
    #define INT_COMPARE_TEMPLATE(op)\
    constexpr bool operator op(const int_t & v) const { return value op v.value;}\

    INT_COMPARE_TEMPLATE(==)
    INT_COMPARE_TEMPLATE(!=)
    INT_COMPARE_TEMPLATE(>=)
    INT_COMPARE_TEMPLATE(>)
    INT_COMPARE_TEMPLATE(<=)
    INT_COMPARE_TEMPLATE(<)

    #undef INT_COMPARE_TEMPLATE
    //#endregion compare


    //#region op
    #define INT_OP_TEMPLATE(op)\
    [[nodiscard]] __inline constexpr int_t operator op(int_t other) const { return int_t(value op other.value);}\

    INT_OP_TEMPLATE(+)
    INT_OP_TEMPLATE(-)
    INT_OP_TEMPLATE(*)
    INT_OP_TEMPLATE(/)
    INT_OP_TEMPLATE(%)

    #undef INT_OP_TEMPLATE
    //#endregion op


    //#region self-op
    #define INT_SELF_OP_TEMPLATE(op)\
    __inline constexpr int_t & operator op(int_t other) { other op *this; return other;}\

    INT_SELF_OP_TEMPLATE(+=)
    INT_SELF_OP_TEMPLATE(-=)
    INT_SELF_OP_TEMPLATE(*=)
    INT_SELF_OP_TEMPLATE(/=)

    #undef INT_SELF_OP_TEMPLATE
    //#endregion self-op
};



using u8 = int_t<uint8_t>;
using u16 = int_t<uint16_t>;
using u32 = int_t<uint32_t>;
using u64 = int_t<uint64_t>;

using i8 = int_t<int8_t>;
using i16 = int_t<int16_t>;
using i32 = int_t<int32_t>;
using i64 = int_t<int64_t>;