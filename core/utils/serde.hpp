#pragma once

#include "core/utils/Result.hpp"
#include "core/utils/reflecter.hpp"

#include "core/math/float/bf16.hpp"

namespace ymd::serde{

struct RawBytes;
struct ReadableAscii;

template<typename Protocol, typename T>
struct SerializeIter{};


enum class DeserializeError:uint8_t{
    BytesLengthShort,
    BytesLengthShortParsingIq,
    BytesLengthShortParsingInt,
    BytesLengthShortParsingFloating,
    BytesLengthShortParsingBf16,
    BytesLengthShortParsingElement,
    BytesLengthLong,
    BytesLengthMismatch
};

template<typename Protocol, typename T>
requires std::is_aggregate_v<std::decay_t<T>>
struct StructDeserializer;

template<typename Protocol, typename T>
struct Deserializer {
    static constexpr Result<T, DeserializeError>
    deserialize(std::span<const uint8_t> pbuf){
        return StructDeserializer<Protocol, T>().template deserialize(pbuf);
    }
};

template<typename T>
requires std::is_aggregate_v<std::decay_t<T>>
struct StructDeserializer<RawBytes, T>{
    static constexpr Result<T, DeserializeError> 
    deserialize(std::span<const uint8_t> pbuf) {
        return deserialize_struct(pbuf);
    }

// private:
    static constexpr Result<T, DeserializeError> 
    deserialize_struct(const std::span<const uint8_t> pbuf) {
        T result{};  // Initialize empty struct

        // Process members with error propagation
        auto process = [&]<size_t... Is>(std::index_sequence<Is...>) -> 
        Result<std::span<const uint8_t>, DeserializeError> {
            // Fold over all members with error handling
            Result<std::span<const uint8_t>, DeserializeError> final_remaining = Ok(pbuf);
            ((final_remaining = final_remaining.and_then([&](const std::span<const uint8_t> rem) { 
                return deserialize_member<Is>(result, rem);
            })), ...);

            return final_remaining;
        };
        
        const auto remaining_result = process(std::make_index_sequence<reflecter::Reflecter<T>::member_count_v>{});
        if (remaining_result.is_err()) {
            return Err(remaining_result.unwrap_err());
        }

        // Verify all bytes were consumed
        const auto final_remaining = remaining_result.unwrap();
        if (!final_remaining.empty()) {
            return Err(DeserializeError::BytesLengthLong);
        }

        return Ok(result);
    }

