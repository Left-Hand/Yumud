#pragma once

#include "core/constants/enums.hpp"
#include "core/utils/data_iter.hpp"
#include "core/utils/Option.hpp"
#include <bit>
#include <span>

namespace ymd{

template<size_t Extents>
struct PaddingZero{
    using Self = PaddingZero;
    [[nodiscard]] constexpr RepeatIter<uint8_t, Extents> to_bytes() const {
        return RepeatIter<uint8_t, Extents>(0, Extents);
    };

    [[nodiscard]] static constexpr Self from_bytes(std::span<const uint8_t, Extents>){;}
};

namespace details{
template<typename T>
struct _from_bits{
    using obj_type = void;
};

template<typename T>
requires requires {
    T::bits;
}
struct _from_bits<T>{
    using bits_type = decltype(T::bits);
    [[nodiscard]] static constexpr T into_obj(const bits_type bits){
        return T{.bits = bits};
    }
};

template<typename T>
requires (std::is_integral_v<T>)
struct _from_bits<T>{
    using bits_type = T;

    [[nodiscard]] static constexpr T into_obj(const bits_type bits){
        return bits;
    }
};

template<>
struct _from_bits<float>{
    static_assert(sizeof(float) == 4);
    using bits_type = uint32_t;

    [[nodiscard]] static constexpr float into_obj(const bits_type bits){
        return std::bit_cast<float>(bits);
    } 
};

template<>
struct _from_bits<double>{
    static_assert(sizeof(double) == 8);
    using bits_type = uint64_t;

    [[nodiscard]] static constexpr double into_obj(const bits_type bits){
        return std::bit_cast<double>(bits);
    } 
};


template<typename T>
struct _from_bits_argu_t;

template<typename Ret, typename Arg>
struct _from_bits_argu_t<Ret(*)(Arg)> {
    using type = Arg;
};

template<typename T>
requires requires {
    T::from_bits(0);
}
struct _from_bits<T> {
private:
    // 获取 from_bits 的函数类型
    using from_bits_func_type = decltype(&T::from_bits);
public:
    // 提取参数类型
    using bits_type = typename _from_bits_argu_t<from_bits_func_type>::type;

    [[nodiscard]] static constexpr T into_obj(const bits_type bits){
        return T::from_bits(bits);
    }
};

template<size_t I, typename Tup>
requires (I < std::tuple_size_v<Tup>)
struct _accumulate_bytes_of_bits_ctorable_v{
    //特定位置的元素
    using T = std::tuple_element_t<I, Tup>;

    //其他元素之和加上这个元素的大小
    static constexpr size_t value = []{
        if constexpr (I > 0){
            return (_accumulate_bytes_of_bits_ctorable_v<I-1, Tup>::value + sizeof(typename _from_bits<T>::bits_type));
        }else{
            return sizeof(typename _from_bits<T>::bits_type);
        }
    }();
};



template<typename T>
struct _as_bits{
    using type = void;
};

template<typename T>
requires requires {
    T::bits;
}
struct _as_bits<T>{
    using bits_type = decltype(T::bits);
    [[nodiscard]] static constexpr bits_type into_bits(const T & obj){
        return obj.bits;
    }
};

template<typename T>
requires (std::is_integral_v<T>)
struct _as_bits<T>{
    using bits_type = T;

    [[nodiscard]] static constexpr bits_type into_bits(const T & obj){
        return std::bit_cast<bits_type>(obj);
    }
};

template<typename T>
requires (std::is_floating_point_v<T>)
struct _as_bits<T>{
    using bits_type = std::conditional_t<std::is_same_v<T, float>, uint32_t, uint64_t>;

