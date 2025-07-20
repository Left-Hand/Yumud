#pragma once

#include "rts_support.hpp"


template<size_t Q>
requires (Q < 32)
struct _iq{
private:
    int32_t value_;
    __fast_inline constexpr explicit _iq<Q>(const int32_t value):
        value_(value){
    }
public:
    operator int32_t() = delete;

    constexpr _iq(){;}
    static __fast_inline constexpr _iq<Q> from_i32(const int32_t value){
        return _iq<Q>(value);
    }

    __fast_inline constexpr int32_t as_i32() const {
        return value_;
    }

    __fast_inline constexpr uint32_t as_u32() const {
        return std::bit_cast<uint32_t>(value_);
    }

    __fast_inline constexpr bool signbit() const{
        return value_ & 0x80000000;
    }

    __fast_inline constexpr operator bool() const{return value_;}

    template<size_t P>
    __fast_inline constexpr explicit operator _iq<P>() const{
        return transform<P>(value_);
    }

    template<size_t P>
    __fast_inline static constexpr _iq<P> transform(const int32_t value){
        if constexpr (P > Q){
            return _iq<P>::from_i32(int32_t(value) << (P - Q));
        }else if constexpr (P < Q){
            return _iq<P>::from_i32(int32_t(value) >> (Q - P));
        }else{
            return _iq<P>::from_i32(int32_t(value));
        }
    }
};
