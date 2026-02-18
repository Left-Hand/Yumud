#pragma once

#include "core/utils/Result.hpp"

#include "core/math/realmath.hpp"
#include "core/container/heapless_vector.hpp"
#include "core/tmp/reflect/enum.hpp"

#include "primitive/arithmetic/angular.hpp"
#include "primitive/can/bxcan_frame.hpp"

#include "algebra/regions/range2.hpp"


namespace ymd::robots::zdtmotor{
static constexpr size_t MAX_CONTEXT_BYTES = 12;
static constexpr size_t MAX_PACKET_BYTES = 16;

struct [[nodiscard]] NodeId final{
    using Self = NodeId;
    uint8_t count;

    static constexpr NodeId from_u8(uint8_t bits) {
        return NodeId{bits};
    }
    [[nodiscard]] constexpr uint8_t to_u8() const {
        return count;
    }
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


struct [[nodiscard]] FlatPacket final{
    NodeId node_id;
    FuncCode func_code;
    uint8_t context[MAX_CONTEXT_BYTES];
    uint8_t payload_len;

    constexpr std::span<const uint8_t> payload_bytes() const {
        return std::span<const uint8_t>(context, payload_len);
    }

    constexpr std::span<const uint8_t> tailed_context_bytes() const {
        //with crc
        return std::span<const uint8_t>(context, payload_len + 1);
    }

    void set_tailed_context_bytes(std::span<const uint8_t> bytes) {
        if(bytes.size() < 1) __builtin_trap();
        payload_len = bytes.size() - 1;
        std::copy(bytes.begin(), bytes.end(), context);
    }

    constexpr std::span<const uint8_t> bytes() const {
        return std::span<const uint8_t>(
            (&node_id.count)
            , payload_len
        );
    }

    struct [[nodiscard]] Bytes2CanFrameIterator{
        static constexpr size_t MAX_PAYLOAD_LENGTH_PER_CAN_FRAME = 7;
        explicit constexpr Bytes2CanFrameIterator(
            const NodeId node_id,
            const FuncCode func_code,
            const std::span<const uint8_t> tailed_context_bytes
        ):
            node_id_(node_id),
            func_code_(func_code),
            tailed_context_bytes_(tailed_context_bytes){;}

        explicit constexpr Bytes2CanFrameIterator(
            const FlatPacket & flat_packet
        ):
            Bytes2CanFrameIterator(
                flat_packet.node_id,
                flat_packet.func_code,
                flat_packet.tailed_context_bytes()
            ){;}

        [[nodiscard]] constexpr bool has_next() const {
            return tailed_context_bytes_.size() > offset_;
        }
        [[nodiscard]] constexpr hal::BxCanFrame next(){

            const auto frame_len = MIN(
                tailed_context_bytes_.size() - offset_,
                MAX_PAYLOAD_LENGTH_PER_CAN_FRAME
            );

            const auto frame = make_canmsg(
                node_id_, func_code_,
                offset_ / MAX_PAYLOAD_LENGTH_PER_CAN_FRAME,
                tailed_context_bytes_.subspan(offset_, frame_len)
            );

            offset_ += frame_len;

            return frame;
        }
    private:
        static constexpr hal::BxCanFrame make_canmsg(
            const NodeId node_id,
            const FuncCode func_code,
            const uint8_t piece_cnt,
            const std::span<const uint8_t> bytes
        ){
            constexpr size_t CAN_MAX_PAYLOAD_SIZE = 8;
            auto buf = HeaplessVector<uint8_t, CAN_MAX_PAYLOAD_SIZE>{};
            buf.append_unchecked(std::bit_cast<uint8_t>(func_code));
            buf.append_unchecked(bytes);

            return hal::BxCanFrame(
                nodeid_and_piececnt_to_canid(node_id, piece_cnt),
                hal::BxCanPayload::from_bytes(buf.as_slice())
            );
        }

        //固定为拓展帧
        static constexpr hal::CanExtId nodeid_and_piececnt_to_canid(
            const NodeId node_id,
            const uint8_t piece
        ){
            return hal::CanExtId::from_bits(
                uint32_t(node_id.to_u8() << 8) |
                uint32_t(piece << 0)
            );
        }

    private:
        NodeId node_id_;
        FuncCode func_code_;
        std::span<const uint8_t> tailed_context_bytes_;
        size_t offset_ = 0;
    };

    constexpr Bytes2CanFrameIterator to_canframe_iter() const {
        auto & self = *this;
        return Bytes2CanFrameIterator(
            self.node_id,
            self.func_code,
            self.payload_bytes()
        );
    }
};

static_assert(__builtin_offsetof(FlatPacket, FlatPacket::context[0]) == 2, "packet not nested");

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


using Buf = HeaplessVector<uint8_t, MAX_PACKET_BYTES>;

enum class [[nodiscard]] VerifyMethod:uint8_t{
    X6B      = 0x00,
    XOR      = 0x01,
    CRC8     = 0x02,
    Default = X6B
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

struct [[nodiscard]] HommingStatus final{
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



struct [[nodiscard]] Rpm final{
    static constexpr Rpm from_speed(const iq16 speed){
        const uint16_t temp = uint16_t(iq16(speed) * 600);
        return {__builtin_bswap16(temp)};
    }
    constexpr uint16_t to_u16() const {
        return bits;
    }

    uint16_t bits;
};

struct [[nodiscard]] PulseCnt final{
    static constexpr uint32_t SCALE = 3200 * (256/16);

    static constexpr PulseCnt from_pulses(const uint32_t pulses){
        return {__builtin_bswap32(pulses)};
    }

    static constexpr Option<PulseCnt> from_angle(const Angular<uq16> angle){
        const uq16 turns = (angle.to_turns());
        const uint32_t frac_part = uint32_t(math::frac(turns) * SCALE);
        const uint32_t int_part  = uint32_t(uint32_t(turns) * SCALE);
        const uint32_t pulses = uint32_t(frac_part + int_part);
        return Some(from_pulses(pulses));
    }

    constexpr uint32_t to_u32() const {
        return bits;
    }

    uint32_t bits;
};

struct [[nodiscard]] AcclerationLevel{
    static constexpr AcclerationLevel from(const iq16 acc_per_second){
        // TODO
        return AcclerationLevel{10};
    }

    static constexpr AcclerationLevel zero(){
        return AcclerationLevel{0};
    }

    static constexpr AcclerationLevel from_u8(const uint8_t raw){
        return AcclerationLevel{raw};
    }
    uint8_t bits;
};

}