    [[nodiscard]] static constexpr bits_type into_bits(const T & obj){
        return std::bit_cast<bits_type>(obj);
    } 
};

template<typename T>
requires requires{
    T::as_bits();
}
struct _as_bits<T>{
    using bits_type = std::invoke_result_t<decltype(T::as_bits)>;
    [[nodiscard]] static constexpr bits_type into_bits(const T & obj){
        return obj.as_bits();
    }
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
using from_bits_t = typename details::_from_bits<T>::bits_type;

template<typename T>
using as_bits_t = typename details::_as_bits<T>::bits_type;

template<typename T, typename D = from_bits_t<T>>
[[nodiscard]] static constexpr T obj_from_bits(const D bits){
    return details::_from_bits<T>::into_obj(bits);
}

template<typename T, typename D = as_bits_t<T>>
[[nodiscard]] static constexpr D obj_as_bits(T && obj){
    return details::_as_bits<T>::into_bits(std::forward<T>(obj));
}


template<std::endian E, typename D, size_t Extent>
requires ((Extent == sizeof(D)) || (Extent == std::dynamic_extent))
[[nodiscard]] static constexpr D bytes_to_int(const std::span<const uint8_t, Extent> bytes){
    static_assert((Extent == sizeof(D)) || (Extent == std::dynamic_extent),
        "static extent must equal to sizeof(D)"
    );
    using UD = std::make_unsigned_t<D>;

    UD sum = 0;

    constexpr auto _LSB = std::endian::little;
    if constexpr (std::endian(E) == _LSB){
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
    return bytes_to_int<std::endian::little, D, Extent>(bytes);
}

template<typename D, size_t Extent>
[[nodiscard]] static constexpr D be_bytes_to_int(const std::span<const uint8_t, Extent> bytes){
    return bytes_to_int<std::endian::big, D, Extent>(bytes);
}

//可以将整数完成bits构造
template<typename D>
struct [[nodiscard]] IntoBitsCtor{
    D bits;

    //no explicit
    template<typename T>
    requires (std::is_same_v<from_bits_t<T>, D>)
    [[nodiscard]] constexpr operator T() const {
        return obj_from_bits<T>(bits);
    }
};

template<typename D>
IntoBitsCtor(D) -> IntoBitsCtor<D>;



//可以将字节片段完成从bits构造
template<std::endian E, size_t Extent>
struct [[nodiscard]] BytesCtorBits{

    constexpr explicit BytesCtorBits(
        const std::span<const uint8_t, Extent> bytes
    ):  
        bytes_(bytes) {}

    //no explicit
    template<typename T, typename D = from_bits_t<T>>
    requires (Extent != std::dynamic_extent)
    [[nodiscard]] constexpr operator T() const {
        static_assert(Extent == sizeof(D), "Extent != sizeof(D)");
        const auto bits = bytes_to_int<E, D>(bytes_);
        return T(IntoBitsCtor<D>(bits));
    }

    
    //no explicit
    template<typename T, typename D = from_bits_t<T>>
    requires (Extent == std::dynamic_extent)
    [[deprecated("use try_into instead for better runtime safety")]]
    [[nodiscard]] constexpr operator T() const {
        if(Extent != sizeof(D)) [[unlikely]] __builtin_abort();
        const auto bits = bytes_to_int<E, D>(bytes_.template subspan<0, sizeof(D)>());
        return T(IntoBitsCtor<D>(bits));
    }

    template<typename T, typename D = from_bits_t<T>>
    [[nodiscard]] constexpr Option<T> try_into() const{
        if constexpr (Extent == std::dynamic_extent){
            if(bytes_.size() != sizeof(D)) [[unlikely]] return None;
        }
        const auto bits = bytes_to_int<E, T>(bytes_);
        return Some(IntoBitsCtor<T>(bits));
    }
private:
    std::span<const uint8_t, Extent> bytes_;
};

template<std::endian E, size_t Extents>
BytesCtorBits(std::span<const uint8_t, Extents>) -> BytesCtorBits<E, Extents>;


template<std::endian E, size_t Extents>
[[nodiscard]] static constexpr auto make_bytes_into_bits_caster(std::span<const uint8_t, Extents> bytes) {
    return BytesCtorBits<E, Extents>(bytes);
}

template<size_t Extents>
[[nodiscard]] static constexpr BytesCtorBits<std::endian::little, Extents> le_bytes_ctor_bits(
    const std::span<const uint8_t, Extents> bytes
){
    return BytesCtorBits<std::endian::little, Extents>(bytes);
}

template<size_t Extents>
[[nodiscard]] static constexpr BytesCtorBits<std::endian::big, Extents> be_bytes_ctor_bits(
    const std::span<const uint8_t, Extents> bytes
){
    return BytesCtorBits<std::endian::big, Extents>(bytes);
}




}