#pragma once

#include "core/utils/Result.hpp"
#include "primitive/arithmetic/angle.hpp"

#include "core/math/realmath.hpp"
#include "core/container/inline_vector.hpp"
#include "core/magic/enum_traits.hpp"

#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"

#include "types/regions/range2.hpp"


namespace ymd::robots{


namespace zdtmotor{

struct [[nodiscard]] NodeId{
    using Self = NodeId;
    uint8_t count;

    static constexpr NodeId from_u8(uint8_t bits) {
        return NodeId{bits};
    }
    [[nodiscard]] constexpr uint8_t to_u8() const {
        return count;
    }
};

namespace prelude{
enum class Error:uint8_t{
    SubDivideOverflow,
    RxNoMsgToDump,
    RxMsgIdTypeNotMatch,
    RxMsgNodeIdNotTheSame,
    RxMsgFuncCodeNotTheSame,
    RxMsgPieceIsNotSteady,
    RxMsgVerifyed,
    RxMsgNoPayload
};

DEF_DERIVE_DEBUG(Error)

template<typename T = void>
using IResult = Result<T, Error>;


static constexpr size_t MAX_PACKET_BYTES = 16;

using Buf = HeaplessVector<uint8_t, MAX_PACKET_BYTES>;

enum class [[nodiscard]] VerifyMethod:uint8_t{
    X6B      = 0x00,
    XOR      = 0x01,
    CRC8     = 0x02,
    Default = X6B
};

enum class [[nodiscard]] FuncCode:uint8_t{
    TrigCali = 0x06,
    QueryHommingParaments = 0x22,
    QueryHommingStatus = 0x3B,
    UpdateHommingParaments = 0x4C,
    SetSubDivide = 0x84,
    TrigHomming = 0x9a,
    AbortHomming = 0x9c,

    Activation = 0xf3,
    SetCurrent = 0xf5,
    SetSpeed = 0xf6,
    // SetCurrent = 0xfb,
    SetPosition = 0xfd,
    Brake = 0xfe,
    // Stop = 0xfe,
    MultiAxisSync = 0xff
};

enum class [[nodiscard]] HommingMode:uint8_t{
    // 00表示触发单圈就近回
    // 01表示触发单圈方向回零
    // 02表示触发多圈无限位碰撞回零
    // 03表示触发多圈有限位开关回零

    LapNearest = 0x00,
    LapDirection = 0x01,
    LapsCollision = 0x02,
    LapsEndstop = 0x03
};

struct [[nodiscard]] HommingStatus{
    // 编码器就绪状态标志位     = 0x03 & 0x01 = 0x01
    // 校准表就绪状态标志位     = 0x03 & 0x02 = 0x01
    // 正在回零标志位            = 0x03 & 0x04 = 0x00
    // 回零失败标志位               = 0x03 & 0x08 = 0x00

    uint8_t encoder_ready:1;
    uint8_t cali_table_ready:1;
    uint8_t homing_in_progress:1;
    uint8_t homing_failed:1;
    uint8_t :4;
};

static_assert(sizeof(HommingStatus) == 1); 


struct [[nodiscard]] Bytes2CanFrameIterator{
    explicit constexpr Bytes2CanFrameIterator(
        const NodeId nodeid, 
        const FuncCode func_code,
        const std::span<const uint8_t> payload
    ):
        nodeid_(nodeid),
        func_code_(func_code),
        payload_(payload){;}


    constexpr bool has_next(){
        return payload_.size() - offset_;
    }
    constexpr hal::CanClassicFrame next(){
        constexpr size_t MAX_PAYLOAD_LENGTH = 7;
        const auto msg_len = MIN(
            payload_.size() - offset_, 
            MAX_PAYLOAD_LENGTH);

        const auto frame = make_canmsg(
            nodeid_, func_code_, 
            offset_ / MAX_PAYLOAD_LENGTH,
            payload_.subspan(offset_, msg_len)
        ); 

        offset_ += msg_len;

        return frame;
    }
private:
    static constexpr hal::CanClassicFrame make_canmsg(
        const NodeId nodeid,
        const FuncCode func_code,
        const uint8_t piece_cnt,
        const std::span<const uint8_t> bytes
    ){
        constexpr size_t CAN_MAX_PAYLOAD_SIZE = 8;
        auto buf = HeaplessVector<uint8_t, CAN_MAX_PAYLOAD_SIZE>{};
        buf.append_unchecked(std::bit_cast<uint8_t>(func_code));
        buf.append_unchecked(bytes);

        return hal::CanClassicFrame(
            map_nodeid_and_piececnt_to_canid(nodeid, piece_cnt),
            hal::CanClassicPayload::from_bytes(buf.as_slice())
        );
    }

