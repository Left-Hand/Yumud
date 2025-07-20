#pragma once

#include <cstdint>

namespace ymd{

struct BitsQueue{
    constexpr explicit BitsQueue() = default;
    constexpr bool pop_bit(){
        const auto ret = buf_ & 0x01;
        length_ -= 1;
        buf_ = buf_ >> 1;
        return ret;
    }

    template<size_t N>
    constexpr uint32_t pop_bits(){
        constexpr uint32_t MASK = (1 << N) - 1;
        const auto ret = buf_ & MASK;
        length_ -= N;
        buf_ = buf_ >> N;
        return ret;
    }

    constexpr void push_bit(bool bit){
        buf_ = buf_ | (bit << (length_++));
    }

    template<size_t N>
    constexpr void push_bits(const uint32_t bits){
        buf_ = buf_ | (bits << (length_++));
    }

    constexpr uint32_t pop_remaining(){
        length_ = 0;
        return buf_;
    }

    constexpr uint32_t as_u64() const {
        return buf_;
    }

    constexpr size_t available() const {
        return length_;
    }

    constexpr size_t writable_capacity() const {
        return 32 - length_;
    }
private:
    uint64_t buf_ = 0;
    size_t length_ = 0;

    static void static_test(){
        {
            constexpr auto queue = []{
                auto q = BitsQueue{};
                q.push_bits<5>(0b10111);
                q.pop_bits<2>();
                return q;
            }();
            static_assert(queue.as_u64() == 0b101);
        }
    }
};


}