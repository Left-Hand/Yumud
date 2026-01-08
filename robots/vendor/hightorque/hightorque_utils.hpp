#pragma once

#include "hightorque_primitive.hpp"
#include "core/utils/bits/bits_caster.hpp"


namespace ymd::robots::hightorque{

namespace utils{
using namespace primitive;

template<typename T>
static constexpr ElementType element_type_v = [] -> ElementType{
    static_assert(not std::is_reference_v<T>);
    if constexpr(std::is_integral_v<T>){
        static_assert(sizeof(T) <= 4);
        if constexpr(sizeof(T) == 1) return ElementType::B1;
        else if constexpr(sizeof(T) == 2) return ElementType::B2;
        else if constexpr (sizeof(T) == 4) return ElementType::B4;
        __builtin_unreachable();
    }else if constexpr(std::is_same_v<T, float>){
        return ElementType::Float;
    }else if constexpr(requires {T::ELEMENT_TYPE;}){
        return T::ELEMENT_TYPE;
    }else if constexpr(std::is_same_v<T, Mode>){
        return ElementType::B1;
    }else{
        __builtin_unreachable();
    }
}();



namespace details{
template<typename... Types>
struct common_element_type;

template<typename T>
struct common_element_type<T> {
    static constexpr ElementType value = element_type_v<T>;
};

template<typename T, typename U>
struct common_element_type<T, U> {
    static constexpr ElementType value = element_type_v<T>;
    static_assert(element_type_v<T> == element_type_v<U>, "Element types must be the same");
};

template<typename T, typename U, typename... Rest>
struct common_element_type<T, U, Rest...> {
    static constexpr ElementType value = [] {
        static_assert(element_type_v<T> == element_type_v<U>, "Element types must be the same");
        static_assert(element_type_v<T> == common_element_type<U, Rest...>::value, "Element types must be the same");
        return element_type_v<T>;
    }();
};
}

template<typename ... Args>
static constexpr ElementType common_element_type_v = []{
    return details::common_element_type<std::decay_t<Args> ...>::value;
}();

static consteval size_t element_type_to_size(ElementType element_type){
    switch(element_type){
        case ElementType::B1: return 1;
        case ElementType::B2: return 2;
        case ElementType::B4: return 4;
        case ElementType::Float: return 4;
    }
    __builtin_unreachable();
}

template<typename ... Args>
static constexpr SlotSpecifier make_slot_specifier(
    const SlotCommand slot_command
){
    static_assert(sizeof...(Args) <= 0b11);
    return SlotSpecifier{
        sizeof...(Args), 
        common_element_type_v<Args ...>,
        slot_command
    };
}


template<std::endian E, typename T>
static constexpr void int_fill_bytes(std::span<uint8_t, sizeof(T)> bytes, const T value){
    static_assert(sizeof(T) <= 4);
    
    if constexpr (E == std::endian::little) {
        // Native endianness - fill bytes directly
        T temp_value = value;
        for (size_t i = 0; i < sizeof(T); ++i) {
            bytes[i] = static_cast<uint8_t>(temp_value & 0xFF);
            temp_value >>= 8;
        }
    } else {
        // Opposite endianness - reverse the byte order
        T temp_value = value;
        for (size_t i = 0; i < sizeof(T); ++i) {
            bytes[sizeof(T) - 1 - i] = static_cast<uint8_t>(temp_value & 0xFF);
            temp_value >>= 8;
        }
    }
}

template<ElementType element_type, typename T>
static constexpr auto element_to_bits(const T element){
    if constexpr(element_type == ElementType::B1) return std::bit_cast<uint8_t>(element);
    else if constexpr(element_type == ElementType::B2) return std::bit_cast<uint16_t>(element);
    else if constexpr(element_type == ElementType::B4) return std::bit_cast<uint32_t>(element);
    else if constexpr(element_type == ElementType::Float) return std::bit_cast<uint32_t>(element);
    else __builtin_unreachable();
}

template<size_t N, typename ... Args>
static constexpr void fill_bytes(
    const std::span<uint8_t, N> bytes,
    const SlotCommand slot_command,
    const RegAddr reg_addr,
    Args && ... args 
){
    constexpr auto common_element_type = common_element_type_v<std::decay_t<Args> ...>;
    constexpr size_t total_elements_size = sizeof...(Args) * element_type_to_size(common_element_type);
    constexpr size_t required_size = (1 + // Slot specifier 
        1 + // Reg addr
        total_elements_size
    );
    static_assert(
        required_size == N
    );

    const auto slot_specifier = make_slot_specifier<Args ...>(slot_command);
    bytes[0] = slot_specifier.to_bits();
    bytes[1] = static_cast<uint8_t>(reg_addr);

    size_t offset = 2;

    auto fill_element = [&]<typename T>(const T element) {
        constexpr auto element_type = common_element_type;
        constexpr size_t element_size = element_type_to_size(element_type);

        const auto bits = utils::element_to_bits<element_type>(element);
        // Fill bytes
        int_fill_bytes<std::endian::little>(std::span<uint8_t, element_size>(bytes.data() + offset, element_size), bits);
        offset += element_size;
    };

    (fill_element(std::forward<Args>(args)), ...);
}


struct [[nodiscard]] SlotBuilder{
    SlotCommand slot_command;
    RegAddr reg_addr;

    template<size_t N, typename ... Args>
    constexpr void build(std::span<uint8_t, N> bytes, Args && ... args) const{
        utils::fill_bytes(bytes, slot_command, reg_addr, std::forward<Args>(args)...);
    }
};

}
}