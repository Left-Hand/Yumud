#pragma once

#include "core/utils/Result.hpp"
#include "core/utils/reflecter.hpp"

#include "core/math/float/bf16.hpp"
#include "core/string/view/string_view.hpp"

namespace ymd::serde{
template<std::endian Endian>
struct [[nodiscard]] RawBytes{
    using item_type = uint8_t;
};

using RawLeBytes = RawBytes<std::endian::little>;

struct [[nodiscard]] ReadableAscii{
    using item_type = char;
};

template<typename Protocol, typename T>
struct [[nodiscard]] SerializeGenerator{};


enum class DeserializeError:uint8_t{
    Short,
    ShortParsingIq,
    ShortParsingInt,
    ShortParsingFloating,
    ShortParsingBf16,
    ShortParsingElement,
    Long,
    Mismatch
};

template<typename Protocol, typename T>
requires std::is_aggregate_v<std::decay_t<T>>
struct [[nodiscard]] StructDeserializer;

template<typename Protocol, typename T>
struct [[nodiscard]] Deserializer {
    static constexpr Result<T, DeserializeError>
    deserialize(std::span<const uint8_t> pbuf){
        return StructDeserializer<Protocol, T>().template deserialize(pbuf);
    }
};

template<typename T>
requires std::is_aggregate_v<std::decay_t<T>>
struct [[nodiscard]] StructDeserializer<RawLeBytes, T>{
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
            return Err(DeserializeError::Long);
        }

        return Ok(result);
    }

    //输入给定的成员的字节流 返回剩余待处理的字节流或错误
    template<size_t N>
    static constexpr Result<std::span<const uint8_t>, DeserializeError> 
    deserialize_member(T& obj, std::span<const uint8_t> pbuf) {
        using MemberType = typename reflecter::Reflecter<T>::template member_t<N>;

        if(pbuf.size() < sizeof(MemberType))
            return Err(DeserializeError::ShortParsingElement);
        const auto res = Deserializer<RawLeBytes, MemberType>
            ::deserialize(pbuf.subspan(0, sizeof(MemberType)));
        
        if (res.is_err())
            return Err(res.unwrap_err());

        constexpr auto member_ptr = reflecter::Reflecter<T>::template member_ptr_v<N>;
        obj.*member_ptr = res.unwrap();
        return Ok(pbuf.subspan(sizeof(MemberType)));
    }
};



template<typename Protocol, typename T>
struct [[nodiscard]] SerializeGeneratorFactory{
    static constexpr auto from(const T & obj){
        return SerializeGenerator<Protocol, T>(obj);
    }
};


template<typename Protocol, typename T>
struct [[nodiscard]] DeserializerFactory{
    static constexpr auto from(){
        return Deserializer<Protocol, T>{};
    }
};


template<typename Protocol, typename T>
struct [[nodiscard]] serialize_generator_support_sbo:std::false_type{};

template<typename Protocol, typename T>
static constexpr bool serialize_generator_support_sbo_v = 
    serialize_generator_support_sbo<Protocol, T>::value;


template<typename Protocol, typename T>
static constexpr auto make_serialize_generator(const T & obj){
    return SerializeGeneratorFactory<Protocol, T>::from(obj);
}



template<typename Protocol, typename T>
static constexpr auto make_deserializer() {
    return DeserializerFactory<Protocol, T>::from();
}


template<typename Protocol, typename T>
static constexpr auto deserialize(const auto & pbuf) {
    return make_deserializer<Protocol, T>()
        .deserialize(pbuf);
}


template<size_t Q, typename D>
struct [[nodiscard]] SerializeGenerator<RawLeBytes, math::fixed_t<Q, D>>{
    using Item = typename RawLeBytes::item_type;
    static constexpr size_t N = sizeof(D);
    constexpr explicit SerializeGenerator(const math::fixed_t<Q, D> num):
        buf_(serialize(num)){;}
    [[nodiscard]] constexpr bool has_next() const {
        return pos_ < N;
    }
    [[nodiscard]] constexpr Item next() {
        return buf_[pos_++];
    }

