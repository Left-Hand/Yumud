#pragma once

#include "core/string/utils/c_style/strnlen.hpp"
#include "core/string/view/string_view.hpp"

namespace ymd::str{

template<size_t MaxExtents>
struct UCharsView{
    using Self = UCharsView;

    static_assert((MaxExtents != std::dynamic_extent), "must give a bounded size");
    // static_assert(MaxExtents > 0, "must give a positive size");
    static_assert(std::is_same_v<unsigned char, uint8_t>);


    static constexpr size_t CAPACITY = MaxExtents;
    const unsigned char * p_uchars_;
    size_t length_;

    [[nodiscard]] static imconstexpr Self from_str(const StringView str){
        return from_uchars(str.as_uchars());
    }

    [[nodiscard]] static imconstexpr Self from_str_bounded(const StringView str){
        return from_uchars_bounded(str.as_uchars());
    }

    template<size_t OtherExtents>
    requires ((OtherExtents == std::dynamic_extent) || (OtherExtents <= MaxExtents))
    [[nodiscard]] static constexpr Self from_uchars(const std::span<const unsigned char, OtherExtents> u_chars){

        //precheck extents
        if constexpr(OtherExtents == std::dynamic_extent){
            if(OtherExtents > MaxExtents) __builtin_trap();
        }else{
            static_assert(OtherExtents <= MaxExtents);
        }

        const size_t str_length = strnlen_from_left(u_chars.data(), std::min(OtherExtents, MaxExtents));

        return Self{
            .p_uchars_ = u_chars.data(),
            .length_ = str_length
        };
    }

    template<size_t OtherExtents>
    [[nodiscard]] static constexpr Self from_uchars_bounded(const std::span<const unsigned char, OtherExtents> u_chars){
        constexpr size_t LENGTH = std::min(OtherExtents, MaxExtents);
        static_assert(LENGTH != std::dynamic_extent);

        const size_t str_length = strnlen_from_left(u_chars.data(), LENGTH);

        return Self{
            .p_uchars_ = u_chars.data(),
            .length_ = str_length
        };
    }

    [[nodiscard]] constexpr uint8_t operator[](const size_t idx) const {
        if(idx >= length_)
            __builtin_trap();
        return p_uchars_[idx];
    }

    [[nodiscard]] constexpr std::span<const uint8_t> uchars() const {
        return {p_uchars_, length_};
    }

    [[nodiscard]] constexpr size_t length() const {

        if(p_uchars_ == nullptr) [[unlikely]]
            return 0;
        return length_;
    }

    [[nodiscard]] static constexpr size_t capacity() {
        return CAPACITY;
    }

    [[nodiscard]] imconstexpr StringView as_str() const {
        return StringView{
            reinterpret_cast<const char *>(p_uchars_), 
            length()
        };
    }
};

template<size_t Extents>
static constexpr UCharsView<Extents> make_uchars_view(
    const std::span<const uint8_t,Extents> uchars
){
    return UCharsView<Extents>::from_uchars(uchars);
}

template<size_t Extents>
static constexpr UCharsView<Extents> make_uchars_view_bounded(
    const StringView str
){
    return UCharsView<Extents>::from_str_bounded(str.as_uchars());
}

template<size_t Extents>
static constexpr Option<UCharsView<Extents>> try_make_uchars_view(
    const StringView str
){
    if(str.length() > Extents) return None;
    return Some(make_uchars_view_bounded<Extents>(str));
}


//CTAD
template<size_t Extents>
UCharsView(const std::span<const uint8_t, Extents>) -> UCharsView<Extents>;

}