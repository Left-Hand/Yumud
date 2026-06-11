#pragma once

#include <cstdint>
#include <cstddef>
#include <bit>
#include <array>
#include <bitset>

namespace ymd{


static constexpr void bit_cursor_store_bits(
    __restrict__ uint8_t * ptr,
    const size_t bit_offset,
    __restrict__ const uint8_t * data,
    const size_t data_width
);

static constexpr void bit_cursor_load_bits(
    __restrict__ const uint8_t * ptr,
    const size_t bit_offset,
    __restrict__ uint8_t * data,
    const size_t data_width
);


struct [[nodiscard]] BitCursor final {
    uint8_t* ptr;
    size_t bit_offset;

    [[nodiscard]] constexpr bool is_aligned_to_byte() const {
        return (bit_offset & 0b111) == 0;
    }

    [[nodiscard]] /* constexpr */ bool is_aligned_to_word() const {
        if((bit_offset & 0b111) != 0) return false;
        const uint8_t * end = ptr + (bit_offset >> 3);
        const uintptr_t byte_addr = reinterpret_cast<uintptr_t>(end);
        return (byte_addr & (sizeof(size_t) - 1)) == 0;
    }
    
    constexpr void push_bits(const uint8_t* data, const size_t data_width) {
        auto& self = *this;
        bit_cursor_store_bits(self.ptr, self.bit_offset, data, data_width);
        self.bit_offset += data_width;
    }

    template<typename T>
    constexpr void push_int(const T int_val) {
        static constexpr size_t LEN = sizeof(T);
        const auto src = std::bit_cast<std::array<uint8_t, LEN>>(int_val);
        push_bits(src.data(), LEN * 8);
    }

    template<typename T>
    constexpr void push_int(const T int_val, const size_t width) {
        static constexpr size_t LEN = sizeof(T);
        const auto src = std::bit_cast<std::array<uint8_t, LEN>>(int_val);
        push_bits(src.data(), width);
    }

    template<size_t N>
    constexpr void push_bitset(const std::bitset<N>& bitset) {
        static_assert(N > 0, "Bitset size must be positive");
        
        constexpr size_t byte_size = (N + 7) / 8;
        std::array<uint8_t, byte_size> buffer{};
        
        for (size_t i = 0; i < N; ++i) {
            if (bitset.test(i)) {
                size_t byte_idx = i / 8;
                size_t bit_idx = i % 8;
                buffer[byte_idx] |= (1 << bit_idx);
            }
        }
        
        push_bits(buffer.data(), N);
    }

    // 跳过指定位数
    constexpr void skip_bits(const size_t bits) {
        bit_offset += bits;
    }
};

struct [[nodiscard]] BitReader final {
    const uint8_t* ptr;
    size_t bit_offset;

    [[nodiscard]] constexpr bool is_aligned_to_byte() const {
        return (bit_offset & 0b111) == 0;
    }

    [[nodiscard]] /* constexpr */ bool is_aligned_to_word() const {
        if ((bit_offset & 0b111) != 0) return false;
        const uint8_t* end = ptr + (bit_offset >> 3);
        const uintptr_t byte_addr = reinterpret_cast<uintptr_t>(end);
        return (byte_addr & (sizeof(size_t) - 1)) == 0;
    }
    
    // 从当前位位置读取指定位数的数据到输出缓冲区
    constexpr void pull_bits(uint8_t* out_data, const size_t data_width) {
        auto& self = *this;
        bit_cursor_load_bits(self.ptr, self.bit_offset, out_data, data_width);
        self.bit_offset += data_width;
    }
    
    // 读取并返回整数类型（小端序）
    template<typename T>
    constexpr T pull_int() {
        static constexpr size_t LEN = sizeof(T);
        std::array<uint8_t, LEN> buffer{};
        pull_bits(buffer.data(), LEN * 8);
        return std::bit_cast<T>(buffer);
    }
    
