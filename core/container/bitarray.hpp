#pragma once

#include <cstddef>
#include <array>

namespace ymd{

namespace details{

template <size_t NUM_BITS>
struct [[nodiscard]] _BitArray_Storage;
}
template<size_t NUM_BITS>
struct [[nodiscard]] BitArray {
private:
    struct [[nodiscard]] MutBitProxy {
    public:
        constexpr explicit MutBitProxy(uint8_t& byte, const uint8_t offset)
            : byte_(byte), offset_(offset) {}

        // 转换为 bool（读取位值）
        [[nodiscard]] constexpr operator bool() const {
            return (byte_ >> offset_) & 1;
        }

        // 赋值操作（设置位值）
        constexpr MutBitProxy& operator=(bool value) {
            if (value) {
                byte_ |= (1 << offset_);
            } else {
                byte_ &= ~(1 << offset_);
            }
            return *this;
        }

        // 禁止拷贝构造
        MutBitProxy(const MutBitProxy&) = delete;
        MutBitProxy& operator=(const MutBitProxy&) = delete;

    private:
        uint8_t& byte_;
        uint8_t offset_;
    };

    struct [[nodiscard]] BitProxy {
    public:
        constexpr explicit BitProxy(const uint8_t byte, const uint8_t offset)
            : byte_(byte), offset_(offset) {}
        // 获取位值
        [[nodiscard]] constexpr bool get() const {
            return (byte_ >> offset_) & 1;
        }

        // 转换为 bool（读取位值）
        [[nodiscard]] constexpr operator bool() const {
            return get();
        }

    private:
        const uint8_t byte_;
        const uint8_t offset_;
    };

public:
    static_assert(NUM_BITS % 8 == 0);
    static constexpr size_t NUM_BYTES = NUM_BITS / 8;

    static constexpr BitArray from_zero() {
        auto ret = BitArray();
        ret.fill(0);
        return ret;
    }

    static constexpr BitArray from_uninitialized() {
        return BitArray();
    }


    // 设置所有位为指定值
    constexpr void fill(bool value) {
        uint8_t fill_byte = value ? 0xFF : 0x00;
        std::fill_n(flag_.begin(), NUM_BYTES, fill_byte);
    }


    // 可修改的位访问操作符 []
    [[nodiscard]] constexpr MutBitProxy operator[](size_t index) {
        if(index >= NUM_BITS) [[unlikely]]
            on_out_of_range();
        return MutBitProxy(flag_[index / 8], index % 8);
    }

    // 只读的位访问操作符 []
    [[nodiscard]] constexpr BitProxy operator[](size_t index) const {
        if(index >= NUM_BITS) [[unlikely]]
            on_out_of_range();
        return BitProxy(flag_[index / 8], index % 8);
    }

    // 获取位总数
    [[nodiscard]] constexpr size_t size() const {
        return NUM_BITS;
    }

    // 检查是否所有位都是 0
    [[nodiscard]] constexpr bool is_zero() const {
        return std::all_of(flag_.begin(), flag_.end(), [](uint8_t byte) {
            return byte == 0;
        });
    }

    // 检查是否所有位都是 1
    [[nodiscard]] constexpr bool is_full() const {
        return std::all_of(flag_.begin(), flag_.end(), [](uint8_t byte) {
            return byte == 0xFF;
        });
    }

    // 统计设置为 1 的位数
    [[nodiscard]] constexpr size_t count() const {
        size_t result = 0;
        for (uint8_t byte : flag_) {
            result += __builtin_popcount(byte); // GCC/Clang
        }
        return result;
    }

private:
    constexpr explicit BitArray() {
    }

    std::array<uint8_t, NUM_BYTES> flag_ = {};

    void on_out_of_range(){
        __builtin_abort();
    }
};
}