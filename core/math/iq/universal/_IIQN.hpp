#pragma once

#include "rts_support.hpp"

template<size_t Q>
requires (Q < 64)
struct _iiq{
private:
    int64_t value_ = 0;
    __fast_inline constexpr _iiq<Q>(const int64_t value):
        value_(value){
    }
public:
    operator int64_t() = delete;

    static __fast_inline constexpr _iiq<Q> from_i64(const int64_t value){
        return _iiq<Q>(value);
    }

    __fast_inline constexpr int64_t to_i64() const {
        return value_;
    }

    __fast_inline constexpr uint64_t to_u64() const {
        return std::bit_cast<uint64_t>(value_);
    }

    __fast_inline constexpr bool signbit() const{
        return value_ & 0x80000000'0000000;
    }
    __fast_inline constexpr operator bool() const{return value_;}

    template<size_t P>
    __fast_inline constexpr explicit operator _iiq<P>() const{
        if constexpr (P > Q){
            return _iiq<P>::from_i64(int64_t(value_) << (P - Q));
        }else if constexpr (P < Q){
            return _iiq<P>::from_i64(int64_t(value_) >> (Q - P));
        }else{
            return _iiq<P>::from_i64(int64_t(value_));
        }
    }
};
