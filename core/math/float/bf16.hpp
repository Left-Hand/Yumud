#pragma once

#include "fp32.hpp"
#include "core/math/real.hpp"

#include <cstdint>
#include <bit>

namespace ymd{

class OutputStream;

struct bf16 {
    struct Storage{
        #pragma pack(push, 1)
        uint16_t frac:7;
        uint16_t exp:8;
        uint16_t sign:1;
        #pragma pack(pop)

        constexpr Storage(){;}
        
        constexpr explicit Storage(const uint16_t val):
            frac(val & 0x7F),
            exp((val >> 7) & 0xFF),
            sign(val >> 15){;}


        constexpr uint16_t as_u16() const{
            return std::bit_cast<uint16_t>(*this);
        }

        constexpr Storage & operator =(const uint16_t _raw){
            *this = std::bit_cast<Storage>(_raw);
            return *this;
        }
    };

    static_assert(sizeof(Storage) == 2);

    Storage raw;

    constexpr bf16(){;}

    constexpr bf16 & operator = (const bf16 & other){
        raw = other.raw;
        return *this;
    }

    constexpr bf16(const bf16 & other):raw(other.raw){;}
    constexpr bf16(bf16 && other):raw(other.raw){;}

    constexpr uint16_t frac() const {return raw.frac;};
    constexpr uint16_t exp() const {return raw.exp;};
    constexpr uint16_t sign() const {return raw.sign;};

    template<typename T>
    requires(std::is_floating_point_v<T>)
    constexpr bf16(T fv) : raw(std::bit_cast<uint32_t>(float(fv)) >> 16) {}

    template<size_t Q>
    constexpr bf16(iq_t<Q> qv) : bf16(float(qv)) {}
    constexpr bf16(int iv) : bf16(float(iv)) {}
    constexpr bf16 operator -() const{
        return from_u16(raw.as_u16() ^ 0x8000);
    }

    // bf16 -> float
    explicit constexpr operator float() const {
        uint32_t f32_bits = uint32_t(raw.as_u16()) << 16;
        return std::bit_cast<float>(f32_bits);
    }

    explicit constexpr operator int() const {
        return int(float(*this));
    }

    template <size_t Q>
    explicit constexpr operator iq_t<Q>() const{
        return iq_t<Q>::from(float(*this));
    }

    friend OutputStream & operator << (OutputStream & os, const bf16 v);

    constexpr uint16_t as_u16() const {
        return raw.as_u16();
    }

    static constexpr bf16 from_u16(const uint16_t _raw){
        bf16 ret;
        ret.raw = _raw;
        return ret;
    }
private:
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
}
