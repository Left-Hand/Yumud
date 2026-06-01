#pragma once

#include "mavlink_primitive.hpp"
#include "core/utils/marco_utils.hpp"

namespace ymd::mavlink{

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
using bits_type_t = typename _bits_type<T>::type;

template<typename T>
static constexpr size_t storage_capacity_v = 0;

template<typename T, size_t N>
static constexpr size_t storage_capacity_v<std::array<T, N>> = N;

template<typename T, size_t N>
static constexpr size_t storage_capacity_v<std::span<T, N>> = N;

template<typename T>
static constexpr bool is_cstyle_array_v = false;

template<typename T, std::size_t N>
static constexpr bool is_cstyle_array_v<T[N]> = true;

template<typename T, std::size_t N>
static constexpr bool is_cstyle_array_v<const T[N]> = true;



template<typename T>
static constexpr bool is_cstyle_char_array_v = false;

template<std::size_t N>
static constexpr bool is_cstyle_char_array_v<char[N]> = true;

template<std::size_t N>
static constexpr bool is_cstyle_char_array_v<const char[N]> = true;

template<typename T>
struct _cstyle_array_size;

template<typename T, std::size_t N>
struct _cstyle_array_size<T[N]> : std::integral_constant<std::size_t, N> {};

template<typename T, std::size_t N>
struct _cstyle_array_size<const T[N]> : std::integral_constant<std::size_t, N> {};

template<typename T>
static constexpr std::size_t cstyle_array_size_v = 
    _cstyle_array_size<T>::value;



static_assert(is_cstyle_char_array_v<char[8]> == true);
static_assert(is_cstyle_char_array_v<const char[8]> == true);
static_assert(is_cstyle_char_array_v<uint8_t[8]> == false);




template<typename T>
struct MavBytesOp;


template<>
struct MavBytesOp<void>{
    static constexpr size_t ELEMENT_SIZE = 0;
};

template<>
struct MavBytesOp<uint8_t>{
    static constexpr size_t ELEMENT_SIZE = 1;
    static constexpr uint8_t load(const uint8_t * ptr){
        return static_cast<uint8_t>(*ptr);
    }

    static constexpr void store(uint8_t * ptr, const uint8_t obj){
        *ptr = static_cast<uint8_t>(obj);
    }
};

template<>
struct MavBytesOp<uint16_t>{
    static constexpr size_t ELEMENT_SIZE = 2;
    static constexpr uint16_t load(const uint8_t * ptr){
        uint16_t sum = 0;
        sum |= ptr[0];
        sum |= ptr[1] << 8;
        return sum;
    }

    static constexpr void store(uint8_t * ptr, const uint16_t obj){
        ptr[0] = static_cast<uint8_t>(obj);
        ptr[1] = static_cast<uint8_t>(obj >> 8);
    }
};

template<>
struct MavBytesOp<uint32_t>{
    static constexpr size_t ELEMENT_SIZE = 4;
    static constexpr uint32_t load(const uint8_t * ptr){
        uint32_t sum = 0;
        sum |= ptr[0];
        sum |= ptr[1] << 8;
        sum |= ptr[2] << 16;
        sum |= ptr[3] << 24;
        return sum;
    }

    static constexpr void store(uint8_t * ptr, const uint32_t obj){
        ptr[0] = static_cast<uint8_t>(obj);
        ptr[1] = static_cast<uint8_t>(obj >> 8);
        ptr[2] = static_cast<uint8_t>(obj >> 16);
        ptr[3] = static_cast<uint8_t>(obj >> 24);
    }
};

template<typename T>
requires(std::is_integral_v<T> and std::is_signed_v<T>)
struct MavBytesOp<T>{
    static constexpr size_t ELEMENT_SIZE = sizeof(T);
    using U = std::make_unsigned_t<T>;
    static constexpr T load(const uint8_t * ptr){
        return std::bit_cast<T>(MavBytesOp<U>::load(ptr));
    }

