#pragma once

#include "core/utils/Option.hpp"
#include "core/polymorphism/reflect.hpp"

namespace ymd::serde{

struct RawBytes;
struct ReadableAscii;


template<typename Protocol, typename T>
struct SerializeIter{};


enum class DeserializeError:uint8_t{
    BytesLengthShort
};


template<typename Protocol, typename T>
struct Deserializer{};


template<typename Protocol, typename T>
struct SerializeIterMaker{
    static constexpr auto make(const T & obj){
        return SerializeIter<Protocol, T>(obj);
    }
};


template<typename Protocol, typename T>
struct DeserializerMaker{
    static constexpr auto make(){
        return Deserializer<Protocol, T>{};
    }
};


template<typename Protocol, typename T>
struct serialize_iter_support_sso:std::false_type{};

template<typename Protocol, typename T>
static constexpr bool serialize_iter_sso_v = serialize_iter_support_sso<Protocol, T>::value;


template<typename Protocol, typename T>
static constexpr auto make_serialize_iter(const T & obj){
    return SerializeIterMaker<Protocol, T>::make(obj);
}



template<typename Protocol, typename T>
static constexpr auto make_deserializer() {
    return DeserializerMaker<Protocol, T>::make();
}


template<typename Protocol, typename T>
static constexpr auto make_deserialize(auto && pbuf) {
    return make_deserializer<Protocol, T>()
        .deserialize(std::forward<std::decay_t<decltype(pbuf)>>(pbuf));
}


template<size_t Q>
struct SerializeIter<RawBytes, iq_t<Q>>{
    constexpr explicit SerializeIter(const iq_t<Q> num):
        buf_(serialize(num)){;}
    constexpr bool has_next() const {
        return pos_ < N;
    }
    constexpr uint8_t next() {
        return buf_[pos_++];
    }

    static constexpr std::array<uint8_t, 4> serialize(const iq_t<Q> num){
        const auto inum = num.to_i32();
        return std::bit_cast<std::array<uint8_t, 4>>(inum);
    } 
private:
    static constexpr size_t N = sizeof(decltype(serialize(std::declval<iq_t<Q>>())));
    using Buf = std::array<uint8_t, N>;
    Buf buf_;
    size_t pos_ = 0;
};

template<typename Protocol, size_t Q>
struct serialize_iter_support_sso<Protocol, iq_t<Q>>:std::true_type{};


template<typename T>
requires (std::is_integral_v<T> || std::is_floating_point_v<T>)
struct SerializeIter<RawBytes, T>{
    static constexpr size_t N = sizeof(T);
    constexpr explicit SerializeIter(const T num):
        buf_(serialize(num)){;}

    constexpr bool has_next() const {
        return pos_ < N;
    }
    constexpr uint8_t next() {
        return buf_[pos_++];
    }

    static constexpr std::array<uint8_t, N> serialize(const T num){
        return std::bit_cast<std::array<uint8_t, N>>(num);
    } 
private:
    using Buf = std::array<uint8_t, N>;
    Buf buf_;
    size_t pos_ = 0;
};

template<typename Protocol, typename T>
requires (std::is_integral_v<T> || std::is_floating_point_v<T>)
struct serialize_iter_support_sso<Protocol, T>:std::true_type{;};


// 枚举类型特化
template<typename T>
requires std::is_enum_v<T>
struct SerializeIter<RawBytes, T> {
    using UnderlyingType = std::underlying_type_t<T>;
    static constexpr size_t N = sizeof(UnderlyingType);
    
    constexpr explicit SerializeIter(const T value)
        : buf_(serialize(value)) {}

    constexpr bool has_next() const { return pos_ < N; }
    
    constexpr uint8_t next() { return buf_[pos_++]; }

    static constexpr std::array<uint8_t, N> serialize(const T value) {
        return SerializeIter<RawBytes, UnderlyingType>::serialize(
            static_cast<UnderlyingType>(value));
    }

private:
    std::array<uint8_t, N> buf_;
    size_t pos_ = 0;
};

template<typename Protocol, typename T>
requires(std::is_enum_v<T>)

struct serialize_iter_support_sso<Protocol, T>:std::true_type{;};


template<typename Protocol, typename ... Ts>
struct SerializeIter<Protocol, std::tuple<Ts ... >> {
    static constexpr size_t N = sizeof...(Ts);
    
    constexpr explicit SerializeIter(const std::tuple<Ts...> & tup)
        : iters_(make_iter_tuple(tup, std::index_sequence_for<Ts...>{})) {}
        
    constexpr bool has_next() const {
        if (pos_ >= N) return false;
        
        return [this]<size_t ...Is>(std::index_sequence<Is...>) {
            bool has_next = false;
            // 使用折叠表达式检查当前迭代器是否有下一个元素
            ((Is == pos_ ? (has_next = std::get<Is>(iters_).has_next()) : false), ...);
            return has_next;
        }(std::make_index_sequence<N>{});
    }
    
    constexpr uint8_t next() {
        return [this]<size_t ...Is>(std::index_sequence<Is...>) -> uint8_t {
            uint8_t val = 0;
            // 使用折叠表达式调用当前迭代器的next()
            ((Is == pos_ ? (val = forward<Is>()) : 0), ...);
            return val;
        }(std::make_index_sequence<N>{});
    }

