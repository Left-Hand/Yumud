#pragma once

#include <cstdint>
#include <span>

namespace ymd::robots::xdrive{


struct [[nodiscard]] BytesReader final{
    explicit constexpr BytesReader(std::span<const uint8_t> bytes) : 
        cursor_(bytes.data()), end_(bytes.data() + bytes.size()) {}


    template<typename T>
    [[nodiscard]] constexpr T fetch(){
        if(remaining().size() < sizeof(T))
            __builtin_trap();
        return bytes_to_int_le<T>(fetch_bytes<sizeof(T)>());
    }

private:
    // std::span<const uint8_t> bytes_;
    const uint8_t * cursor_;
    const uint8_t * end_;

    template<size_t N>
    [[nodiscard]] constexpr std::span<const uint8_t, N> fetch_bytes(){
        const auto ret = std::span<const uint8_t, N>(cursor_, N);
        // bytes_ = std::span<const uint8_t>(bytes_.data() + N, bytes_.size() - N);
        cursor_ += N;
        return ret;
    }

    [[nodiscard]] constexpr std::span<const uint8_t> remaining() const noexcept {
        return {cursor_, end_};
    }
};


}