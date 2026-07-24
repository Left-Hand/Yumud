#pragma once

#include <cstdint>
#include <array>
#include "core/utils/Option.hpp"
#include "core/math/fixed/fixed.hpp"

namespace ymd::drivers::bissc{

static constexpr uint8_t fast_clzll(const uint64_t x){
    if (x == 0) return 64;  // 特殊情况：输入为0时，有64个前导零
    const uint32_t low = x & 0xFFFFFFFF;
    const uint32_t high = x >> 32;
    if(high){
        return (__builtin_clz(high));
    }else{
        return (__builtin_clz(low) + 32);
    }
}

#if 0
static constexpr std::array<uint8_t, 64> CRC6_TABLE = []{
    constexpr uint8_t POLY = 0x43;
    std::array<uint8_t, 64> table;
	for(size_t i = 0; i < 64; i++){
		int crc = i;

		for (size_t j = 0; j < 6; j++){
			if (crc & 0x20){
				crc <<= 1;
				crc ^= POLY;
			} else {
				crc <<= 1;
			}
		}
		table[i] = crc;
	}
    return table;
}();
#else

static constexpr std::array<uint8_t, 64> CRC6_TABLE{
    0x00, 0x03, 0x06, 0x05, 0x0C, 0x0F, 0x0A, 0x09,
    0x18, 0x1B, 0x1E, 0x1D, 0x14, 0x17, 0x12, 0x11,
    0x30, 0x33, 0x36, 0x35, 0x3C, 0x3F, 0x3A, 0x39,
    0x28, 0x2B, 0x2E, 0x2D, 0x24, 0x27, 0x22, 0x21,
    0x23, 0x20, 0x25, 0x26, 0x2F, 0x2C, 0x29, 0x2A,
    0x3B, 0x38, 0x3D, 0x3E, 0x37, 0x34, 0x31, 0x32,
    0x13, 0x10, 0x15, 0x16, 0x1F, 0x1C, 0x19, 0x1A,
    0x0B, 0x08, 0x0D, 0x0E, 0x07, 0x04, 0x01, 0x02,
};
#endif



static constexpr uint8_t calc_crc(const uint64_t data, const uint8_t data_width, const uint8_t crc_width){
    // Fallback to bitwise computation for non-standard CRC sizes.
    //
    // Note, this is almost certain too slow to use in firmware, and
    // is included just for reference.
    const uint32_t polynomial = (1 << crc_width) | 1;
    uint32_t crc = 0;
    for (int i = data_width - 1; i >= 0; i--) {
        const bool bit = (data >> i) & 1;
        const bool msb = (crc >> (crc_width - 1)) & 1;
        crc <<= 1;
        if (bit ^ msb) {
            crc ^= polynomial;
        }
    }
    return crc & ((1 << crc_width) - 1);
}

static constexpr uint8_t calc_crc6(const uint64_t data, const uint8_t data_width){
    uint8_t crc = 0;
    if (data_width > 30) {
        crc = CRC6_TABLE[(data >> 54) & 0x3F];
        crc = CRC6_TABLE[((data >> 48) & 0x3F) ^ crc];
        crc = CRC6_TABLE[((data >> 42) & 0x3F) ^ crc];
        crc = CRC6_TABLE[((data >> 36) & 0x3F) ^ crc];
        crc = CRC6_TABLE[((data >> 30) & 0x3F) ^ crc];
    }
    crc = CRC6_TABLE[((data >> 24) & 0x3F) ^ crc];
    crc = CRC6_TABLE[((data >> 18) & 0x3F) ^ crc];
    crc = CRC6_TABLE[((data >> 12) & 0x3F) ^ crc];
    crc = CRC6_TABLE[((data >> 6) & 0x3F) ^ crc];
    crc = CRC6_TABLE[(data & 0x3F) ^ crc];
    return crc;
}

static constexpr uint8_t calc_checksum(const uint32_t d32){
    uint8_t checksum;  // CRC种子为0b000000
    checksum = ((d32 >> 30) & 0x03);  // 获取最高2位
    checksum = CRC6_TABLE[static_cast<size_t>(((d32 >> 24) & 0x3F) ^ checksum)];
    checksum = CRC6_TABLE[static_cast<size_t>(((d32 >> 18) & 0x3F) ^ checksum)];
    checksum = CRC6_TABLE[static_cast<size_t>(((d32 >> 12) & 0x3F) ^ checksum)];
    checksum = CRC6_TABLE[static_cast<size_t>(((d32 >> 6) & 0x3F) ^ checksum)];
    checksum = CRC6_TABLE[static_cast<size_t>(((d32 >> 0) & 0x3F) ^ checksum)];
    checksum = 0x3F & ~checksum; // CRC输出取反
    return checksum;
}

//通过8B的字节和分辨率计算区间为[0, 1)的单圈位置
//返回为空说明校验位失败
static constexpr Option<uq32> parse(uint64_t d64, const size_t resolution){
    d64 <<= fast_clzll(d64);  // 左移，跳过前导零
    d64 &= 0x3FFFFFFFFFFFFFFF;  // 清除前两位（保留除前导零外的62位）

    const size_t num_bit =  64-fast_clzll(d64);  // 计算有效位数
    if ( num_bit >= (resolution + 10) ) {
        d64 = d64 >> (num_bit-(resolution + 10));  // 如果有效位过多，则右移截断
    }

    const uint8_t recv_checksum = d64 & 0x3F;  // 提取6位CRC
    const uint32_t d32 = (d64 >> 6) & ((1<<(resolution + 2)) - 1);  // 提取位置数据+错误/警告位
    const uint32_t lap32 = ((d32 >> 2) & ((1<<resolution) - 1)) << (32 - resolution);  // 提取纯位置数据

    const uint32_t actual_checksum = calc_checksum(d32);

    if(actual_checksum != recv_checksum) return None;
    return Some(uq32::from_bits(lap32));
}


static constexpr uint64_t 
encode(
    uq32 position,
    size_t resolution,
    bool cds = false,
    bool error_flag = false,
    bool warning_flag = false
) {
    // 1. 构建 d32: [位置(res位)][Error][Warning]
    uint32_t d32 = 0;
    d32 |= (position.to_bits() >> (32 - resolution)) << 2;
    if (error_flag) d32 |= (1u << 1);
    if (warning_flag) d32 |= (1u << 0);
    
    // 2. 计算 CRC (6位)
    uint8_t checksum = calc_checksum(d32);
    
    // 3. 构建完整的数据段（不含 Start 和 CDS）
    //    格式: [位置(res位)][Error][Warning][CRC(6位)]  -> 总共 res+2+6 = res+8 位
    uint64_t data_part = (static_cast<uint64_t>(d32) << 6) | checksum;
    
    // 4. 添加 Start bit (1) 和 CDS bit，形成最终帧
    //    总位数 = 1(start) + 1(cds) + (res+8) = res+10
    uint64_t frame = 0;
    frame |= (1ULL << (resolution + 9));        // Start bit 放在最高位（相对于 frame 的 MSB）
    if (cds) frame |= (1ULL << (resolution + 8)); // CDS bit 次高位
    frame |= data_part;                          // 低 (res+8) 位放数据段
    
    // 5. 左移到 64 位的高位（模拟实际传输时的对齐方式）
    //    这样接收端通过左移跳过前导零后，就能得到上面左对齐的 frame
    const size_t total_bits = resolution + 10;
    frame <<= (64 - total_bits);
    
    return frame;
}

}