    static constexpr void store(uint8_t * ptr, const T obj){
        MavBytesOp<U>::store(ptr, std::bit_cast<U>(obj));
    }
};

template<typename T>
requires(not std::is_same_v<bits_type_t<T>, void>)
struct MavBytesOp<T>{
    using U = bits_type_t<T>;
    static constexpr size_t ELEMENT_SIZE = sizeof(U);
    static constexpr T load(const uint8_t * ptr){
        return std::bit_cast<T>(MavBytesOp<U>::load(ptr));
    }

    static constexpr void store(uint8_t * ptr, const T obj){
        MavBytesOp<U>::store(ptr, std::bit_cast<U>(obj));
    }
};

template<typename T>
requires(std::is_enum_v<T>)
struct MavBytesOp<T>{
    using D = std::underlying_type_t<T>;
    static constexpr size_t ELEMENT_SIZE = sizeof(D);

    static constexpr T load(const uint8_t * ptr){
        return std::bit_cast<T>(MavBytesOp<D>::load(ptr));
    }

    static constexpr void store(uint8_t * ptr, const T obj){
        MavBytesOp<D>::store(ptr, std::bit_cast<D>(obj));
    }
};


template<>
struct MavBytesOp<MavModeFlag>{
    static constexpr size_t ELEMENT_SIZE = 1;
    static constexpr MavModeFlag load(const uint8_t * ptr){
        return static_cast<MavModeFlag>(*ptr);
    }

    static constexpr void store(uint8_t * ptr, const MavModeFlag obj){
        *ptr = static_cast<uint8_t>(obj.bits);
    }
};



template<>
struct MavBytesOp<math::fp32>{
    static constexpr size_t ELEMENT_SIZE = 4;
    static constexpr fp32 load(const uint8_t * ptr){
        return fp32::from_bits(MavBytesOp<uint32_t>::load(ptr));
    }

    static constexpr void store(uint8_t * ptr, const math::fp32 obj){
        MavBytesOp<uint32_t>::store(ptr, obj.to_bits());
    }
};

template<typename T, size_t N>
requires (not std::is_same_v<T, char>)
struct MavBytesOp<T[N]>{
    static constexpr size_t ELEMENT_SIZE = N;
};


template<size_t N>
struct MavBytesOp<char[N]>{
    static constexpr size_t ELEMENT_SIZE = N;
};


template <size_t N, typename Tup>
struct _nth_offset{
    static constexpr size_t value = 
        _nth_offset<N - 1, Tup>::value + MavBytesOp<std::tuple_element_t<N - 1, Tup>>::ELEMENT_SIZE;
};


template<typename Tup>
struct _nth_offset<0, Tup>{
    static constexpr size_t value = 0;
};


template<typename T>
struct MemberBytesProxy{
    using ptr_type = std::conditional_t<std::is_const_v<T>, const uint8_t *, uint8_t *>;
    using U = std::remove_const_t<T>;



    ptr_type ptr;

    constexpr U get() const {
        return MavBytesOp<U>::load(ptr);
    }

    constexpr void set(U obj) requires(not std::is_const_v<T>) {
        return MavBytesOp<U>::store(ptr, obj);
    }
};

template<typename T>
requires(is_cstyle_char_array_v<T>)
struct MemberBytesProxy<T>{
    static_assert(not std::is_pointer_v<T>);
    using ptr_type = std::conditional_t<std::is_const_v<T>, const uint8_t *, uint8_t *>;
    using U = std::remove_const_t<T>;
    static_assert(not std::is_pointer_v<U>);
    static constexpr size_t CAPACITY = cstyle_array_size_v<U>;

    ptr_type ptr;

    StringView get() const {
        return StringView::from_zero_terminated(
            reinterpret_cast<const char *>(ptr),
            CAPACITY
        );
    }

    constexpr size_t length() const {
        size_t i = 0;
        for(; i < CAPACITY; i++){
            if(ptr[i] == 0) break;
        }
        return i;
    }

    consteval size_t capacity() const {
        return CAPACITY;
    }

    constexpr uint8_t operator[](const size_t idx) const {
        if(idx >= CAPACITY) __builtin_trap();
        return static_cast<uint8_t>(ptr[idx]);
    }

    constexpr uint8_t & operator[](const size_t idx) requires(not std::is_const_v<T>) {
        if(idx >= CAPACITY) __builtin_trap();
        return (ptr[idx]);
    }

