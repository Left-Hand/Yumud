#pragma once

#include "core/utils/Result.hpp"

namespace ymd::encrypt::lz77{
    
// LZ77压缩参数
static constexpr size_t WINDOW_SIZE = 4096;     // 滑动窗口大小
static constexpr size_t LOOKAHEAD_SIZE = 18;    // 前向缓冲区大小
static constexpr size_t MIN_MATCH_LENGTH = 3;   // 最小匹配长度
static constexpr size_t MAX_MATCH_LENGTH = 258; // 最大匹配长度

// LZ77令牌格式
// 使用一种简单的编码方式:
// 字面量: 0x00 + 1字节数据 (2字节)
// 匹配:   0x01 + 长度(1字节) + 距离(2字节) (4字节)
struct LZ77Token {
    enum Type : uint8_t {
        LITERAL = 0x00,
        MATCH = 0x01
    };
    
    Type type;
    union {
        struct {
            uint8_t literal;
        } literal_data;
        
        struct {
            uint8_t length;    // 实际长度-3存储，范围3-258
            uint16_t distance; // 距离
        } match_data;
    };
};

enum class LZ77Error:uint8_t{
    OutOfMemory,
    InvalidToken,
    InvalidData,
    InvalidDistance,
    Unreachable
};

// 压缩函数
// 返回实际写入dst的字节数，如果dst空间不足则返回std::nullopt
[[nodiscard]] constexpr Result<size_t, LZ77Error> compress(
    std::span<uint8_t> dst, 
    std::span<const uint8_t> src) {
    
    size_t dst_pos = 0;
    size_t src_pos = 0;
    const size_t src_size = src.size();
    
    while (src_pos < src_size && dst_pos < dst.size()) {
        size_t best_length = 0;
        size_t best_distance = 0;
        
        // 搜索窗口起始位置
        const size_t window_start = (src_pos > WINDOW_SIZE) ? src_pos - WINDOW_SIZE : 0;
        
        // 在滑动窗口中寻找最长匹配
        for (size_t i = window_start; i < src_pos; ++i) {
            size_t length = 0;
            const size_t max_length = std::min({
                static_cast<size_t>(MAX_MATCH_LENGTH),
                src_size - src_pos,
                LOOKAHEAD_SIZE
            });
            
            // 计算从位置i开始的匹配长度
            while (length < max_length && src[i + length] == src[src_pos + length]) {
                length++;
            }
            
            // 如果找到更长的有效匹配
            if (length >= MIN_MATCH_LENGTH && length > best_length) {
                best_length = length;
                best_distance = src_pos - i;
            }
        }
        
        // 检查是否有足够空间写入令牌
        if (best_length >= MIN_MATCH_LENGTH) {
            // 匹配令牌需要4字节
            if (dst_pos + 4 > dst.size()) {
                return Err(LZ77Error::OutOfMemory); // 空间不足
            }
            
            // 写入匹配令牌
            dst[dst_pos++] = LZ77Token::MATCH;
            dst[dst_pos++] = static_cast<uint8_t>(best_length - MIN_MATCH_LENGTH);
            dst[dst_pos++] = static_cast<uint8_t>(best_distance & 0xFF);
            dst[dst_pos++] = static_cast<uint8_t>((best_distance >> 8) & 0xFF);
            
            src_pos += best_length;
        } else {
            // 字面量令牌需要2字节
            if (dst_pos + 2 > dst.size()) {
                return Err(LZ77Error::OutOfMemory); // 空间不足
            }
            
            // 写入字面量令牌
            dst[dst_pos++] = LZ77Token::LITERAL;
            dst[dst_pos++] = src[src_pos];
            src_pos++;
        }
    }
    
    return Ok(dst_pos);
}

// 解压缩函数
// 返回实际写入dst的字节数，如果dst空间不足或数据格式错误则返回std::nullopt
[[nodiscard]] constexpr Result<size_t, LZ77Error> decompress(
    std::span<uint8_t> dst, 
    std::span<const uint8_t> src) {
    
    size_t dst_pos = 0;
    size_t src_pos = 0;
    
    while (src_pos < src.size() && dst_pos < dst.size()) {
        if (src_pos >= src.size()) {
            return Err(LZ77Error::InvalidData); // 数据不完整
        }
        
        const uint8_t token_type = src[src_pos++];
        
        switch (token_type) {
            case LZ77Token::LITERAL: {
                if (src_pos >= src.size() || dst_pos >= dst.size()) {
                    return Err(LZ77Error::InvalidData); // 数据不完整或空间不足
                }
                
                dst[dst_pos++] = src[src_pos++];
                break;
            }
            
            case LZ77Token::MATCH: {
                if (src_pos + 2 >= src.size()) {
                    return Err(LZ77Error::InvalidData); // 数据不完整
                }
                
                const uint8_t length_code = src[src_pos++];
                const uint16_t distance = static_cast<uint16_t>(src[src_pos] | (src[src_pos + 1] << 8));
                src_pos += 2;
                
                const size_t length = static_cast<size_t>(length_code) + MIN_MATCH_LENGTH;
                
                // 检查距离有效性
                if (distance > dst_pos || distance == 0) {
                    return Err(LZ77Error::InvalidDistance); // 无效距离
                }
                
                // 检查是否有足够空间
                if (dst_pos + length > dst.size()) {
                    return Err(LZ77Error::OutOfMemory); // 空间不足
                }
                
                // 复制匹配数据（处理重叠情况）
                const size_t source_pos = dst_pos - distance;
                for (size_t i = 0; i < length; ++i) {
                    if (source_pos + i < dst.size() && dst_pos < dst.size()) {
                        dst[dst_pos] = dst[source_pos + i];
                        dst_pos++;
                    } else {
                        return Err(LZ77Error::InvalidData); // 错误
                    }
                }
                break;
            }
            default:
                return Err(LZ77Error::Unreachable);
        }
    }
    
    return Ok(dst_pos);
}

}