    [[nodiscard]] static constexpr std::array<Item, sizeof(D)> serialize(const math::fixed_t<Q, D> num){
        const auto inum = num.to_bits();
        return std::bit_cast<std::array<Item, sizeof(D)>>(inum);
    } 
private:

    using Buf = std::array<Item, N>;
    Buf buf_;
    size_t pos_ = 0;
};

template<typename Protocol, size_t Q, typename D>
struct [[nodiscard]] serialize_generator_support_sbo<Protocol, math::fixed_t<Q, D>>:std::true_type{};


template<typename T>
requires (std::is_integral_v<T> || std::is_floating_point_v<T>)
struct [[nodiscard]] SerializeGenerator<RawLeBytes, T>{
    static constexpr size_t N = sizeof(T);
    constexpr explicit SerializeGenerator(const T num):
        buf_(serialize(num)){;}

    [[nodiscard]] constexpr bool has_next() const {
        return pos_ < N;
    }
    [[nodiscard]] constexpr uint8_t next() {
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
struct [[nodiscard]] SerializeGenerator<RawLeBytes, math::bf16>{
    static constexpr size_t N = sizeof(math::bf16);
    constexpr explicit SerializeGenerator(const math::bf16 num):
        buf_(serialize(num)){;}

    [[nodiscard]] constexpr bool has_next() const {
        return pos_ < N;
    }
    [[nodiscard]] constexpr uint8_t next() {
        return buf_[pos_++];
    }

    static constexpr std::array<uint8_t, N> serialize(const math::bf16 num){
        return std::bit_cast<std::array<uint8_t, N>>(num.to_bits());
    } 
private:
    using Buf = std::array<uint8_t, N>;
    Buf buf_;
    size_t pos_ = 0;
};


template<typename Protocol, typename T>
requires (std::is_integral_v<T> || std::is_floating_point_v<T>)
struct [[nodiscard]] serialize_generator_support_sbo<Protocol, T>:std::true_type{;};


// 枚举类型特化
template<typename T>
requires std::is_enum_v<T>
struct [[nodiscard]] SerializeGenerator<RawLeBytes, T> {
    using UnderlyingType = std::underlying_type_t<T>;
    static constexpr size_t N = sizeof(UnderlyingType);
    
    constexpr explicit SerializeGenerator(const T value)
        : buf_(serialize(value)) {}

    [[nodiscard]] constexpr bool has_next() const { return pos_ < N; }
    
    [[nodiscard]] constexpr uint8_t next() { return buf_[pos_++]; }

    static constexpr std::array<uint8_t, N> serialize(const T value) {
        return SerializeGenerator<RawLeBytes, UnderlyingType>::serialize(
            static_cast<UnderlyingType>(value));
    }

private:
    std::array<uint8_t, N> buf_;
    size_t pos_ = 0;
};


template<typename Protocol, typename T>
requires(std::is_enum_v<T>)
struct [[nodiscard]] serialize_generator_support_sbo<Protocol, T>:std::true_type{;};

template <typename T>
struct [[nodiscard]] SerializeGenerator<RawLeBytes, std::span<const T>> {
    using ElementIter = SerializeGenerator<RawLeBytes, T>;
    
    constexpr explicit SerializeGenerator(const std::span<const T> pbuf)
        : pbuf_(pbuf), index_(0) 
    {
        if (pbuf_.size()) {
            element_iter_ = ElementIter(pbuf_[0]); // 使用 placement new 初始化
        }
    }

    // 必须手动管理 union 成员的析构
    constexpr ~SerializeGenerator() {
        if (pbuf_.size()) {
            element_iter_.~ElementIter(); // 手动调用析构
        }
    }

    [[nodiscard]] constexpr bool has_next() const {
        if(index_ >= pbuf_.size()) return false;
        else if(index_ + 1 == pbuf_.size()) return element_iter_.has_next();    
        return true;
    }
    
    [[nodiscard]] constexpr uint8_t next() {
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
struct [[nodiscard]] SerializeGeneratorFactory<Protocol, std::span<const T, N>>{
    static constexpr auto from(const std::span<const T, N> obj){
        return SerializeGenerator<Protocol, std::span<const T>>(std::span<const T>(obj));
    }
};

template<typename Protocol, typename T, size_t N>
struct [[nodiscard]] SerializeGeneratorFactory<Protocol, std::array<T, N>>{
    static constexpr auto from(const std::array<T, N> & obj){
        return SerializeGenerator<Protocol, std::span<const T>>(std::span<const T>(obj));
    }
};

template<typename Protocol, typename T, size_t N>
struct [[nodiscard]] SerializeGeneratorFactory<Protocol, T[N]>{
    static constexpr auto from(const T (&obj)[N]) {
        return SerializeGenerator<Protocol, std::span<const T>>(std::span<const T>(obj));
    }
};

template<typename Protocol>
struct [[nodiscard]] SerializeGeneratorFactory<Protocol, StringView>{
    static constexpr auto from(const StringView str) {
        return SerializeGenerator<Protocol, std::span<const char>>(str.chars());
    }
};

template<typename Protocol, typename T>
struct [[nodiscard]] SerializeGeneratorFactory<Protocol, std::initializer_list<T>> {
    static constexpr auto from(std::initializer_list<T> list) {
        return SerializeGenerator<Protocol, std::span<const T>>(
            std::span<const T>(list.begin(), list.size())
        );
    }
};

template<typename Protocol, typename ... Ts>
struct [[nodiscard]] SerializeGenerator<Protocol, std::tuple<Ts ... >> {
    static constexpr size_t N = sizeof...(Ts);
    
    constexpr explicit SerializeGenerator(const std::tuple<Ts...> & tup)
        : sub_generators_(make_generator_tuple(tup, std::index_sequence_for<Ts...>{})) {}
        
    [[nodiscard]] constexpr bool has_next() const {
        bool searched = false;
        bool result = false;
        return [&]<size_t ...Is>(std::index_sequence<Is...>) {
            // 使用 if constexpr 替代三元表达式
            ([&] {
                if ((Is == pos_) and (searched == false)) {
                    searched = true;
                    result = std::get<Is>(sub_generators_).has_next();
                }
            }(), ...);
            return result;
        }(std::make_index_sequence<N>{});
    }
    
    [[nodiscard]] constexpr uint8_t next() {
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
        auto& iter = std::get<I>(sub_generators_);
        uint8_t val = iter.next();
        
        // 如果当前迭代器完成，移动到下一个迭代器
        if (!iter.has_next()) {
            pos_++;
        }
        
        return val;
    }
private:
    template<size_t... Is>
    static constexpr auto make_generator_tuple(const std::tuple<Ts...>& tup, std::index_sequence<Is...>) {
        return std::tuple{
            [&]<size_t I>{
                using RawType = std::tuple_element_t<I, std::tuple<Ts...>>;
                using ElemType = std::decay_t<RawType>;
                if constexpr (serialize_generator_support_sbo_v<Protocol, ElemType>) {
                    return SerializeGeneratorFactory<Protocol, ElemType>::from(std::get<I>(tup));
                } else {
                    return SerializeGeneratorFactory<Protocol, ElemType>::from(std::get<I>(tup));
                }
            }.template operator()<Is>()...
        };
    }
    
    using Iters = decltype(make_generator_tuple(std::declval<std::tuple<Ts...>>(), std::index_sequence_for<Ts...>{}));
    Iters sub_generators_;
    size_t pos_ = 0;
};


template<typename Protocol, typename T>
struct [[nodiscard]] SerializeGenerator<Protocol, Option<T>> {
    using InnerSerializeGenerator = SerializeGenerator<Protocol, T>;
    constexpr explicit SerializeGenerator(const Option<T> & may_value)
        : 
        inner_serialize_iter_(InnerSerializeGenerator(may_value)),
        is_some_(may_value.is_some()){;}

    constexpr bool has_next() const { return is_some_ && inner_serialize_iter_.has_next();}
    
    constexpr uint8_t next() { return is_some_ ? inner_serialize_iter_.next() : 0;}

private:
    InnerSerializeGenerator inner_serialize_iter_;
    bool is_some_;
};

template<size_t Q, typename D>
struct [[nodiscard]] Deserializer<RawLeBytes, math::fixed_t<Q, D>> {
    static constexpr size_t N = sizeof(D);
    [[nodiscard]] static constexpr size_t size(){
        return N;
    }

    [[nodiscard]] __fast_inline static constexpr std::span<const uint8_t>
    take(std::span<const uint8_t> pbuf){
        return pbuf.subspan(size());
    }

    [[nodiscard]] static constexpr Result<math::fixed_t<Q, D>, DeserializeError> 
    deserialize(std::span<const uint8_t> pbuf) {
        if(pbuf.size() < N) return Err(DeserializeError::ShortParsingIq);
        static_assert(N == 4);
        D val = std::bit_cast<D>(
            std::array<uint8_t, N>{pbuf[0], pbuf[1], pbuf[2], pbuf[3]});
        return Ok(math::fixed_t<Q, D>::from_bits(val));
    }
};


template<typename T>
requires (std::is_integral_v<T> || std::is_floating_point_v<T>)
struct [[nodiscard]] Deserializer<RawLeBytes, T> {
    static constexpr size_t N = sizeof(T);
    [[nodiscard]] static constexpr size_t size(){
        return N;
    }

    [[nodiscard]] __fast_inline static constexpr std::span<const uint8_t>
    take(std::span<const uint8_t> pbuf){
        return pbuf.subspan(size());
    }

    [[nodiscard]] static constexpr Result<T, DeserializeError> 
    deserialize(std::span<const uint8_t> pbuf) {
        if (pbuf.size() < N) {
            if constexpr(std::is_integral_v<T>)
                return Err(DeserializeError::ShortParsingInt);
            else 
                return Err(DeserializeError::ShortParsingFloating);
        }

        std::array<uint8_t, N> bytes{};
        std::copy_n(pbuf.data(), N, bytes.begin());
        return Ok(std::bit_cast<T>(bytes));
    }
};

template<>
struct [[nodiscard]] Deserializer<RawLeBytes, math::bf16> {
    static constexpr size_t N = sizeof(math::bf16);
    [[nodiscard]] static constexpr size_t size(){
        return N;
    }

    [[nodiscard]] __fast_inline static constexpr std::span<const uint8_t>
    take(std::span<const uint8_t> pbuf){
        return pbuf.subspan(size());
    }

    [[nodiscard]] static constexpr Result<math::bf16, DeserializeError> 
    deserialize(std::span<const uint8_t> pbuf) {
        if (pbuf.size() < N) {
            return Err(DeserializeError::ShortParsingBf16);
        }

        std::array<uint8_t, N> bytes{};
        std::copy_n(pbuf.data(), N, bytes.begin());
        return Ok(math::bf16::from_bits(std::bit_cast<uint16_t>(bytes)));
    }
};

template<typename Protocol, typename... Ts>
struct [[nodiscard]] Deserializer<Protocol, std::tuple<Ts...>> {
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
            return Err(DeserializeError::Short);
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
            return DeserializeError::Short; // Dummy return, won't be used
        };

        // Execute deserialization for each element
        (..., deserialize_element(std::integral_constant<size_t, Is>{}));

        return Ok(result);
    }
};




#define DEF_DERIVE_SERIALIZE_AS_TUPLE(T)\
template<typename Protocol>\
struct [[nodiscard]] serde::SerializeGeneratorFactory<Protocol, T>{\
    static constexpr auto from(T obj){\
        return make_serialize_generator<Protocol>(reflect::to<std::tuple>(obj));\
    }\
};

#define DEF_DERIVE_RAW_BYTES_DESERIALIZER(T) \
template<> \
struct [[nodiscard]] serde::DeserializerFactory<serde::RawLeBytes, T> { \
    static constexpr serde::Deserializer<RawLeBytes, T> \
    from() { \
        return serde::Deserializer<RawLeBytes, T>(); \
    } \
};


#define DEF_DERIVE_DEBUG_AS_DISPLAY(T)\
inline OutputStream & operator<<(OutputStream & os, const T & self){ \
    return reflecter::Displayer<T>::display(os, self);\
}





}