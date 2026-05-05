#pragma once

#include "im6014_primitive.hpp"


namespace ymd::robots::unitree::im6014{

namespace utils{

static constexpr uint8_t * ptr_push_u16(uint8_t * ptr, const uint16_t bits){
    ptr[0] = bits & 0xff;
    ptr[1] = (bits >> 8) & 0xff;
    return ptr + 2;
}

static constexpr uint8_t * ptr_push_u32(uint8_t * ptr, const uint32_t bits){
    ptr[0] = bits & 0xff;
    ptr[1] = (bits >> 8) & 0xff;
    ptr[2] = (bits >> 16) & 0xff;
    ptr[3] = (bits >> 24) & 0xff;
    return ptr + 4;
}



template<typename T, uint32_t SCALE>
struct mul;

template<std::floating_point T, uint32_t SCALE>
struct mul<T, SCALE>{
    static constexpr int32_t calc(const T x){
        return static_cast<int32_t>(x * SCALE);
    }
};

template<size_t Q, uint32_t SCALE>
struct mul<math::fixed<Q, int32_t>, SCALE>{
    static constexpr int32_t calc(const math::fixed<Q, int32_t> x){
        constexpr int32_t FACTOR = SCALE * (1u << (32 - x.NUM_Q));
        return static_cast<int32_t>(
            (int64_t(x.to_bits()) * FACTOR) >> 32
        );
    }
};

template<size_t Q>
struct mul<math::fixed<Q, int32_t>, 32768u>{
    static constexpr int32_t calc(const math::fixed<Q, int32_t> x){
        constexpr int32_t FACTOR = 32768u;
        return static_cast<int32_t>(
            (int64_t(x.to_bits()) * FACTOR) >> x.NUM_Q
        );
    }
};

template<size_t Q>
struct mul<math::fixed<Q, int32_t>, 51200u>{
    static constexpr int32_t calc(const math::fixed<Q, int32_t> x){
        constexpr int32_t FACTOR = 51200u;
        return static_cast<int32_t>(
            (int64_t(x.to_bits()) * FACTOR) >> x.NUM_Q
        );
    }
};


template<typename Meta, typename T>
static constexpr int32_t bounded_encode(T x){
    if constexpr(std::is_floating_point_v<T>){
        constexpr T min = (T)(Meta::MIN_VALUE);

        constexpr T max = (T)(Meta::MAX_VALUE);

        x = std::clamp(x, min, max);
    }else{
        constexpr T min = (T)std::max(
            (long double)std::numeric_limits<T>::min(), 
            (long double)(Meta::MIN_VALUE));
    
        constexpr T max = (T)std::min(
            (long double)std::numeric_limits<T>::max(), 
            (long double)(Meta::MAX_VALUE));

        x = std::clamp(x, min, max);
    }


    int32_t bits = utils::mul<T, Meta::RATIO>::calc(x);
    return bits;
}

template<typename T, uint32_t SCALE>
struct div;

template<>
struct div<iq16, 100>{
    static constexpr iq16 calc(int32_t bits){
        constexpr int32_t FACTOR = int32_t(0.01 * (1ull << 32));
        return iq16::from_bits((int64_t(bits) * FACTOR) >> 16);
    }
};

template<uint32_t SCALE>
struct div<float, SCALE>{
    static constexpr float INV_SCALE = 1.0f / SCALE;
    static constexpr float calc(int32_t bits){
        return static_cast<float>(bits) * INV_SCALE;
    }
};


template<typename Meta, typename T>
static constexpr T bounded_decode(int32_t x){
    return div<T, Meta::RATIO>::calc(x);
}


template<typename D, uint32_t RATIO, typename T>
[[nodiscard]] static constexpr D bounded_scale(T x){
    int32_t bits = utils::mul<T, RATIO>::calc(x);
    return std::clamp(bits, 
        (int32_t)std::numeric_limits<D>::min(), 
        (int32_t)std::numeric_limits<D>::max());
}


}
}