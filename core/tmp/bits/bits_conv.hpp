#pragma once


#include <tuple>
#include <cstdint>
#include <bit>
#include <concepts>

namespace ymd::tmp{

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
struct _to_bits{
    using type = void;
};

template<typename T>
requires requires {
    T::bits;
}
struct _to_bits<T>{
    using bits_type = decltype(T::bits);
    [[nodiscard]] static constexpr bits_type into_bits(const T & obj){
        return obj.bits;
    }
};

template<typename T>
requires (std::is_integral_v<T>)
struct _to_bits<T>{
    using bits_type = T;

    [[nodiscard]] static constexpr bits_type into_bits(const T & obj){
        return std::bit_cast<bits_type>(obj);
    }
};

template<typename T>
requires (std::is_floating_point_v<T>)
struct _to_bits<T>{
    using bits_type = std::conditional_t<std::is_same_v<T, float>, uint32_t, uint64_t>;

    [[nodiscard]] static constexpr bits_type into_bits(const T & obj){
        return std::bit_cast<bits_type>(obj);
    } 
};

template<typename T>
requires requires{
    T::to_bits();
}
struct _to_bits<T>{
    using bits_type = std::invoke_result_t<decltype(T::to_bits)>;
    [[nodiscard]] static constexpr bits_type into_bits(const T & obj){
        return obj.to_bits();
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
using to_bits_t = typename details::_to_bits<T>::bits_type;


template<typename T, typename D = tmp::from_bits_t<T>>
__attribute__((always_inline))
[[nodiscard]] static constexpr T obj_from_bits(const D bits){
    return details::_from_bits<T>::into_obj(bits);
}

template<typename T, typename D = tmp::to_bits_t<T>>
__attribute__((always_inline))
[[nodiscard]] static constexpr D obj_to_bits(const T & obj){
    return details::_to_bits<T>::into_bits(obj);
}


}