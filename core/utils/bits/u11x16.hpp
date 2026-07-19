#pragma once

#include "bit_cursor.hpp"
#include <cstdint>
#include <span>

namespace ymd{



#pragma pack(push, 1)

template<typename Storage>
struct [[nodiscard]]  U11X16_Interpreter final{
    Storage bytes;

private:
    static constexpr uint16_t get_u11_from_slice(
        std::span<const uint8_t, 22> bytes,
        const size_t idx
    ) {
        if (idx >= 16) {
            #ifdef NDEBUG
            return 0;
            #else
            __builtin_unreachable();
            #endif
        }

        std::array<uint8_t, 2> buf = {0, 0};
        bit_cursor_load_bits(bytes.data(), idx * 11, buf.data(), 11);
        return buf[0] | (buf[1] << 8);
    }

    static constexpr void set_u11_for_slice(
        std::span<uint8_t, 22> bytes,
        const size_t idx,
        const uint16_t value
    ) {
        if (idx >= 16) {
            #ifdef NDEBUG
            return;
            #else
            __builtin_unreachable();
            #endif
        }

        const std::array<uint8_t, 2> buf = {
            uint8_t(value & 0xff),
            uint8_t((value >> 8) & 0x07)
        };

        bit_cursor_store_bits(bytes.data(), idx * 11, buf.data(), 11);
    }

    friend class Proxy;
    friend class MutProxy;

public:
    struct [[nodiscard]] MutProxy final{
        std::span<uint8_t, 22> bytes;
        size_t idx;

        constexpr MutProxy & operator =(const uint16_t data){
            set_u11_for_slice(bytes, idx, data);
            return *this;
        };
    };

    struct [[nodiscard]] Proxy final{
        std::span<const uint8_t, 22> bytes;
        size_t idx;
        
        constexpr operator uint16_t() const {
            return get_u11_from_slice(bytes, idx);
        }
    };

    constexpr MutProxy operator [](const size_t idx){
        return MutProxy{std::span<uint8_t, 22>{bytes.data(), bytes.size()}, idx};
    }
    
    constexpr Proxy operator[](size_t idx) const {
        return Proxy{std::span<const uint8_t, 22>{bytes.data(), bytes.size()}, idx};
    }

};

#pragma pack(pop)

using U11X16Owned = U11X16_Interpreter<std::array<uint8_t, 22>>;
using U11X16View = U11X16_Interpreter<std::span<const uint8_t, 22>>;

static_assert(sizeof(U11X16Owned) == 22);
}