    constexpr Result<void, void> set(const StringView str) requires(not std::is_const_v<T>) {
        if(str.length() > CAPACITY) return Err();
        for(size_t i = 0; i < str.length(); i++){
            ptr[i] = static_cast<uint8_t>(str[i]);
        }
    }
};


template<typename R, typename B>
static constexpr auto make_member_bytes_proxy(B ptr){
    static_assert(not std::is_pointer_v<R>);
    using T = std::conditional_t<std::is_const_v<std::remove_pointer_t<B>>, 
        const R, R>;
    return MemberBytesProxy<T>(ptr);
}



template<typename T>
requires(is_cstyle_array_v<T> and (not is_cstyle_char_array_v<T>))
struct MemberBytesProxy<T>{
    using ptr_type = std::conditional_t<std::is_const_v<T>, const uint8_t *, uint8_t *>;
    using U = std::remove_const_t<T>;

    static constexpr size_t CAPACITY = cstyle_array_size_v<U>;

    using element_type = std::remove_all_extents_t<U>;
    using maymut_element_type = std::conditional_t<std::is_const_v<T>, const element_type, element_type>;

    static constexpr size_t ELEMENT_SIZE = MavBytesOp<element_type>::ELEMENT_SIZE;

    ptr_type ptr;

    consteval size_t capacity() const {return CAPACITY;}

    template<typename Self>
    constexpr auto operator[](this Self && self, const size_t idx) {
        if(idx >= CAPACITY) __builtin_trap();
        
        return make_member_bytes_proxy<maymut_element_type>(&self.ptr[idx * ELEMENT_SIZE]);
    }
};


#define MAVLINK_REFLECT_PROXY(_NAME, _RET, ...) (_RET, _NAME, __VA_ARGS__)

#define _MAVLINK_REFLECT_ELEMENT_RANK(_method_tuple_) \
_TRAIT_METHOD_PICK_NAME(_method_tuple_),


#define _MAVLINK_REFLECT_TYPETUPLE(_method_tuple_) \
_TRAIT_METHOD_PICK_RET(_method_tuple_),

#define _MAVLINK_REFLECT_MEMBER_BYTES_PROXY(_method_tuple_) \
template<typename Self>\
constexpr auto _TRAIT_METHOD_PICK_NAME(_method_tuple_) (this Self && self){\
    constexpr size_t OFFSET = nth_offset_v<static_cast<size_t>(ElementRank::_TRAIT_METHOD_PICK_NAME(_method_tuple_))>;\
    return make_member_bytes_proxy<_TRAIT_METHOD_PICK_RET(_method_tuple_)>(&self.data[OFFSET]);}\

#define _MAVLINK_REFLECT_STRUCT_BASE(NAME, NUM_MSG_ID, ...) \
template<typename Storage>\
struct [[nodiscard]] NAME##_Facade final{\
    static constexpr MavMessageId MSG_ID = MavMessageId{NUM_MSG_ID};\
    using members_tuple = std::tuple<MACRO_MAP(_MAVLINK_REFLECT_TYPETUPLE, __VA_ARGS__) void>;\
    enum class ElementRank:int{MACRO_MAP(_MAVLINK_REFLECT_ELEMENT_RANK, __VA_ARGS__) _NUM_MEMBERS};\
    static constexpr size_t NUM_MEMBERS = static_cast<size_t>(ElementRank::_NUM_MEMBERS);\
    template<size_t N> static constexpr size_t nth_offset_v = _nth_offset<N, members_tuple>::value;\
    static constexpr size_t BYTES_SIZE = nth_offset_v<std::tuple_size_v<members_tuple> - 1>;\
    static_assert(storage_capacity_v<Storage> >= BYTES_SIZE, "bytes size not enough");\
    Storage data;\
    MACRO_MAP(_MAVLINK_REFLECT_MEMBER_BYTES_PROXY, __VA_ARGS__) \
};\

#define MAVLINK_REFLECT_DEFMSG(NAME, NUM_MSG_ID, ...) _MAVLINK_REFLECT_STRUCT_BASE(NAME, NUM_MSG_ID, __VA_ARGS__)



}