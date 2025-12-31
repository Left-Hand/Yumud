#pragma once

#include "myactuator_primitive.hpp"


namespace ymd::robots::myactuator { 

static constexpr size_t PAYLOAD_CAPACITY = 7;

struct [[nodiscard]] BytesFiller{
public:
    static constexpr size_t CAPACITY = PAYLOAD_CAPACITY;

    constexpr explicit BytesFiller(std::span<uint8_t, CAPACITY> bytes):
        bytes_(bytes){;}

    constexpr ~BytesFiller(){
        if(not is_full()) __builtin_abort();
    }

    constexpr __always_inline 
    void push_byte(const uint8_t byte){
        if(pos_ >= bytes_.size()) [[unlikely]] 
            on_overflow();
        bytes_[pos_++] = byte;
    }

    constexpr __always_inline 
    void push_zero(){
        push_byte(0);
    }

    constexpr __always_inline 
    void push_zeros(size_t n){
        #pragma GCC unroll(4)
        for(size_t i = 0; i < n; i++)
            push_byte(0);
    }

    constexpr __always_inline 
    void fill_remaining(const uint8_t byte){
        const size_t n = bytes_.size() - pos_;

        #pragma GCC unroll(4)
        for(size_t i = 0; i < n; i++){
            push_byte_unchecked(byte);
        }
    }

    template<size_t Extents>
    constexpr __always_inline 
    void push_bytes(const std::span<const uint8_t, Extents> bytes){
        if(pos_ + bytes.size() > bytes_.size()) [[unlikely]]
            on_overflow();
        push_bytes_unchecked(bytes);
    }

    constexpr __always_inline 
    void push_float(const math::fp32 f_val){
        static_assert(sizeof(float) == 4);
        const auto bytes = std::bit_cast<std::array<uint8_t, sizeof(float)>>(f_val);
        push_bytes(std::span(bytes));
    }

    template<typename T>
    requires (std::is_integral_v<T>)
    constexpr __always_inline 
    void push_int(const T i_val){
        const auto bytes = std::bit_cast<std::array<uint8_t, sizeof(T)>>(i_val);
        push_bytes(std::span(bytes));
    }


    [[nodiscard]] constexpr bool is_full() const {
        return pos_ == CAPACITY;
    }
private:
    std::span<uint8_t, CAPACITY> bytes_;
    size_t pos_ = 0;

    constexpr __always_inline 
    void push_byte_unchecked(const uint8_t byte){ 
        bytes_[pos_++] = byte;
    }

    template<size_t Extents>
    constexpr __always_inline 
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

    constexpr __always_inline void on_overflow(){
        __builtin_trap();
    }
};



struct [[nodiscard]] CommandHeadedDataField{
    ReqCommand cmd;
    std::array<uint8_t, PAYLOAD_CAPACITY> payload_bytes;

    static constexpr CommandHeadedDataField from_command_and_payload_bytes(
        const ReqCommand cmd,
        std::span<const uint8_t, BytesFiller::CAPACITY> payload_bytes
    ){
        CommandHeadedDataField ret;
        ret.cmd = cmd;
        std::copy(payload_bytes.begin(), payload_bytes.end(), ret.payload_bytes.begin());
        return ret;
    }

    static constexpr CommandHeadedDataField from_bytes(const std::span<uint8_t, BytesFiller::CAPACITY + 1> bytes){
        return from_command_and_payload_bytes(
            static_cast<ReqCommand>(bytes[0]),
            bytes.subspan<1, BytesFiller::CAPACITY>()
        );
    }
};

static_assert(sizeof(CommandHeadedDataField) == 1 + BytesFiller::CAPACITY);



}
