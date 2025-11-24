#pragma once

#include <cstdint>
#include "core/stream/ostream.hpp"

namespace ymd{


struct Nth{
    constexpr explicit Nth(const size_t count):
        count_(count){;}

    [[nodiscard]] constexpr size_t count() const {
        return count_;
    }
private:
    size_t count_;
};

consteval Nth operator"" _nth(uint64_t count){
    return Nth(count);
};

}