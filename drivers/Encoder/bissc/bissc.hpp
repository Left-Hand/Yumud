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