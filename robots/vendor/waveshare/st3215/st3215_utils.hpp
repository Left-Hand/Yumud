#pragma once

#include <span>
#include <cstdint>

namespace ymd::robots::waveshare::st3215{

struct [[nodiscard]] ChecksumBuilder final{
    using Self = ChecksumBuilder;

    static constexpr Self from_default(){
        Self self;
        self.checksum = 0;
        return self;
    }
    
    constexpr Self push_bytes(std::span<const uint8_t> bytes) const noexcept {
        Self self = *this;

        #pragma GCC unroll 4
        for(size_t i = 0; i < bytes.size(); i++) {
            self = self.push_byte(bytes[i]);
        }

        return self;
    }


    __attribute__((always_inline))
    constexpr Self push_byte(const uint8_t byte) const noexcept {
        Self self = *this;
        self.checksum = static_cast<uint8_t>(self.checksum + byte);

        return self;
    }

    [[nodiscard]] constexpr uint16_t finalize() const noexcept {
        return ~checksum;
    }

private:
    uint8_t checksum;
};


[[nodiscard]] static constexpr uint8_t calc_checksum(const uint8_t id, std::span<const uint8_t> pbuf){
    const size_t len = pbuf.size() + 1;
    // const uint32_t sum = id.count + len + std::accumulate(pbuf.begin(), pbuf.end(), 0);

    return ChecksumBuilder::from_default()
        .push_byte(id)
        .push_byte(static_cast<uint8_t>(len))
        .push_bytes(pbuf)
        .finalize();
}


struct [[nodiscard]] BytesFiller{
public:

    constexpr explicit BytesFiller(std::span<uint8_t> bytes):
        bytes_(bytes){;}

    constexpr ~BytesFiller(){
        if(not is_full()) __builtin_abort();
    }

    constexpr  
    void push_byte(const uint8_t byte){
        if(pos_ >= bytes_.size()) [[unlikely]] 
            on_overflow();
        bytes_[pos_++] = byte;
    }

    constexpr  
    void push_zero(){
        push_byte(0);
    }

    constexpr  
    void push_zeros(size_t n){
        #pragma GCC unroll(4)
        for(size_t i = 0; i < n; i++)
            push_byte(0);
    }

    constexpr  
    void fill_remaining(const uint8_t byte){
        const size_t n = bytes_.size() - pos_;

        #pragma GCC unroll(4)
        for(size_t i = 0; i < n; i++){
            push_byte_unchecked(byte);
        }
    }

    template<size_t Extents>
    constexpr  
    void push_bytes(const std::span<const uint8_t, Extents> bytes){
        if(pos_ + bytes.size() > bytes_.size()) [[unlikely]]
            on_overflow();
        push_bytes_unchecked(bytes);
    }


    template<typename T>
    requires (std::is_integral_v<T>)
    constexpr  
    void push_int(const T i_val){
        const auto bytes = std::bit_cast<std::array<uint8_t, sizeof(T)>>(i_val);
        push_bytes(std::span(bytes));
    }


    [[nodiscard]] constexpr bool is_full() const noexcept {
        return pos_ == bytes_.size();
    }
private:
    std::span<uint8_t> bytes_;
    size_t pos_ = 0;

    constexpr  
    void push_byte_unchecked(const uint8_t byte){ 
        bytes_[pos_++] = byte;
    }

    template<size_t Extents>
    constexpr  
    void push_bytes_unchecked(const std::span<const uint8_t, Extents> bytes){ 
        if constexpr(Extents == std::dynamic_extent){
            #pragma GCC unroll(4)
            for(size_t i = 0; i < bytes.size(); i++){
                push_byte(bytes[i]);
            }
        }else{
            #pragma GCC unroll(4)
            for(size_t i = 0; i < Extents; i++){
                push_byte(bytes[i]);
            }
        }
    }

    constexpr  void on_overflow(){
        __builtin_trap();
    }
};

}