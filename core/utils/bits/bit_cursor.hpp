#pragma once

#include <cstdint>
#include <cstddef>
#include <bit>
#include <array>
#include <bitset>

namespace ymd{


static constexpr void bit_cursor_store_bits(
    __restrict__ uint8_t * ptr,
    const size_t offset_bits,
    __restrict__ const uint8_t * data,
    const size_t data_width
);


struct [[nodiscard]] BitCursor final {
    uint8_t* ptr;
    size_t offset_bits;

    [[nodiscard]] constexpr bool is_aligned_to_byte() const {
        return (offset_bits & 0b111) == 0;
    }

    [[nodiscard]] /* constexpr */ bool is_aligned_to_word() const {
        if((offset_bits & 0b111) != 0) return false;
        const uint8_t * end = ptr + (offset_bits >> 3);
        const uintptr_t byte_addr = reinterpret_cast<uintptr_t>(end);
        return (byte_addr & (sizeof(size_t) - 1)) == 0;
    }
    
    constexpr void push_bits(const uint8_t* data, const size_t data_width) {
        auto& self = *this;
        bit_cursor_store_bits(self.ptr, self.offset_bits, data, data_width);
        self.offset_bits += data_width;
    }

    template<typename T>
    constexpr void push_int(const T int_val) {
        static constexpr size_t LEN = sizeof(T);
        const auto src = std::bit_cast<std::array<uint8_t, LEN>>(int_val);
        push_bits(src.data(), LEN * 8);
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
	const size_t offset_bits,
	__restrict__ const uint8_t * data,
	const size_t data_width
) {
    if (data_width == 0) return;
    

    if ((offset_bits & 7) == 0 && (data_width & 7) == 0) {
        uint8_t * dst = buffer + (offset_bits >> 3);
        for(size_t i = 0; i < (data_width >> 3); i++){
            dst[i] = data[i];
        }
        return;
    }
    

    auto& low_masks = generate_masks.low_masks;
    auto& shift_masks = generate_masks.shift_masks;

    
    size_t bit_pos = offset_bits;
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

}