    template<size_t I>
    constexpr uint8_t forward() {
        auto& iter = std::get<I>(iters_);
        uint8_t val = iter.next();
        
        // 如果当前迭代器完成，移动到下一个迭代器
        if(!iter.has_next()) {
            pos_++;
        }
        
        return val;
    }

private:
    template<size_t... Is>
    static constexpr auto make_iter_tuple(const std::tuple<Ts...>& tup, std::index_sequence<Is...>) {
        return std::tuple{
            [&]<size_t I>{
                using RawType = std::tuple_element_t<I, std::tuple<Ts...>>;
                using ElemType = std::decay_t<RawType>;
                if constexpr (serialize_iter_sso_v<Protocol, ElemType>) {
                    return SerializeIter<Protocol, ElemType>{std::get<I>(tup)};
                } else {
                    return SerializeIter<Protocol, ElemType>{std::get<I>(tup)};
                }
            }.template operator()<Is>()...
        };
    }
    
    using Iters = decltype(make_iter_tuple(std::declval<std::tuple<Ts...>>(), std::index_sequence_for<Ts...>{}));
    Iters iters_;
    size_t pos_ = 0;
};


template<typename T>
requires std::is_aggregate_v<std::decay_t<T>>
OutputStream& print_struct(OutputStream& os, T&& value) {
    using AggregateType = std::decay_t<T>;
    
    os.scoped(reflect::type_name<AggregateType>())([&]<size_t... Is>(std::index_sequence<Is...>) -> OutputStream& {
        // Fold expression with conditional splitter insertion
        bool first = true;
        return(..., [&](const auto& field) -> OutputStream& {
            if (!first) os << os.splitter();
            first = false;
            return os << os.field(reflect::member_name<Is>(value))(os << reflect::get<Is>(value));
        }(reflect::get<Is>(value)));
    }(std::make_index_sequence<reflect::size<AggregateType>()>{}));
    
    return os;
}



template<size_t Q>
struct Deserializer<RawBytes, iq_t<Q>> {
    static constexpr size_t N = sizeof(iq_t<Q>);
    [[nodiscard]] static constexpr size_t size(std::span<const uint8_t>){
        return Q;
    }

    [[nodiscard]] __fast_inline static constexpr std::span<const uint8_t>
    take(std::span<const uint8_t> pbuf){
        return pbuf.subspan(size(pbuf));
    }

    [[nodiscard]] static constexpr Result<iq_t<Q>, DeserializeError> 
    deserialize(std::span<const uint8_t> pbuf) {
        if(pbuf.size() < Q) return Err(DeserializeError::BytesLengthShort);
        int32_t val = std::bit_cast<int32_t>(
            std::array<uint8_t, N>{pbuf[0], pbuf[1], pbuf[2], pbuf[3]});
        return Ok(iq_t<Q>::from_i32(val));
    }
};


template<typename T>
requires (std::is_integral_v<T> || std::is_floating_point_v<T>)
struct Deserializer<RawBytes, T> {
    static constexpr size_t N = sizeof(T);
    [[nodiscard]] static constexpr size_t size(std::span<const uint8_t>){
        return N;
    }

    [[nodiscard]] __fast_inline static constexpr std::span<const uint8_t>
    take(std::span<const uint8_t> pbuf){
        return pbuf.subspan(size(pbuf));
    }

    [[nodiscard]] static constexpr Result<T, DeserializeError> 
    deserialize(std::span<const uint8_t> pbuf) {
        if (pbuf.size() < N) {
            return Err(DeserializeError::BytesLengthShort);
        }

        std::array<uint8_t, N> bytes{};
        std::copy_n(pbuf.data(), N, bytes.begin());
        return Ok(std::bit_cast<T>(bytes));
    }
};


template<typename Protocol, typename... Ts>
struct Deserializer<Protocol, std::tuple<Ts...>> {
    static constexpr size_t N = (sizeof(Ts) + ...);

    [[nodiscard]] static constexpr size_t size() {
        return (Deserializer<Protocol, Ts>::size() + ...);
    }

    [[nodiscard]] __fast_inline static constexpr std::span<const uint8_t>
    take(std::span<const uint8_t> pbuf) {
        return pbuf.subspan(size());
    }

    [[nodiscard]] static constexpr Result<std::tuple<Ts...>, DeserializeError>
    deserialize(std::span<const uint8_t> data) {
        if (data.size() < size()) {
            return Err(DeserializeError::BytesLengthShort);
        }

        return deserialize_impl(data, std::index_sequence_for<Ts...>{});
    }

private:
    template<size_t... Is>
    [[nodiscard]] static constexpr Result<std::tuple<Ts...>, DeserializeError>
    deserialize_impl(std::span<const uint8_t> data, std::index_sequence<Is...>) {
        std::tuple<Ts...> result;
        std::span<const uint8_t> remaining = data;

        // Fold expression to deserialize each element sequentially
        auto deserialize_element = [&]<size_t I>(std::integral_constant<size_t, I>) {
            using ElementType = std::tuple_element_t<I, std::tuple<Ts...>>;
            auto elem_result = make_deserialize<Protocol, ElementType>(remaining);
            if (elem_result.is_err()) {
                return elem_result.unwrap_err();
            }
            std::get<I>(result) = elem_result.unwrap();
            remaining = Deserializer<Protocol, ElementType>::take(remaining);
            return DeserializeError::BytesLengthShort; // Dummy return, won't be used
        };

        // Execute deserialization for each element
        (..., deserialize_element(std::integral_constant<size_t, Is>{}));

        return Ok(result);
    }
};




#define DERIVE_SERIALIZE_AS_TUPLE(T)\
template<typename Protocol>\
struct serde::SerializeIterMaker<Protocol, T>{\
    static constexpr auto make(T obj){\
        return make_serialize_iter<Protocol>(reflect::to<std::tuple>(obj));\
    }\
};


#define DERIVE_DEBUG_AS_DISPLAY(T)\
OutputStream & operator<<(OutputStream & os, const T & self){ \
    return serde::print_struct(os, self);\
}


}