    //固定为拓展帧
    static constexpr hal::CanExtId map_nodeid_and_piececnt_to_canid(
        const NodeId nodeid, 
        const uint8_t piece
    ){
        return hal::CanExtId::from_bits(
            uint32_t(nodeid.to_u8() << 8) | 
            (piece)
        );
    }

private:
    NodeId nodeid_;
    FuncCode func_code_;
    std::span<const uint8_t> payload_;
    size_t offset_ = 0;
};


struct [[nodiscard]] CanFrame2BytesDumper{
    struct [[nodiscard]] DumpInfo{
        NodeId nodeid;
        FuncCode func_code;
        Buf payload;
    };

    static constexpr IResult<DumpInfo> dump(
        std::span<const hal::CanClassicFrame> msgs
    ) {
        if(msgs.size() == 0)
            return Err(Error::RxNoMsgToDump);
        if(msgs[0].is_standard())
            return Err(Error::RxMsgIdTypeNotMatch);
        
        DumpInfo info;

        info.nodeid = ({
            const auto id = map_msg_to_nodeid(msgs[0]);
            for(size_t i = 1; i < msgs.size(); i++){
                const auto next_id = map_msg_to_nodeid(msgs[i]);
                if(id != next_id) return Err(Error::RxMsgNodeIdNotTheSame);
            }
            NodeId::from_u8(id);
        });

        for(size_t i = 0; i < msgs.size(); i++){
            const auto piece_cnt = map_msg_to_piececnt(msgs[i]);
            if(i != piece_cnt) 
                return Err(Error::RxMsgPieceIsNotSteady);
        }

        info.func_code = ({
            if(msgs[0].length() == 0) return Err(Error::RxMsgNoPayload);
            const auto func_code0 = std::bit_cast<FuncCode>(msgs[0].payload_bytes()[0]);
            for(size_t i = 0; i < msgs.size(); i++){
                const auto & frame = msgs[i];
                if(frame.length() == 0) return Err(Error::RxMsgNoPayload);
                const auto func_code = std::bit_cast<FuncCode>(frame.payload_bytes()[0]);
                if(func_code != func_code0)
                    return Err(Error::RxMsgFuncCodeNotTheSame);
            }
            func_code0;
        });

        for(size_t i = 0; i < msgs.size(); i++){
            const auto & frame = msgs[i];
            const auto msg_bytes = frame.payload_bytes()
                .subspan(1);
            info.payload.append_unchecked(msg_bytes);
        }

        //TODO add verify

        return Ok<DumpInfo>(info);
    }

    static inline constexpr uint8_t map_msg_to_nodeid(
        const hal::CanClassicFrame & frame
    ){
        return frame.identifier().try_to_extid().unwrap().to_u29() >> 8;
    }

