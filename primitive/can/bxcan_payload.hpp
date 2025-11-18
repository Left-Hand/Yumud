#pragma once

#include <array>
#include <cstdint>
#include <span>

namespace ymd::hal{

struct [[nodiscard]] BxCanPayload{
public:    
    template<size_t Extents>
    requires ((Extents <= 8) || (Extents == std::dynamic_extent))
    static constexpr BxCanPayload from_bytes(const std::span<const uint8_t> bytes){
        if constexpr(Extents == std::dynamic_extent)
            if(bytes.size() > 8) __builtin_trap();
        BxCanPayload ret;
        std::copy(bytes.begin(), bytes.end(), ret.begin());
        ret.length_ = static_cast<uint8_t>(bytes.size());
        return ret;
    }
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint8_t * data() {return buf_.data();}
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint8_t * begin() {return buf_.begin();}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    const uint8_t * data() const noexcept {return buf_.data();}
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    const uint8_t * begin() const noexcept {return buf_.begin();}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint8_t size() const noexcept {return length_;}
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint8_t operator[](uint8_t i) const noexcept {return buf_[i];}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    std::span<const uint8_t> bytes() const noexcept {
        return std::span(buf_.data(), static_cast<size_t>(length_));
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    std::span<uint8_t> bytes() noexcept {
        return mut_bytes();
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    std::span<uint8_t> mut_bytes() noexcept {
        return std::span(buf_.data(), static_cast<size_t>(length_));
    }


    template<size_t I>
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint8_t get() const noexcept {return get_element<I>(*this);}

    template<size_t I>
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint8_t & get() {return get_element<I>(*this);}
private:
    alignas(4) std::array<uint8_t, 8> buf_;
    uint8_t length_;

    friend class CanClassicMsg;

    template<size_t I>
    [[nodiscard]] static __attribute__((always_inline)) constexpr 
    auto get_element(auto & self) noexcept {
        return std::get<I>(self.buf_);
    }
};

static_assert(sizeof(BxCanPayload) == 12);
}