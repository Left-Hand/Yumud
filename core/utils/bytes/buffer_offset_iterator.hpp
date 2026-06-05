#pragma once

#include <type_traits>
#include <span>
#include <bit>

#include "core/constants/enums.hpp"


namespace ymd{

template<typename std::endian>
struct BufferOffsetIterator;

template<>
struct BufferOffsetIterator<std::endian::little>{
    using Self = BufferOffsetIterator;

    struct Config{
        size_t dsize;
        size_t quantity;
    };

    const size_t num_bytes;
    size_t offset;

    static constexpr Self from(const Config cfg){
        return Self{
            .num_bytes = cfg.dsize * cfg.quantity,
            .offset = 0
        };
    }

    [[nodiscard]] constexpr bool has_next() const {
        return offset < num_bytes;
    }

    [[nodiscard]] constexpr size_t next() {
        const size_t ret = offset;
        offset++;
        return offset;
    }
};

template<>
struct BufferOffsetIterator<std::endian::little>{
    using Self = BufferOffsetIterator;

    struct Config{
        size_t dsize;
        size_t quantity;
    };

    const size_t num_bytes;
    size_t offset;

    static constexpr Self from(const Config cfg){
        return Self{
            .num_bytes = cfg.dsize * cfg.quantity,
            .offset = 0
        };
    }

    [[nodiscard]] constexpr bool has_next() const {
        return offset < num_bytes;
    }

    [[nodiscard]] constexpr size_t next() {
        const size_t ret = offset;
        offset++;
        return offset;
    }
};

struct BytesIterator{
    const size_t dsize;
    const size_t quantity;
    const std::endian endian;

    union State{
        size_t le_offset;
        struct {
            size_t be_element_offset;
            size_t be_element_index;
        };
    };


};


};