    // 读取指定宽度的整数（宽度以位为单位）
    template<typename T>
    constexpr T pull_int(const size_t width) {
        static_assert(std::is_integral_v<T>, "T must be integral type");
        static constexpr size_t LEN = sizeof(T);
        
        if (width == 0) return 0;
        if (width > LEN * 8) {
            // 宽度超过类型大小，截断
            return pull_int<T>() & ((static_cast<T>(1) << width) - 1);
        }
        
        std::array<uint8_t, LEN> buffer{};
        pull_bits(buffer.data(), width);
        
        T result = std::bit_cast<T>(buffer);
        
        // 清除高位未使用的位
        if (width < LEN * 8) {
            result &= (static_cast<T>(1) << width) - 1;
        }
        
        return result;
    }
    
    // 读取并返回 bitset
    template<size_t N>
    constexpr std::bitset<N> pull_bitset() {
        static_assert(N > 0, "Bitset size must be positive");
        
        constexpr size_t byte_size = (N + 7) / 8;
        std::array<uint8_t, byte_size> buffer{};
        pull_bits(buffer.data(), N);
        
        std::bitset<N> result;
        for (size_t i = 0; i < N; ++i) {
            size_t byte_idx = i / 8;
            size_t bit_idx = i % 8;
            if (buffer[byte_idx] & (1 << bit_idx)) {
                result.set(i);
            }
        }
        
        return result;
    }
    
    // 读取单个位
    [[nodiscard]] constexpr bool pull_bit() {
        size_t byte_idx = bit_offset >> 3;
        size_t bit_idx = bit_offset & 7;
        bool result = (ptr[byte_idx] >> bit_idx) & 1;
        bit_offset += 1;
        return result;
    }

    
    // 跳过指定位数
    constexpr void skip_bits(const size_t bits) {
        bit_offset += bits;
    }
    
