#pragma once

#include "core/utils/Result.hpp"


namespace ymd::cobs{

#if 1


struct [[nodiscard]] CobsEncoder final {
    static constexpr size_t MAX_BLOCK_SIZE = 254; // COBS 最大块大小
    static constexpr size_t BLOCK_BUFFER_SIZE = MAX_BLOCK_SIZE + 1;

    using BufferStorage = std::array<uint8_t, BLOCK_BUFFER_SIZE>;
    
    static constexpr std::array<uint8_t, 1> EMPTY_BYTE = {0};

    template<typename Receiver>
    requires requires(Receiver receiver) {
        { receiver.push_bytes(std::span<const uint8_t>()) } 
            -> std::same_as<Result<void, typename Receiver::Error>>;
    }
    constexpr Result<void, typename Receiver::Error> encode(
        Receiver& receiver,
        std::span<const uint8_t> data_to_encode
    ) {
        BufferStorage block_buffer; // 每个块的本地缓冲区
        size_t block_pos = 1; // 从 1 开始，位置 0 留给代码字节
        
        // 处理空输入的特殊情况
        if(data_to_encode.empty()) {
            // COBS 编码空数据为 0x01
            block_buffer[0] = 0x01;
            return receiver.push_bytes(std::span(block_buffer.data(), 1));
        }
        
        auto flush_block = [&](bool is_final_block) -> Result<void, typename Receiver::Error> {
            if(block_pos > 1 || is_final_block) {
                // 设置代码字节
                // 注意：代码字节等于块中数据字节数 + 1
                block_buffer[0] = static_cast<uint8_t>(block_pos);
                
                // 输出当前块
                if(auto res = receiver.push_bytes(std::span(block_buffer.data(), block_pos));
                    res.is_err()) return Err(res.unwrap_err());

                
                // 如果不是最后一个块，为新块预留代码字节位置
                if(!is_final_block) {
                    block_pos = 1; // 重置为新块，位置 0 将存放新的代码字节
                }
            }
            return Ok();
        };
        
        // 处理每个输入字节
        for(size_t i = 0; i < data_to_encode.size(); ++i) {
            uint8_t byte = data_to_encode[i];
            
            if(byte != 0) {
                // 非零字节，添加到当前块
                if(block_pos > MAX_BLOCK_SIZE) {
                    // 块已满（达到 254 个数据字节），先刷新当前块
                    if(auto res = flush_block(false); res.is_err()) {
                        return Err(res.unwrap_err());
                    }
                }
                
                block_buffer[block_pos++] = byte;
            }
            
            // 遇到零字节或达到块末尾，需要结束当前块
            if(byte == 0 || block_pos > MAX_BLOCK_SIZE) {
                if(auto res = flush_block(false); res.is_err()) {
                    return Err(res.unwrap_err());
                }
                
                // 开始新块（如果有更多数据）
                if(i + 1 < data_to_encode.size()) {
                    // 新块已经通过 flush_block 的 block_pos = 1 开始了
                    continue;
                }
            }
        }
        
        // 刷新最后一个块
        return flush_block(true);
    }
};

#else
/** COBS encode data to codec_buffer
 * @param data Pointer to input data to encode
 * @param length Number of bytes to encode
 * @param codec_buffer Pointer to encoded output codec_buffer
 * @return Encoded codec_buffer length in bytes
 * @note Does not output delimiter byte
 */
[[nodiscard]] constexpr size_t encode(
    __restrict uint8_t *codec_buffer, 
    __restrict const uint8_t *data, 
    size_t length
){
    uint8_t *encode = codec_buffer; // Encoded byte pointer
    uint8_t *codep = encode++; // Output code pointer
    uint8_t code = 1; // Code value

    for (const uint8_t *byte = static_cast<const uint8_t *>(data); length--; ++byte)
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
#endif


template<typename Receiver>
requires requires(Receiver receiver, std::span<const uint8_t> span) {
    { receiver.push_bytes(span) } -> std::same_as<Result<void, typename Receiver::Error>>;
}
[[nodiscard]] constexpr Result<void, typename Receiver::Error> cobs_decode(
    Receiver & receiver,
    std::span<const uint8_t> encoded_bytes
) noexcept {
    // 检查空输入
    if(encoded_bytes.empty()) [[unlikely]]
        return Ok();
    
    const uint8_t* byte = encoded_bytes.data();
    const uint8_t* const end = encoded_bytes.data() + encoded_bytes.size();
    
    // 检查是否有至少一个字节
    if(byte >= end) [[unlikely]]
        return Ok();
    
    // 读取第一个代码字节
    uint8_t code = *byte++;
    uint8_t remaining_in_block = code - 1;
    
    while(byte < end) {
        // 处理当前块中的数据字节
        for(uint8_t i = 0; i < remaining_in_block && byte < end; ++i) {
            uint8_t data_byte = *byte++;
            
            // 在COBS编码中，数据字节不应为0（除非是块结尾）
            if(data_byte == 0) [[unlikely]] {
                return Ok();
            }
            
            // 创建单字节span并发送
            std::span<const uint8_t> single_byte_span{&data_byte, 1};
            if(const auto res = receiver.push_bytes(single_byte_span);
                res.is_err()) return Err(res.unwrap_err());
        }
        
        // 如果到达输入末尾，结束解码
        if(byte >= end) break;
        
        // 遇到代码字节0表示结束（虽然这在标准COBS编码中不应该出现）
        if(*byte == 0) {
            byte++; // 跳过结束符
            break;
        }
        
        // 读取下一个代码字节
        code = *byte++;
        remaining_in_block = code - 1;
    }
    
    return Ok();
}
}