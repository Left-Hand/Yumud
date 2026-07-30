#pragma once

#include <cstdint>
#include <span>
#include <bit>

// https://zhuanlan.zhihu.com/p/452697297

namespace ymd::mem{

struct [[nodiscard]] alignas(size_t) ArenaAllocater final{
    uint8_t * data;
    uint8_t * cursor;
    uint8_t * end;

    static ArenaAllocater from(std::span<uint8_t> buffer){
        ArenaAllocater self;
        self.data = buffer.data();
        self.cursor = self.data;
        self.end = self.data + buffer.size();
        return self;
    }

    constexpr size_t length() const {
        return end - cursor;
    }

    constexpr uint8_t * allocate(const size_t nbytes){
        if(cursor + nbytes > end) return nullptr;
        auto ret = cursor;
        cursor += nbytes;
        return ret;
    } 
};

}