    // 回退指定位数
    constexpr void rewind_bits(const size_t bits) {
        if (bits <= bit_offset) {
            bit_offset -= bits;
        }
    }
};


// 编译期生成掩码表
static constexpr auto generate_masks = []() {
	struct MaskTable {
		uint8_t low_masks[9];  // low_masks: 0-8位
		uint8_t shift_masks[8][9];  // shift_masks[byte_offset][bits]
		
		constexpr MaskTable() : low_masks{}, shift_masks{} {
			// 生成低位掩码表 low_masks[n] = (1 << n) - 1
			for (int i = 0; i <= 8; ++i) {
				low_masks[i] = (i == 0) ? 0 : ((1 << i) - 1);
			}
			
			// 生成移位掩码表 shift_masks[offset][bits]
			for (int offset = 0; offset < 8; ++offset) {
				for (int bits = 0; bits <= 8; ++bits) {
					if (bits == 0) {
						shift_masks[offset][bits] = 0;
					} else if (offset + bits <= 8) {
						shift_masks[offset][bits] = low_masks[bits] << offset;
					} else {
						shift_masks[offset][bits] = 0xFF; // 无效组合
					}
				}
			}
		}
	};
	return MaskTable{};
}();




static constexpr void bit_cursor_store_bits(
	__restrict__ uint8_t * buffer,
	const size_t bit_offset,
	__restrict__ const uint8_t * data,
	const size_t data_width
) {
    if (data_width == 0) return;
    

    if ((bit_offset & 7) == 0 && (data_width & 7) == 0) {
        uint8_t * dst = buffer + (bit_offset >> 3);
        for(size_t i = 0; i < (data_width >> 3); i++){
            dst[i] = data[i];
        }
        return;
    }
    

    auto& low_masks = generate_masks.low_masks;
    auto& shift_masks = generate_masks.shift_masks;

    
    size_t bit_pos = bit_offset;
    size_t data_bit_pos = 0;
    size_t remaining = data_width;
    
    while (remaining > 0) {
        // 使用位运算代替除法和取模
        size_t byte_idx = bit_pos >> 3;           // bit_pos / 8
        size_t byte_bit_off = bit_pos & 7;        // bit_pos % 8
        size_t bits_this_byte = std::min<size_t>(8 - byte_bit_off, remaining);
        
        size_t data_byte_idx = data_bit_pos >> 3; // data_bit_pos / 8
        size_t data_bit_off = data_bit_pos & 7;   // data_bit_pos % 8
        
        uint8_t value;
        if (data_bit_off + bits_this_byte <= 8) {
            // 源数据在一个字节内
            value = (data[data_byte_idx] >> data_bit_off) & low_masks[bits_this_byte];
        } else {
            // 源数据跨两个字节
            size_t bits_from_first = 8 - data_bit_off;
            size_t bits_from_second = bits_this_byte - bits_from_first;
            
            uint8_t first_part = (data[data_byte_idx] >> data_bit_off) & low_masks[bits_from_first];
            uint8_t second_part = data[data_byte_idx + 1] & low_masks[bits_from_second];
            value = first_part | (second_part << bits_from_first);
        }
        
        // 使用预计算的掩码表
        uint8_t mask = shift_masks[byte_bit_off][bits_this_byte];
        buffer[byte_idx] = (buffer[byte_idx] & ~mask) | (value << byte_bit_off);
        
        bit_pos += bits_this_byte;
        data_bit_pos += bits_this_byte;
        remaining -= bits_this_byte;
    }
}



static constexpr void bit_cursor_load_bits(
    __restrict__ const uint8_t * buffer,
    const size_t bit_offset,
    __restrict__ uint8_t * data,
    const size_t data_width
) {
    if (data_width == 0) return;
    
    const auto& low_masks = generate_masks.low_masks;
    const auto& shift_masks = generate_masks.shift_masks;
    
    size_t bit_pos = bit_offset;
    size_t data_bit_pos = 0;
    size_t remaining = data_width;
    
    while (remaining > 0) {
        // 计算源buffer的字节位置和位偏移
        size_t buffer_byte_idx = bit_pos >> 3;      // bit_pos / 8
        size_t buffer_bit_off = bit_pos & 7;        // bit_pos % 8
        
        // 本次读取的位数（不超过剩余位数，不超过当前字节剩余位）
        size_t bits_this_byte = std::min<size_t>(8 - buffer_bit_off, remaining);
        
        // 计算目标data的字节位置和位偏移
        size_t data_byte_idx = data_bit_pos >> 3;   // data_bit_pos / 8
        size_t data_bit_off = data_bit_pos & 7;     // data_bit_pos % 8
        
        // 从buffer中读取值
        uint8_t value;
        if (bits_this_byte == 8 && buffer_bit_off == 0) {
            // 完整字节读取，直接拷贝
            value = buffer[buffer_byte_idx];
        } else {
            // 从当前字节提取指定位
            value = (buffer[buffer_byte_idx] >> buffer_bit_off) & low_masks[bits_this_byte];
        }
        
        // 将value写入data（可能需要跨字节）
        if (data_bit_off == 0 && bits_this_byte == 8) {
            // 完整字节写入，直接拷贝
            data[data_byte_idx] = value;
        } else if (data_bit_off + bits_this_byte <= 8) {
            // 目标在一个字节内
            uint8_t mask = shift_masks[data_bit_off][bits_this_byte];
            data[data_byte_idx] = (data[data_byte_idx] & ~mask) | (value << data_bit_off);
        } else {
            // 目标跨两个字节
            size_t bits_to_first = 8 - data_bit_off;
            size_t bits_to_second = bits_this_byte - bits_to_first;
            
            // 写入第一个字节
            uint8_t first_part = value & low_masks[bits_to_first];
            uint8_t mask1 = shift_masks[data_bit_off][bits_to_first];
            data[data_byte_idx] = (data[data_byte_idx] & ~mask1) | (first_part << data_bit_off);
            
            // 写入第二个字节
            uint8_t second_part = (value >> bits_to_first) & low_masks[bits_to_second];
            data[data_byte_idx + 1] = (data[data_byte_idx + 1] & ~low_masks[bits_to_second]) | second_part;
        }
        
        // 更新位置
        bit_pos += bits_this_byte;
        data_bit_pos += bits_this_byte;
        remaining -= bits_this_byte;
    }
}

}