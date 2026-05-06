#pragma once

#include <cstdint>
#include <array>
#include <span>

namespace ymd::robots::flyskytech::fsi6x{

namespace utils{

// 摇杆范围：240(-784) ~ 1024 ~ 1807(+783)
// 拨杆范围：UP:240(-784) ~ MID:1024 ~ DOWN:1807(+783)

static constexpr float parse_u11(const uint16_t bits){
    // 将11位值转换为浮点数，根据注释中的范围
    // 原始范围是240-1807，中心点是1024
    // 映射到-1.0到1.0之间
    return static_cast<float>(bits - 1024) / 783.0f;
}

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

    const size_t start_bit = idx * 11;
    const size_t byte0 = start_bit / 8;
    const size_t bit_offset = start_bit % 8;

    // 至少需要两个字节，最多三个
    uint32_t combined = bytes[byte0];
    if (byte0 + 1 < bytes.size()) {
        combined |= static_cast<uint32_t>(bytes[byte0 + 1]) << 8;
    }
    if (byte0 + 2 < bytes.size()) {
        combined |= static_cast<uint32_t>(bytes[byte0 + 2]) << 16;
    }

    // 从 combined 中提取从 bit_offset 开始的 11 位
    const uint32_t shifted = combined >> bit_offset;
    return static_cast<uint16_t>(shifted & 0x7FF);
}

static constexpr void set_u11_for_slice(
    std::span<uint8_t, 22> bytes,
    const size_t idx,
    const uint16_t value
) {
    if (idx >= 16) {
        #ifdef NDEBUG
        return 0;
        #else
        __builtin_unreachable();
        #endif
    }

    const uint16_t masked_value = value & 0x7FF;
    const size_t start_bit = idx * 11;
    const size_t byte0 = start_bit / 8;
    const size_t bit_offset = start_bit % 8;

    // 读取最多3个字节到一个32位临时变量中
    uint32_t combined = 0;
    if (byte0 < bytes.size()) combined |= static_cast<uint32_t>(bytes[byte0]);
    if (byte0 + 1 < bytes.size()) combined |= static_cast<uint32_t>(bytes[byte0 + 1]) << 8;
    if (byte0 + 2 < bytes.size()) combined |= static_cast<uint32_t>(bytes[byte0 + 2]) << 16;

    // 清除原来的11位
    const uint32_t mask = 0x7FF << bit_offset;
    combined &= ~mask;

    // 写入新值
    combined |= static_cast<uint32_t>(masked_value) << bit_offset;

    // 写回字节（只写实际存在的字节）
    if (byte0 < bytes.size()) {
        bytes[byte0] = static_cast<uint8_t>(combined & 0xFF);
    }
    if (byte0 + 1 < bytes.size()) {
        bytes[byte0 + 1] = static_cast<uint8_t>((combined >> 8) & 0xFF);
    }
    if (byte0 + 2 < bytes.size()) {
        bytes[byte0 + 2] = static_cast<uint8_t>((combined >> 16) & 0xFF);
    }
}


#pragma pack(push, 1)
struct [[nodiscard]]  U11X16 final{
    std::array<uint8_t, 22> bytes;


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

static_assert(sizeof(U11X16) == 22);
}


enum class [[nodiscard]] ConnectionCode:uint8_t{
    Connected = 0x00,
    Disconnected = 0x0c
};

#pragma pack(push, 1)
struct [[nodiscard]] Packet final{

    utils::U11X16 channels;

    ConnectionCode connection_code;
};

#pragma pack(pop)

static_assert(sizeof(Packet) == 23);

}