    //输入给定的成员的字节流 返回剩余待处理的字节流或错误
    template<size_t N>
    static constexpr Result<std::span<const uint8_t>, DeserializeError> 
    deserialize_member(T& obj, std::span<const uint8_t> pbuf) {
        using MemberType = typename reflecter::Reflecter<T>::template member_t<N>;

        if(pbuf.size() < sizeof(MemberType))
            return Err(DeserializeError::BytesLengthShortParsingElement);
        const auto res = Deserializer<RawBytes, MemberType>
            ::deserialize(pbuf.subspan(0, sizeof(MemberType)));
        
        if (res.is_err())
            return Err(res.unwrap_err());

        constexpr auto member_ptr = reflecter::Reflecter<T>::template member_ptr_v<N>;
        obj.*member_ptr = res.unwrap();
        return Ok(pbuf.subspan(sizeof(MemberType)));
    }
};



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
struct serialize_iter_support_sbo:std::false_type{};

template<typename Protocol, typename T>
static constexpr bool serialize_iter_sbo_v = serialize_iter_support_sbo<Protocol, T>::value;


template<typename Protocol, typename T>
static constexpr auto make_serialize_iter(const T & obj){
    return SerializeIterMaker<Protocol, T>::make(obj);
}



template<typename Protocol, typename T>
static constexpr auto make_deserializer() {
    return DeserializerMaker<Protocol, T>::make();
}


template<typename Protocol, typename T>
static constexpr auto make_deserialize(const auto & pbuf) {
    return make_deserializer<Protocol, T>()
        .deserialize(pbuf);
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
struct serialize_iter_support_sbo<Protocol, iq_t<Q>>:std::true_type{};


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

template<>
struct SerializeIter<RawBytes, bf16>{
    static constexpr size_t N = sizeof(bf16);
    constexpr explicit SerializeIter(const bf16 num):
        buf_(serialize(num)){;}

    constexpr bool has_next() const {
        return pos_ < N;
    }
    constexpr uint8_t next() {
        return buf_[pos_++];
    }

    static constexpr std::array<uint8_t, N> serialize(const bf16 num){
        return std::bit_cast<std::array<uint8_t, N>>(num.as_u16());
    } 
private:
    using Buf = std::array<uint8_t, N>;
    Buf buf_;
    size_t pos_ = 0;
};


template<typename Protocol, typename T>
requires (std::is_integral_v<T> || std::is_floating_point_v<T>)
struct serialize_iter_support_sbo<Protocol, T>:std::true_type{;};


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
struct serialize_iter_support_sbo<Protocol, T>:std::true_type{;};

template <typename T>
struct SerializeIter<RawBytes, std::span<const T>> {
    using ElementIter = SerializeIter<RawBytes, T>;
    
    constexpr explicit SerializeIter(const std::span<const T> pbuf)
        : pbuf_(pbuf), index_(0) 
    {
        if (pbuf_.size()) {
            element_iter_ = ElementIter(pbuf_[0]); // 使用 placement new 初始化
        }
    }

    // 必须手动管理 union 成员的析构
    constexpr ~SerializeIter() {
        if (pbuf_.size()) {
            element_iter_.~ElementIter(); // 手动调用析构
        }
    }

    constexpr bool has_next() const {
        if(index_ >= pbuf_.size()) return false;
        else if(index_ + 1 == pbuf_.size()) return element_iter_.has_next();    
        return true;
    }
    
    constexpr uint8_t next() {
        if (!element_iter_.has_next()) {
            element_iter_.~ElementIter(); // 先销毁当前对象
            
            if (++index_ >= pbuf_.size()) {
                // 错误处理：可以 throw 或返回特殊值
                return 0; 
            }
            
            element_iter_ = ElementIter(pbuf_[index_]); // 重建新对象
        }
        return element_iter_.next();
    }

private:
    std::span<const T> pbuf_;
    size_t index_;
    union {
        ElementIter element_iter_;
    };
};
template<typename Protocol, typename T, size_t N>
struct SerializeIterMaker<Protocol, std::span<const T, N>>{
    static constexpr auto make(const std::span<const T, N> obj){
        return SerializeIter<Protocol, std::span<const T>>(std::span<const T>(obj));
    }
};

template<typename Protocol, typename ... Ts>
struct SerializeIter<Protocol, std::tuple<Ts ... >> {
    static constexpr size_t N = sizeof...(Ts);
    
    constexpr explicit SerializeIter(const std::tuple<Ts...> & tup)
        : iters_(make_iter_tuple(tup, std::index_sequence_for<Ts...>{})) {}
        
    constexpr bool has_next() const {
        bool searched = false;
        bool result = false;
        return [&]<size_t ...Is>(std::index_sequence<Is...>) {
            // 使用 if constexpr 替代三元表达式
            ([&] {
                if ((Is == pos_) and (searched == false)) {
                    searched = true;
                    result = std::get<Is>(iters_).has_next();
                }
            }(), ...);
            return result;
        }(std::make_index_sequence<N>{});
    }
    
    constexpr uint8_t next() {
        bool searched = false;
        uint8_t val = 0;
        return [&]<size_t ...Is>(std::index_sequence<Is...>) -> uint8_t {
            // 使用 if constexpr 替代三元表达式
            ([&] {
                if((Is == pos_) and (searched == false)) {
                    searched = true;
                    val = forward<Is>();
                }
            }(), ...);
            return val;
        }(std::make_index_sequence<N>{});
    }

    template<size_t I>
    constexpr uint8_t forward() {
        auto& iter = std::get<I>(iters_);
        uint8_t val = iter.next();
        
        // 如果当前迭代器完成，移动到下一个迭代器
        if (!iter.has_next()) {
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
                if constexpr (serialize_iter_sbo_v<Protocol, ElemType>) {
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
        if(pbuf.size() < N) return Err(DeserializeError::BytesLengthShortParsingIq);
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
            if constexpr(std::is_integral_v<T>)
                return Err(DeserializeError::BytesLengthShortParsingInt);
            else 
                return Err(DeserializeError::BytesLengthShortParsingFloating);
        }

        std::array<uint8_t, N> bytes{};
        std::copy_n(pbuf.data(), N, bytes.begin());
        return Ok(std::bit_cast<T>(bytes));
    }
};

template<>
struct Deserializer<RawBytes, bf16> {
    static constexpr size_t N = sizeof(bf16);
    [[nodiscard]] static constexpr size_t size(std::span<const uint8_t>){
        return N;
    }

    [[nodiscard]] __fast_inline static constexpr std::span<const uint8_t>
    take(std::span<const uint8_t> pbuf){
        return pbuf.subspan(size(pbuf));
    }

    [[nodiscard]] static constexpr Result<bf16, DeserializeError> 
    deserialize(std::span<const uint8_t> pbuf) {
        if (pbuf.size() < N) {
            return Err(DeserializeError::BytesLengthShortParsingBf16);
        }

        std::array<uint8_t, N> bytes{};
        std::copy_n(pbuf.data(), N, bytes.begin());
        return Ok(bf16::from_u16(std::bit_cast<uint16_t>(bytes)));
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




#define DEF_DERIVE_SERIALIZE_AS_TUPLE(T)\
template<typename Protocol>\
struct serde::SerializeIterMaker<Protocol, T>{\
    static constexpr auto make(T obj){\
        return make_serialize_iter<Protocol>(reflect::to<std::tuple>(obj));\
    }\
};

#define DEF_DERIVE_RAW_BYTES_DESERIALIZER(T) \
template<> \
struct serde::DeserializerMaker<serde::RawBytes, T> { \
    static constexpr serde::Deserializer<RawBytes, T> \
    make() { \
        return serde::Deserializer<RawBytes, T>(); \
    } \
};


#define DEF_DERIVE_DEBUG_AS_DISPLAY(T)\
OutputStream & operator<<(OutputStream & os, const T & self){ \
    return reflecter::Displayer<T>::display(os, self);\
}





}