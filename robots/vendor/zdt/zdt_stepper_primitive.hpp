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
    [[nodiscard]] constexpr uint8_t to_u8() const noexcept {
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

struct [[nodiscard]] CanFrameUtils{
    static constexpr hal::ClassicCanFrame make_can_frame(
        const NodeId node_id,
        const FuncCode func_code,
        const uint8_t piece_cnt,
        const std::span<const uint8_t> bytes
    ){
        constexpr size_t CAN_MAX_PAYLOAD_SIZE = 8;

        auto buf = HeaplessVector<uint8_t, CAN_MAX_PAYLOAD_SIZE>{};
        buf.append_unchecked(std::bit_cast<uint8_t>(func_code));
        buf.append_unchecked(bytes);

        return hal::ClassicCanFrame::from_parts(
            nodeid_and_piececnt_to_canid(node_id, piece_cnt),
            hal::ClassicCanPayload::from_bytes(buf.as_slice())
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
};


struct [[nodiscard]] Bytes2CanFrameIterator final:public CanFrameUtils{
    static constexpr size_t MAX_PAYLOAD_LENGTH_PER_CAN_FRAME = 7;
    explicit constexpr Bytes2CanFrameIterator(
        const NodeId node_id,
        const FuncCode func_code,
        const std::span<const uint8_t> context
    ):
        node_id_(node_id),
        func_code_(func_code),
        bytes_(context){;}

    [[nodiscard]] constexpr bool has_next() const noexcept {
        return bytes_.size() > offset_;
    }

    [[nodiscard]] constexpr hal::ClassicCanFrame next(){
        if(not has_next()){
            #ifdef NDEBUG
            __builtin_trap();
            #else
            __builtin_unreachable();
            #endif
        }

        const auto frame_len = std::min<size_t>(
            size_t(bytes_.size() - offset_),
            MAX_PAYLOAD_LENGTH_PER_CAN_FRAME
        );

        const auto frame = make_can_frame(
            node_id_, 
            func_code_,
            static_cast<uint8_t>(piece_cnt_),    
            bytes_.subspan(offset_, frame_len)
        );

        offset_ += frame_len;
        piece_cnt_++;

        return frame.clone();
    }

private:
    NodeId node_id_;
    FuncCode func_code_;
    std::span<const uint8_t> bytes_;
    size_t offset_ = 0;
    size_t piece_cnt_ = 0;
};


struct [[nodiscard]] FlatPacket final{
    NodeId node_id;
    FuncCode func_code;
    std::array<uint8_t, MAX_CONTEXT_BYTES> context;
    uint8_t payload_len;

    template<typename Receiver>
    Result<void, typename Receiver::Error> serialize(Receiver & receiver) const noexcept {
        auto & self = *this;
        {
            const uint8_t buf[] = {
                static_cast<uint8_t>(node_id.count),
                static_cast<uint8_t>(func_code)
            };

            if(const auto res = receiver.push_bytes(std::span(buf));
                res.is_err()) return Err(res.unwrap_err());
        }

        {
            const auto bytes = std::span<const uint8_t>(self.context.data(), self.payload_len);
            if(const auto res = receiver.push_bytes(bytes);
                res.is_err()) return Err(res.unwrap_err());
        }

        return Ok();
    }

    [[nodiscard]] constexpr std::span<const uint8_t> transmittable_bytes() const noexcept {
        return std::span<const uint8_t>(&node_id.count, static_cast<size_t>(payload_len));
    }

    constexpr Bytes2CanFrameIterator to_canframe_iter() const noexcept {
        auto & self = *this;
        return Bytes2CanFrameIterator(
            self.node_id,
            self.func_code,
            std::span<const uint8_t>(self.context.data(), self.payload_len)
        );
    }
};

static_assert(__builtin_offsetof(FlatPacket, FlatPacket::context) == 2, "packet not nested");

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
    static constexpr Rpm from_tps(const iq16 tps){
        const uint16_t temp = uint16_t(iq16(tps) * 600);
        return {__builtin_bswap16(temp)};
    }
    constexpr uint16_t to_u16() const noexcept {
        return bits;
    }

    uint16_t bits;
};

struct [[nodiscard]] PulseCnt final{
    using Self = PulseCnt;
    static constexpr uint32_t SCALE = 3200 * (256/16);

    static constexpr Self from_pulses(const uint32_t pulses){
        return {__builtin_bswap32(pulses)};
    }

    static constexpr Option<Self> from_angle(const Angular<uq16> angle){
        const uq16 turns = (angle.to_turns());
        const uint32_t frac_part = uint32_t(math::frac(turns) * SCALE);
        const uint32_t digit_part  = uint32_t(uint32_t(turns) * SCALE);
        const uint32_t pulses = uint32_t(frac_part + digit_part);
        return Some(from_pulses(pulses));
    }

    constexpr uint32_t to_u32() const noexcept {
        return bits;
    }

    uint32_t bits;
};

struct [[nodiscard]] AcclerationLevel final{
    using Self = AcclerationLevel;
    static constexpr Self from_tpss(const iq16 acc_per_second){
        // TODO
        (void)acc_per_second;
        return Self{10};
    }

    static constexpr Self zero(){
        return Self{0};
    }

    static constexpr Self from_u8(const uint8_t raw){
        return Self{raw};
    }
    uint8_t bits;
};

}
