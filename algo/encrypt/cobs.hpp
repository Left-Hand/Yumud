#pragma once

#include <stddef.h>
#include <stdint.h>

namespace ymd::cobs{


/** COBS encode data to codec_buffer
 * @param data Pointer to input data to encode
 * @param length Number of bytes to encode
 * @param codec_buffer Pointer to encoded output codec_buffer
 * @return Encoded codec_buffer length in bytes
 * @note Does not output delimiter byte
 */
[[nodiscard]] constexpr size_t cobs_encode(
    __restrict uint8_t *codec_buffer, 
    __restrict const uint8_t *data, 
    size_t length
){
    uint8_t *encode = codec_buffer; // Encoded byte pointer
    uint8_t *codep = encode++; // Output code pointer
    uint8_t code = 1; // Code value

    for (const uint8_t *byte = reinterpret_cast<const uint8_t *>(data); length--; ++byte)
    {
        if (*byte) // Byte not zero, write it
            *encode++ = *byte, ++code;

        if (!*byte || code == 0xff) // Input is zero or block completed, restart
        {
            *codep = code, code = 1, codep = encode;
            if (!*byte || length)
                ++encode;
        }
    }
    *codep = code; // Write final code value

    return (size_t)(encode - codec_buffer);
}

/** COBS decode data from codec_buffer
 * @param codec_buffer Pointer to encoded input bytes
 * @param length Number of bytes to decode
 * @param data Pointer to decoded output data
 * @return Number of bytes successfully decoded
 * @note Stops decoding if delimiter byte is found
 */
[[nodiscard]] constexpr size_t cobs_decode(
    __restrict const uint8_t *codec_buffer, 
    __restrict uint8_t *data, 
    size_t length
){
    if(codec_buffer == nullptr) [[unlikely]]
        __builtin_trap();
        
    if(codec_buffer == nullptr) [[unlikely]]
        __builtin_trap();

    const uint8_t *byte = codec_buffer; // Encoded input byte pointer
    uint8_t *decode = data; // Decoded output byte pointer

    for (uint8_t code = 0xff, block = 0; 
        byte < reinterpret_cast<const uint8_t *>(codec_buffer + length); 
        --block
    ){
        if (block) // Decode block byte
            *decode++ = *byte++;
        else{
            block = *byte++; // Fetch the next block length
            if (block && (code != 0xff)) // Encoded zero, write it unless it's delimiter.
                *decode++ = 0;
            code = block;
            if (!code) // Delimiter code found
                break;
        }
    }

    return static_cast<size_t>(decode - data);
}
}