    static inline constexpr uint8_t map_msg_to_piececnt(
        const hal::CanClassicFrame & frame
    ){
        return frame.identifier().try_to_extid().unwrap().to_u29() & 0xff;
    }

};


struct [[nodiscard]] VerifyUtils final{
    static constexpr uint8_t get_verify_code(
        const VerifyMethod method, 
        const FuncCode func_code,
        std::span<const uint8_t> bytes 
    ){
        switch(method){
            default:
                __builtin_unreachable();
            case VerifyMethod::X6B:
                return uint8_t{0x6b};
            case VerifyMethod::XOR:
                return VerifyUtils::by_xor(func_code, bytes);
            case VerifyMethod::CRC8:
                // TODO();
                // __builtin_unreachable();
                return VerifyUtils::by_crc8(func_code, bytes);
        }
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

struct [[nodiscard]] Rpm final{
    static constexpr Rpm from_speed(const real_t speed){
        const uint16_t temp = uint16_t(iq16(speed) * 600);
        return {BSWAP_16(temp)};
    }
    constexpr uint16_t to_u16() const {
        return raw_;
    }

    uint16_t raw_;
}__packed;

struct [[nodiscard]] PulseCnt final{
    static constexpr uint32_t SCALE = 3200 * (256/16);


    static constexpr PulseCnt from_position(const real_t position){
        const uint32_t frac_part = uint32_t(frac(position) * SCALE);
        const uint32_t int_part  = uint32_t(uint32_t(position) * SCALE);
        const uint32_t temp = uint32_t(frac_part + int_part);
        return {BSWAP_32(temp)};
    }

    static constexpr PulseCnt from_position(const Angle<real_t> angle){
        return from_position(angle.to_turns());
    }

    constexpr uint32_t to_u32() const {
        return raw_;
    }

    uint32_t raw_;
}__packed;

struct [[nodiscard]] AcclerationLevel{
    static constexpr AcclerationLevel from(const real_t acc_per_second){
        // TODO
        return AcclerationLevel{10};
    }

    static constexpr AcclerationLevel zero(){
        return AcclerationLevel{0};
    }

    static constexpr AcclerationLevel from_u8(const uint8_t raw){
        return AcclerationLevel{raw};
    }
    uint8_t raw_;
}__packed;

}

namespace msgs{
    using namespace prelude;
    // 地址 + 0xF3 + 0xAB + 使能状态 + 多机同步标志 + 校验字节
    struct [[nodiscard]] Actvation final{
        static constexpr FuncCode FUNC_CODE = FuncCode::Activation;
        //0
        const uint8_t _0 = 0xab;
        const bool en;
        const bool is_sync;
    }__packed;

    struct [[nodiscard]] SetPosition final{
        static constexpr FuncCode FUNC_CODE = FuncCode::SetPosition;
        bool is_ccw;
        Rpm rpm;
        AcclerationLevel acc_level;
        PulseCnt pulse_cnt;
        bool is_absolute; //9
        bool is_sync;
    }__packed;

    struct [[nodiscard]] SetSpeed final{
        static constexpr FuncCode FUNC_CODE = FuncCode::SetSpeed;

        bool is_ccw;//2
        Rpm rpm;//3-4
        AcclerationLevel acc_level;
        bool is_absolute; //9
        bool is_sync; //10
    }__packed;

    struct [[nodiscard]] SetSubDivides{
        // 01 84 8A 01 07 6B
        //  0x84 + 0x8A + 是否存储标志 + 细分值 + 校验字节

        static constexpr FuncCode FUNC_CODE = FuncCode::SetSubDivide;

        uint8_t _0 = 0x8A;
        bool is_burned = true;
        uint8_t subdivides;
    }__packed;

    struct [[nodiscard]] Brake final{
        static constexpr FuncCode FUNC_CODE = FuncCode::Brake;
        const uint8_t _0 = 0x98;
        const bool is_sync;
    }__packed;

    struct [[nodiscard]] TrigCali final{
        static constexpr FuncCode FUNC_CODE = FuncCode::TrigCali;
        const uint8_t _1 = 0x45;

        static constexpr TrigCali from_default(){
            return TrigCali{};
        }
    }__packed;

    struct [[nodiscard]] SetCurrent{
        static constexpr FuncCode FUNC_CODE = FuncCode::SetCurrent;//1

        bool is_ccw;//2
        Rpm rpm;//3-4
        PulseCnt pulse_cnt;//5-8
        bool is_absolute; //9
        bool is_sync; //10
    }__packed;

    struct [[nodiscard]] TrigHomming final{
        static constexpr FuncCode FUNC_CODE = FuncCode::TrigHomming;

        HommingMode homming_mode;
        bool is_sync;
    }__packed;

    struct [[nodiscard]] QueryHommingParaments final{
        static constexpr FuncCode FUNC_CODE = FuncCode::QueryHommingParaments;
    }__packed;
    

    struct [[nodiscard]] QueryHommingStatus final{
        static constexpr FuncCode FUNC_CODE = FuncCode::QueryHommingParaments;
    }__packed;
    

    template<typename Raw, typename T = std::decay_t<Raw>>
    [[nodiscard]] static std::span<const uint8_t> serialize(
        Raw && obj
    ){
        return std::span(
            reinterpret_cast<const uint8_t *>(&obj),
            magic::pure_sizeof_v<T>
        );
    }
}
}

}