#pragma once

#include "fp32.hpp"
#include "core/math/real.hpp"

#include <cstdint>
#include <bit>
#include <array>

namespace ymd{

class OutputStream;

struct [[nodiscard]] bf16 {
    struct [[nodiscard]] Storage{
        uint16_t frac:7;
        uint16_t exp:8;
        uint16_t sign:1;

        static constexpr Storage from_bits(const uint16_t bits){
            return Storage{
                .frac = static_cast<uint16_t>(bits & 0x7F),
                .exp = static_cast<uint16_t>((bits >> 7) & 0xFF),
                .sign = static_cast<uint16_t>(bits >> 15)
            };
        }

        [[nodiscard]] constexpr uint16_t as_bits() const{
            return std::bit_cast<uint16_t>(*this);
        }

    };

    static_assert(sizeof(Storage) == 2);



    constexpr bf16(){;}

    constexpr bf16 & operator = (const bf16 & other){
        storage_ = other.storage_;
        return *this;
    }

    constexpr bf16(const bf16 & other):storage_(other.storage_){;}
    constexpr bf16(bf16 && other):storage_(other.storage_){;}

    [[nodiscard]] constexpr uint16_t frac() const {return storage_.frac;};
    [[nodiscard]] constexpr uint16_t exp() const {return storage_.exp;};
    [[nodiscard]] constexpr uint16_t sign() const {return storage_.sign;};

    template<typename T>
    requires(std::is_floating_point_v<T>)
    constexpr bf16(T fv) : storage_(Storage::from_bits(
        static_cast<uint16_t>(std::bit_cast<uint32_t>(float(fv)) >> 16))){}

    template<size_t Q>
    constexpr bf16(fixed_t<Q, int32_t> qv) : bf16(float(qv)) {}
    constexpr bf16(int iv) : bf16(float(iv)) {}
    constexpr bf16 operator -() const{
        return from_bits(storage_.as_bits() ^ 0x8000);
    }

    // bf16 -> float
    explicit constexpr operator float() const {
        uint32_t f32_bits = uint32_t(storage_.as_bits()) << 16;
        return std::bit_cast<float>(f32_bits);
    }

    explicit constexpr operator int() const {
        return int(float(*this));
    }

    template <size_t Q>
    explicit constexpr operator fixed_t<Q, int32_t>() const{
        return fixed_t<Q, int32_t>::from(float(*this));
    }

    friend OutputStream & operator << (OutputStream & os, const bf16 v);

    [[nodiscard]] constexpr uint16_t as_bits() const {
        return storage_.as_bits();
    }

    [[nodiscard]] constexpr std::array<uint8_t, 2> to_le_bytes() const {
        const uint16_t bits = as_bits();
        return std::array<uint8_t, 2>{
            static_cast<uint8_t>(bits & 0xff),
            static_cast<uint8_t>(bits >> 8),
        };
    }

    static constexpr bf16 from_bits(const uint16_t bits){
        return bf16(Storage::from_bits(bits));
    }
private:
    Storage storage_;

    constexpr explicit bf16(const Storage & sto):
        storage_(sto){;}
};

consteval bf16 operator"" _bf16(long double x){
    return bf16(x);
}

consteval bf16 operator"" _bf16(unsigned long long x){
    return bf16(x);
}


}

namespace std{

    template<>
    struct is_arithmetic<ymd::bf16> : std::true_type {};
    template<>
    struct is_floating_point<ymd::bf16> : std::true_type {};
    template<>
    struct is_signed<ymd::bf16> : std::true_type {};
}
