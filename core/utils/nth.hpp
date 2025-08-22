#pragma once

#include <cstdint>

namespace ymd{


struct Nth{
    constexpr explicit Nth(const size_t count):
        count_(count){;}

    constexpr size_t count() const {
        return count_;
    }
private:
    size_t count_;
};

consteval Nth operator"" _nth(uint64_t count){
    return Nth(count);
};

}