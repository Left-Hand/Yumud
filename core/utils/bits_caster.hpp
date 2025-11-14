#pragma once

#include "core/constants/enums.hpp"
#include <bit>
#include <span>

namespace ymd{

namespace details{
template<typename T>
struct _bits_type{
    using type = void;
};

template<typename T>
requires requires {
    T::bits;
}
struct _bits_type<T>{
    using type = decltype(T::bits);
};

template<typename T>
struct _from_bits_argu_type;

template<typename Ret, typename Arg>
struct _from_bits_argu_type<Ret(*)(Arg)> {
    using type = Arg;
};

template<typename T>
requires requires {
    T::from_bits(0);
}
struct _bits_type<T> {
private:
    // 获取 from_bits 的函数类型
    using from_bits_func_type = decltype(&T::from_bits);
public:
    // 提取参数类型
    using type = typename _from_bits_argu_type<from_bits_func_type>::type;
};

template<size_t I, typename Tup>
requires (I < std::tuple_size_v<Tup>)
struct _accumulate_bytes_of_bits_ctorable_v{
    //特定位置的元素
    using T = std::tuple_element_t<I, Tup>;

    //其他元素之和加上这个元素的大小
    static constexpr size_t value = []{
        if constexpr (I > 0){
            return (_accumulate_bytes_of_bits_ctorable_v<I-1, Tup>::value + sizeof(typename _bits_type<T>::type));
        }else{
            return sizeof(typename _bits_type<T>::type);
        }
    }();
};

};




template<typename Tup>
static constexpr size_t total_bytes_of_bits_ctorable_v = []{
    constexpr size_t N = std::tuple_size_v<Tup>;
    if constexpr (N > 0){
        return (details::_accumulate_bytes_of_bits_ctorable_v<N-1, Tup>::value);
    }else{
        return 0;
    }
}();

template<size_t I, typename Tup>
requires (I < std::tuple_size_v<Tup>)
static constexpr size_t offset_of_bits_ctorable_v =[]{
    if constexpr (I > 0){
        return (details::_accumulate_bytes_of_bits_ctorable_v<I-1, Tup>::value);
    }else{
        return 0;
    }
}();


template<typename T>
using bits_type_t = typename details::_bits_type<T>::type;

template<Endian::Kind E, typename D, size_t Extent>
[[nodiscard]] static constexpr D bytes_to_int(const std::span<const uint8_t, Extent> bytes){
    static_assert((Extent == sizeof(D)) || (Extent == std::dynamic_extent),
        "static extent must equal to sizeof(D)"
    );
    using UD = std::make_unsigned_t<D>;

    UD sum = 0;

    constexpr auto _LSB = Endian(Endian::LSB);
    if constexpr (Endian(E) == _LSB){
        for(auto it = bytes.begin(); it != bytes.end(); it++){
            sum = static_cast<UD>(static_cast<UD>(sum << 8) | static_cast<UD>(*it));
        }
    }else{
        for(auto it = bytes.rbegin(); it != bytes.rend(); it++){
            sum = static_cast<UD>(static_cast<UD>(sum << 8) | static_cast<UD>(*it));
        }
    }
    return std::bit_cast<D>(sum);
}

template<typename D, size_t Extent>
[[nodiscard]] static constexpr D le_bytes_to_int(const std::span<const uint8_t, Extent> bytes){
    return bytes_to_int<LSB, D, Extent>(bytes);
}

template<typename D, size_t Extent>
[[nodiscard]] static constexpr D be_bytes_to_int(const std::span<const uint8_t, Extent> bytes){
    return bytes_to_int<MSB, D, Extent>(bytes);
}

//可以将整数完成bits构造
template<typename D>
struct [[nodiscard]] IntoBitsCtor{
    D bits;

    //no explicit
    template<typename T>
    requires (std::is_same_v<bits_type_t<T>, D>)
    [[nodiscard]] constexpr operator T() const {
        return into_obj<T>();
    }
private:
    template<typename T>
    requires requires {T::bits;}
    [[nodiscard]] constexpr T into_obj() const {
        return T{.bits = bits};
    }

    template<typename T>
    requires requires {T::from_bits(0);}
    [[nodiscard]] constexpr T into_obj() const {
        return T::from_bits(bits);
    }
};

template<typename D>
IntoBitsCtor(D) -> IntoBitsCtor<D>;



//可以将字节片段完成从bits构造
template<Endian::Kind E, size_t Extent>
struct [[nodiscard]] BytesCtorBits{

    constexpr explicit BytesCtorBits(
        const std::span<const uint8_t, Extent> bytes
    ):  
        bytes_(bytes) {}

    //no explicit
    template<typename T, typename D = bits_type_t<T>>
    requires (not std::is_same_v<D, void>)
    [[nodiscard]] constexpr operator T() const {
        return T(IntoBitsCtor<D>(bytes_to_int<E, D>(bytes_)));
    }
private:
    std::span<const uint8_t, Extent> bytes_;
};

template<Endian::Kind E, size_t Extents>
BytesCtorBits(std::span<const uint8_t, Extents>) -> BytesCtorBits<E, Extents>;


template<Endian::Kind E, size_t Extents>
[[nodiscard]] static constexpr auto byte_ctor_bits(std::span<const uint8_t, Extents> bytes) {
    return BytesCtorBits<E, Extents>(bytes);
}

template<size_t Extents>
[[nodiscard]] static constexpr BytesCtorBits<Endian::LSB, Extents> le_bytes_ctor_bits(
    const std::span<const uint8_t, Extents> bytes
){
    return BytesCtorBits<Endian::LSB, Extents>(bytes);
}

template<size_t Extents>
[[nodiscard]] static constexpr BytesCtorBits<Endian::MSB, Extents> be_bytes_ctor_bits(
    const std::span<const uint8_t, Extents> bytes
){
    return BytesCtorBits<Endian::MSB, Extents>(bytes);
}




}