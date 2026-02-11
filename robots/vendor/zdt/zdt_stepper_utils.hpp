#pragma once

#include "zdt_stepper_primitive.hpp"

namespace ymd::robots::zdtmotor { 

struct [[nodiscard]] BytesFiller final{
public:
    constexpr explicit BytesFiller(std::span<uint8_t> bytes):
        bytes_(bytes){;}

    constexpr ~BytesFiller(){
    }

    constexpr __inline 
    void push_byte(const uint8_t byte){
        if(idx_ >= bytes_.size()) [[unlikely]] 
            on_overflow();
        bytes_[idx_++] = byte;
    }


    template<size_t Extents>
    constexpr __inline 
    void push_bytes(const std::span<const uint8_t, Extents> bytes){
        if(idx_ + bytes.size() > bytes_.size()) [[unlikely]]
            on_overflow();
        push_bytes_unchecked(bytes);
    }


    constexpr __inline 
    void push_le_u8(const uint16_t int_val) {
        push_byte(int_val);
    }

    constexpr __inline 
    void push_le_u16(const uint16_t int_val) {
        push_byte(int_val & 0xFF);
        push_byte(int_val >> 8);
    }

    constexpr __inline 
    void push_le_u32(const uint32_t int_val){
        push_le_u16(int_val & 0xFFFF);
        push_le_u16(int_val >> 16);
    }

    [[nodiscard]] constexpr bool is_full() const {
        return idx_ == bytes_.size();
    }

    size_t size() const {
        return idx_;
    }
private:
    std::span<uint8_t> bytes_;
    size_t idx_ = 0;

    constexpr __inline 
    void push_byte_unchecked(const uint8_t byte){ 
        bytes_[idx_++] = byte;
    }

    template<size_t Extents>
    constexpr __inline 
    void push_bytes_unchecked(const std::span<const uint8_t, Extents> bytes){ 
        if constexpr(Extents == std::dynamic_extent){
            // #pragma GCC unroll(4)
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

    constexpr __inline void on_overflow(){
        __builtin_trap();
    }
};



struct [[nodiscard]] VerifyUtils final{
    static constexpr uint8_t get_verify_code(
        const VerifyMethod method, 
        const FuncCode func_code,
        std::span<const uint8_t> bytes 
    ){
        switch(method){
            case VerifyMethod::X6B:
                return uint8_t{0x6b};
            case VerifyMethod::XOR:
                return VerifyUtils::by_xor(func_code, bytes);
            case VerifyMethod::CRC8:
                // TODO();
                // __builtin_unreachable();
                return VerifyUtils::by_crc8(func_code, bytes);
        }
        __builtin_unreachable();
    }

private:
    static constexpr uint8_t by_xor(
        const FuncCode func_code,
        const std::span<const uint8_t> bytes
    ){
        uint8_t code = std::bit_cast<uint8_t>(VerifyMethod::XOR);
        code ^= std::bit_cast<uint8_t>(func_code);
        for(size_t i = 0; i < bytes.size(); i++){
            code ^= bytes[i];
        };
        return code;
    }

    static constexpr uint8_t by_crc8(
        const FuncCode func_code,
        const std::span<const uint8_t> bytes
    ){
        uint16_t crc = 0xffff;
        for(size_t i = 0; i < bytes.size(); i++){
            crc ^= uint16_t(bytes[i]) << 8;
            for(uint8_t j = 0; j < 8; j++){
                if(crc & 0x8000) crc ^= 0x1021;
                crc <<= 1;
            }
        }
        return uint8_t(crc >> 8);
    }

};



struct [[nodiscard]] CanFrame2BytesDumper{
    static constexpr Result<FlatPacket, Error> dump(
        std::span<const hal::BxCanFrame> frames
    ) {
        if(frames.size() == 0)
            return Err(Error::RxNoMsgToDump);

        FlatPacket flat_packet;

        flat_packet.node_id = ({
            if(frames[0].is_standard())
                return Err(Error::RxMsgIdTypeNotMatch);

            const auto id = frame_to_nodeid(frames[0]);

            for(size_t i = 1; i < frames.size(); i++){
                if(frames[i].is_standard())
                    return Err(Error::RxMsgIdTypeNotMatch);
                const auto next_id = frame_to_nodeid(frames[i]);
                if(next_id != id) return Err(Error::RxMsgNodeIdNotTheSame);
            }
            NodeId::from_u8(id);
        });

        for(size_t i = 0; i < frames.size(); i++){
            const auto piece_cnt = frame_to_piececnt(frames[i]);
            if(i != piece_cnt)
                return Err(Error::RxMsgPieceIsNotSteady);
        }

        flat_packet.func_code = ({
            if(frames[0].length() == 0) return Err(Error::RxMsgNoPayload);
            const auto func_code0 = std::bit_cast<FuncCode>(frames[0].payload_bytes()[0]);
            for(size_t i = 0; i < frames.size(); i++){
                const auto & frame = frames[i];
                if(frame.length() == 0) return Err(Error::RxMsgNoPayload);
                const auto func_code = std::bit_cast<FuncCode>(frame.payload_bytes()[0]);
                if(func_code != func_code0)
                    return Err(Error::RxMsgFuncCodeNotTheSame);
            }
            func_code0;
        });

        BytesFiller filler(std::span(flat_packet.context));
        for(size_t i = 0; i < frames.size(); i++){
            const auto & frame = frames[i];
            filler.push_bytes(frame.payload_bytes().subspan(1));
        }
        flat_packet.payload_len = (filler.size() - 1);

        //TODO add tail verify

        return Ok<FlatPacket>(flat_packet);
    }

    static inline constexpr uint8_t frame_to_nodeid(
        const hal::BxCanFrame & frame
    ){
        return static_cast<uint8_t>(frame.identifier().to_extid().to_u29() >> 8);
    }

    static inline constexpr uint8_t frame_to_piececnt(
        const hal::BxCanFrame & frame
    ){
        return static_cast<uint8_t>(frame.identifier().to_extid().to_u29() & 0xff);
    }

};


}