#pragma once

#include "core/utils/Result.hpp"

namespace ymd::encrypt::lz77{
    
// LZ77 compression parameters
static constexpr size_t WINDOW_SIZE = 4096;     // Sliding window size
static constexpr size_t LOOKAHEAD_SIZE = 18;    // Lookahead buffer size
static constexpr size_t MIN_MATCH_LENGTH = 3;   // Minimum match length
static constexpr size_t MAX_MATCH_LENGTH = 258; // Maximum match length

// LZ77 token format
// Using a simple encoding method:
// Literal: 0x00 + 1 byte data (2 bytes)
// Match:   0x01 + length(1 byte) + distance(2 bytes) (4 bytes)
struct Token {
    enum Type : uint8_t {
        Literal = 0x00,
        Match = 0x01
    };
    
    Type type;
    union {
        struct {
            uint8_t literal;
        } literal_data;
        
        struct {
            uint8_t length;    // Actual length-3 stored, range 3-258
            uint16_t distance; // Distance
        } match_data;
    };
};

enum class Error:uint8_t{
    OutOfMemory,
    InvalidToken,
    InvalidData,
    InvalidDistance,
    Unreachable
};

// Compression function
// Returns the number of bytes actually written to dst, returns std::nullopt if dst space is insufficient
[[nodiscard]] constexpr Result<size_t, Error> compress(
    std::span<uint8_t> dst, 
    std::span<const uint8_t> src
) {
    
    size_t dst_pos = 0;
    size_t src_pos = 0;
    const size_t src_size = src.size();
    
    while (src_pos < src_size && dst_pos < dst.size()) {
        size_t best_length = 0;
        size_t best_distance = 0;
        
        // Search window start position
        const size_t window_start = (src_pos > WINDOW_SIZE) ? src_pos - WINDOW_SIZE : 0;
        
        // Find longest match in sliding window
        for (size_t i = window_start; i < src_pos; ++i) {
            size_t length = 0;
            const size_t max_length = std::min({
                static_cast<size_t>(MAX_MATCH_LENGTH),
                src_size - src_pos,
                LOOKAHEAD_SIZE
            });
            
            // Calculate matching length from position i
            while (length < max_length && src[i + length] == src[src_pos + length]) {
                length++;
            }
            
            // If we found a longer valid match
            if (length >= MIN_MATCH_LENGTH && length > best_length) {
                best_length = length;
                best_distance = src_pos - i;
            }
        }
        
        // Check if there's enough space to write token
        if (best_length >= MIN_MATCH_LENGTH) {
            // Match token requires 4 bytes
            if (dst_pos + 4 > dst.size()) {
                return Err(Error::OutOfMemory); // Insufficient space
            }
            
            // Write match token
            dst[dst_pos++] = Token::Match;
            dst[dst_pos++] = static_cast<uint8_t>(best_length - MIN_MATCH_LENGTH);
            dst[dst_pos++] = static_cast<uint8_t>(best_distance & 0xFF);
            dst[dst_pos++] = static_cast<uint8_t>((best_distance >> 8) & 0xFF);
            
            src_pos += best_length;
        } else {
            // Literal token requires 2 bytes
            if (dst_pos + 2 > dst.size()) {
                return Err(Error::OutOfMemory); // Insufficient space
            }
            
            // Write literal token
            dst[dst_pos++] = Token::Literal;
            dst[dst_pos++] = src[src_pos];
            src_pos++;
        }
    }
    
    return Ok(dst_pos);
}

// Decompression function
// Returns the number of bytes actually written to dst, returns std::nullopt if dst space is insufficient or data format error
[[nodiscard]] constexpr Result<size_t, Error> decompress(
    std::span<uint8_t> dst, 
    std::span<const uint8_t> src
) {
    
    size_t dst_pos = 0;
    size_t src_pos = 0;
    
    while (src_pos < src.size() && dst_pos < dst.size()) {
        // This check is redundant as it's already checked in the while condition
        // if (src_pos >= src.size()) {
        //     return Err(Error::InvalidData); // Incomplete data
        // }
        
        const uint8_t token_type = src[src_pos++];
        
        switch (token_type) {
            case Token::Literal: {
                if (src_pos >= src.size() || dst_pos >= dst.size()) {
                    return Err(Error::InvalidData); // Incomplete data or insufficient space
                }
                
                dst[dst_pos++] = src[src_pos++];
                break;
            }
            
            case Token::Match: {
                if (src_pos + 2 >= src.size()) {
                    return Err(Error::InvalidData); // Incomplete data
                }
                
                const uint8_t length_code = src[src_pos++];
                const uint16_t distance = static_cast<uint16_t>(src[src_pos] | (src[src_pos + 1] << 8));
                src_pos += 2;
                
                const size_t length = static_cast<size_t>(length_code) + MIN_MATCH_LENGTH;
                
                // Check distance validity
                if (distance > dst_pos || distance == 0) {
                    return Err(Error::InvalidDistance); // Invalid distance
                }
                
                // Check if there's enough space
                if (dst_pos + length > dst.size()) {
                    return Err(Error::OutOfMemory); // Insufficient space
                }
                
                // Copy matched data (handle overlapping cases)
                const size_t source_pos = dst_pos - distance;
                for (size_t i = 0; i < length; ++i) {
                    // More precise boundary checking
                    if (source_pos + i < dst.size() && dst_pos < dst.size()) {
                        dst[dst_pos] = dst[source_pos + i];
                        dst_pos++;
                    } else {
                        return Err(Error::InvalidData); // Error
                    }
                }
                break;
            }
            default:
                // Return InvalidToken instead of Unreachable for unknown token types
                return Err(Error::InvalidToken);
        }
    }
    
    return Ok(dst_pos);
}

}