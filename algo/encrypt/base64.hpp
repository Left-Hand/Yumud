#pragma once

#include <cstdint>
#include <span>
#include "core/utils/Result.hpp"

// https://github.com/matrixcascade/PainterEngine/blob/master/core/PX_Base64.c

namespace ymd::encrypt::base64{
namespace details{

static constexpr uint8_t BASE64_PAD = '=';

/* BASE 64 encode table */
static constexpr std::array<uint8_t, 64> BASE64EN = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/',
};

static constexpr std::array<uint8_t, 128> BASE64DE = {
    /* nul, soh, stx, etx, eot, enq, ack, bel, */
    255, 255, 255, 255, 255, 255, 255, 255,
    /*  bs,  ht,  nl,  vt,  np,  cr,  so,  si, */
    255, 255, 255, 255, 255, 255, 255, 255,
    /* dle, dc1, dc2, dc3, dc4, nak, syn, etb, */
    255, 255, 255, 255, 255, 255, 255, 255,
    /* can,  em, sub, esc,  fs,  gs,  rs,  us, */
    255, 255, 255, 255, 255, 255, 255, 255,
    /*  sp, '!', '"', '#', '$', '%', '&', ''', */
    255, 255, 255, 255, 255, 255, 255, 255,
    /* '(', ')', '*', '+', ',', '-', '.', '/', */
    255, 255, 255,  62, 255, 255, 255,  63,
    /* '0', '1', '2', '3', '4', '5', '6', '7', */
    52,  53,  54,  55,  56,  57,  58,  59,
    /* '8', '9', ':', ';', '<', '=', '>', '?', */
    60,  61, 255, 255, 255, 255, 255, 255,
    /* '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', */
    255,   0,   1,  2,   3,   4,   5,    6,
    /* 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', */
    7,   8,   9,  10,  11,  12,  13,  14,
    /* 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', */
    15,  16,  17,  18,  19,  20,  21,  22,
    /* 'X', 'Y', 'Z', '[', '\', ']', '^', '_', */
    23,  24,  25, 255, 255, 255, 255, 255,
    /* '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', */
    255,  26,  27,  28,  29,  30,  31,  32,
    /* 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', */
    33,  34,  35,  36,  37,  38,  39,  40,
    /* 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', */
    41,  42,  43,  44,  45,  46,  47,  48,
    /* 'x', 'y', 'z', '{', '|', '}', '~', del, */
    49,  50,  51, 255, 255, 255, 255, 255
};

static constexpr uint32_t get_enc_len(uint32_t beforeEncodeLen){
	if(beforeEncodeLen%3) return (beforeEncodeLen/3+1)*4;
	else return (beforeEncodeLen/3)*4;
}

static constexpr uint32_t get_dec_len(uint32_t beforeEncodeLen)
{
	if(beforeEncodeLen%4) return 0;
	return (beforeEncodeLen/4)*3;
}

}


struct Base64Error{
    enum class Kind:uint8_t{
        InvalidLength,
        InvalidCharacter,
        OutputNotEnough,
    };

    using enum Kind;

    constexpr Base64Error(Kind kind):kind_(kind){}

    constexpr bool operator == (const Base64Error & rhs) const{
        return kind() == rhs.kind();
    }

    constexpr bool operator == (const Kind other_kind) const{
        return kind() == other_kind;
    }
    constexpr Kind kind() const {return kind_;}
private:

    Kind kind_;

    friend OutputStream & operator << (OutputStream & os, const Base64Error & self){
        switch(self.kind_){
            case Kind::InvalidLength:   
                return os << "InvalidLength";
            case Kind::InvalidCharacter:    
                return os << "InvalidCharacter";
            case Kind::OutputNotEnough:     
                return os << "OutputNotEnough";
        }
        __builtin_unreachable();
    }
};


Result<size_t, Base64Error> encode(std::span<uint8_t> dst, std::span<const uint8_t > src ){
    // Calculate required output length
    size_t required_len = details::get_enc_len(static_cast<uint32_t>(src.size()));
    
    // Check if destination buffer is large enough
    if (dst.size() < required_len) {
        return Err(Base64Error(Base64Error::Kind::OutputNotEnough));
    }
    
    int32_t s = 0;
    size_t i = 0;
    size_t j = 0;
    uint8_t c;
    uint8_t l = 0;
    
    for (i = 0; i < src.size(); i++) {
        c = src[i];
        
        switch (s) {
        case 0:
            s = 1;
            dst[j++] = details::BASE64EN[(c >> 2) & 0x3F];
            break;
        case 1:
            s = 2;
            dst[j++] = details::BASE64EN[((l & 0x3) << 4) | ((c >> 4) & 0xF)];
            break;
        case 2:
            s = 0;
            dst[j++] = details::BASE64EN[((l & 0xF) << 2) | ((c >> 6) & 0x3)];
            dst[j++] = details::BASE64EN[c & 0x3F];
            break;
        }
        l = c;
    }
    
    switch (s) {
    case 1:
        dst[j++] = details::BASE64EN[(l & 0x3) << 4];
        dst[j++] = details::BASE64_PAD;
        dst[j++] = details::BASE64_PAD;
        break;
    case 2:
        dst[j++] = details::BASE64EN[(l & 0xF) << 2];
        dst[j++] = details::BASE64_PAD;
        break;
    }
    
    // Return the number of bytes written
    return Ok(j);
}
Result<size_t, Base64Error> decode(std::span<uint8_t> dst, std::span<const uint8_t> src) {
    const size_t inlen = src.size();
    size_t j = 0;
    
    // Check if input length is valid
    if (inlen % 4 != 0) {
        return Err(Base64Error(Base64Error::Kind::InvalidLength));
    }
    
    // 提前计算并检查输出缓冲区大小
    // 最大输出长度是 (inlen * 3) / 4，但由于填充，实际可能更小
    // 为了安全，我们提前检查最大可能大小
    const size_t max_output_size = (inlen * 3 + 3) / 4;
    if (dst.size() < max_output_size) {
        return Err(Base64Error(Base64Error::Kind::OutputNotEnough));
    }
    
    uint32_t buffer = 0;
    uint32_t bits_collected = 0;
    
    for (size_t i = 0; i < inlen; i++) {
        const uint8_t current_char = src[i];
        
        if (current_char == details::BASE64_PAD) {
            break;
        }
        
        // Validate character
        if (current_char >= 128) {
            return Err(Base64Error(Base64Error::Kind::InvalidCharacter));
        }
        
        const uint8_t value = details::BASE64DE[current_char];
        if (value == 255) {
            return Err(Base64Error(Base64Error::Kind::InvalidCharacter));
        }
        
        buffer = (buffer << 6) | value;
        bits_collected += 6;
        
        if (bits_collected >= 8) {
            bits_collected -= 8;
            
            // 检查输出缓冲区
            if (j >= dst.size()) {
                return Err(Base64Error(Base64Error::Kind::OutputNotEnough));
            }
            
            dst[j++] = (buffer >> bits_collected) & 0xFF;
        }
    }
    
    return Ok(j);
}


}