#pragma once

#include "core/math/realmath.hpp"
#include "core/utils/Option.hpp"

#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"

#include "types/regions/range2/range2.hpp"

namespace ymd::robots{

struct ZdtMotor_Collections{

    struct NodeId{
        static constexpr NodeId from_u8(const uint8_t raw) {
            return NodeId{raw};
        }

        constexpr uint8_t to_u8() const {
            return id;
        }

        uint8_t id;
    };

    template<size_t N>
    struct InlineBuf{
        constexpr uint8_t & operator [](const size_t idx){
            ASSERT(idx < size_);
            return buf_[idx];
        }

        constexpr uint8_t operator [](const size_t idx) const {
            ASSERT(idx < size_);
            return buf_[idx];
        }

        constexpr void append(const uint8_t data){
            ASSERT(size_ + 1 <= N);
            buf_[size_] = data;
            size_ = size_ + 1;
        }

        constexpr void append(const std::span<const uint8_t> pbuf){
            ASSERT(size_ + pbuf.size() <= N);
            for(size_t i = 0; i < pbuf.size(); i++){
                buf_[size_ + i] = pbuf[i];
            }
            size_ += pbuf.size();
        }

        constexpr std::span<const uint8_t> to_span() const {
            return std::span(buf_.data(), size());
        }

        constexpr size_t size() const {return size_;}


    private:
        std::array<uint8_t, N> buf_;
        size_t size_ = 0;
    };

    using Buf = InlineBuf<16>;

    enum class VerifyMethod:uint8_t{
        X6B,
        XOR,
        CRC8
    };

    enum class FuncCode:uint8_t{
        Activation = 0xf3,
        SetSpeed = 0xf6,
        // SetCurrent = 0xfb,
        SetCurrent = 0xf5,
        SetPosition = 0xfd,
        Brake = 0xfe,
        // Stop = 0xfe,
        MultiAxisSync = 0xff, 
        TrigCali = 0x06,
        SetSubDivide = 0x84,
        TrigHomming = 0x9a,
        AbortHomming = 0x9c,
        QueryHommingParaments = 0x22,
        UpdateHommingParaments = 0x4C,
        GetHommingFlags = 0x3b

    };

    enum class HommingMode:uint8_t{
        // 00表示触发单圈就近回零，01表示触发单圈方向回零，
        // 02表示触发多圈无限位碰撞回零，03表示触发多圈有限位开关回零

        LapNearest = 0x00,
        LapDirection = 0x01,
        LapsCollision = 0x02,
        LapsEndstop = 0x03
    };

    static constexpr auto DEFAULT_VERIFY_METHOD = VerifyMethod::X6B;

    struct Bytes2CanMsgIterator{
        explicit constexpr Bytes2CanMsgIterator(
            const NodeId nodeid, 
            const FuncCode func_code,
            const std::span<const uint8_t> payload
        ):
            nodeid_(nodeid),
            func_code_(func_code),
            payload_(payload){;}


        constexpr Option<hal::CanMsg> next(){
            constexpr size_t CANMSG_PAYLOAD_MAX_LENGTH = 7;

            const auto msg_len = MIN(
                payload_.size() - offset_, 
                CANMSG_PAYLOAD_MAX_LENGTH);

            if(msg_len == 0) return None;

            const auto msg = make_canmsg(
                nodeid_, func_code_, 
                offset_ / CANMSG_PAYLOAD_MAX_LENGTH,
                payload_.subspan(offset_, msg_len)
            ); 

            offset_ += msg_len;

            return Some(msg);
        }
    private:
        static constexpr hal::CanMsg make_canmsg(
            const NodeId nodeid,
            const FuncCode func_code,
            const uint8_t piece_cnt,
            const std::span<const uint8_t> bytes
        ){
            auto buf = InlineBuf<8>{};
            buf.append(std::bit_cast<uint8_t>(func_code));
            buf.append(bytes);

            return hal::CanMsg::from_bytes(
                map_nodeid_and_piececnt_to_canstdid(nodeid, piece_cnt),
                buf.to_span()
            );
        }

        static constexpr hal::CanExtId map_nodeid_and_piececnt_to_canstdid(
            const NodeId nodeid, 
            const uint8_t piece
        ){
            return hal::CanExtId::from_raw(
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


    struct VerifyUtils final{
        static constexpr uint8_t get_verify_code(
            const VerifyMethod method, 
            const FuncCode FUNC_CODE,
            std::span<const uint8_t> bytes 
        ){
            switch(method){
                default:
                    __builtin_unreachable();
                case VerifyMethod::X6B:
                    return uint8_t{0x6b};
                case VerifyMethod::XOR:
                    return VerifyUtils::by_xor(FUNC_CODE, bytes);
                case VerifyMethod::CRC8:
                    TODO();
                    // return VerifyUtils::by_crc8(FUNC_CODE, bytes);
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
    
        // static constexpr uint8_t by_crc8(
        //     const FuncCode FUNC_CODE,
        //     const std::span<const uint8_t> bytes
        // ){
        //     uint16_t crc = 0xffff;
        //     for(size_t i = 0; i < bytes.size(); i++){
        //         crc ^= uint16_t(bytes[i]) << 8;
        //         for(uint8_t j = 0; j < 8; j++){
        //             if(crc & 0x8000) crc ^= 0x1021;
        //             crc <<= 1;
        //         }
        //     }
        //     return uint8_t(crc >> 8);
        // }

    };

    struct Rpm final{
        static constexpr Rpm from(const real_t speed){
            const auto temp = uint16_t(speed * 600);
            return {BSWAP_16(temp)};
        }
        constexpr uint16_t to_u16() const {
            return raw_;
        }

        uint16_t raw_;
    }__packed;

    struct PulseCnt final{
        static constexpr PulseCnt from(const real_t position){
            constexpr auto scale = 3200 * (256/16);
            const auto temp = uint32_t(q12(position) * scale);
            return {BSWAP_32(temp)};
        }

        constexpr uint32_t to_u32() const {
            return raw_;
        }

        uint32_t raw_;
    }__packed;

    struct AcclerationLevel{
        static constexpr AcclerationLevel from(const real_t acc_per_second){
            // TODO
            return AcclerationLevel{10};
        }

        static constexpr AcclerationLevel from_zero(){
            // TODO
            return AcclerationLevel{0};
        }

        static constexpr AcclerationLevel from_raw(const uint8_t raw){
            // TODO
            return AcclerationLevel{raw};
        }
        uint8_t raw_;
    }__packed;

    struct Payloads{
        // 地址 + 0xF3 + 0xAB + 使能状态 + 多机同步标志 + 校验字节
        struct Actvation final{
            static constexpr FuncCode FUNC_CODE = FuncCode::Activation;
            //0
            const uint8_t _0 = 0xab;
            const bool en;
            const bool is_sync;
        }__packed;

        struct SetPosition final{
            static constexpr FuncCode FUNC_CODE = FuncCode::SetPosition;
            bool is_ccw;
            Rpm rpm;
            AcclerationLevel acc_level;
            PulseCnt pulse_cnt;
            bool is_absolute; //9
            bool is_sync;
        }__packed;

        struct SetSpeed final{
            static constexpr FuncCode FUNC_CODE = FuncCode::SetSpeed;

            bool is_ccw;//2
            Rpm rpm;//3-4
            AcclerationLevel acc_level;
            bool is_absolute; //9
            bool is_sync; //10
        }__packed;

        struct SetSubDivides{
            // 01 84 8A 01 07 6B
            //  0x84 + 0x8A + 是否存储标志 + 细分值 + 校验字节

            static constexpr FuncCode FUNC_CODE = FuncCode::SetSubDivide;

            uint8_t _0 = 0x8A;
            bool is_burned = true;
            uint8_t subdivides;
        }__packed;

        struct Brake final{
            static constexpr FuncCode FUNC_CODE = FuncCode::Brake;
            const uint8_t _0 = 0x98;
            const bool is_sync;
        }__packed;

        struct TrigCali final{
            static constexpr FuncCode FUNC_CODE = FuncCode::TrigCali;
            const uint8_t _1 = 0x45;

            static constexpr TrigCali from_default(){
                return TrigCali{};
            }
        }__packed;

        struct SetCurrent{
            static constexpr FuncCode FUNC_CODE = FuncCode::SetCurrent;//1

            bool is_ccw;//2
            Rpm rpm;//3-4
            PulseCnt pulse_cnt;//5-8
            bool is_absolute; //9
            bool is_sync; //10
        }__packed;

        struct TrigHomming final{
            static constexpr FuncCode FUNC_CODE = FuncCode::TrigHomming;

            HommingMode homming_mode;
            bool is_sync;
        }__packed;

        struct QueryHommingParaments final{
            static constexpr FuncCode FUNC_CODE = FuncCode::QueryHommingParaments;
        }__packed;
        
        template<typename T>
        struct pure_sizeof_impl{
        private:
            [[no_unique_address]] T _1;
            uint8_t _2;
        };

        template<typename T>
        static constexpr size_t pure_sizeof_v = sizeof(pure_sizeof_impl<T>) - 1;


        template<typename Raw, typename T = std::decay_t<Raw>>
        static std::span<const uint8_t> serialize(
            Raw && obj
        ){
            return std::span(
                reinterpret_cast<const uint8_t *>(&obj),
                pure_sizeof_v<T>
            );
        }
    };
};

class ZdtMotorPhy final:
    public ZdtMotor_Collections{
public:


    ZdtMotorPhy(Some<hal::Can *> && can) : 
        uart_(ymd::None),
        can_(std::move(can)
    ){
        // reconf(cfg);
    }

    ZdtMotorPhy(Some<hal::Uart *> && uart) : 
        uart_(std::move(uart)),
        can_(ymd::None)
    {
        // reconf(cfg);

    }


    void write_bytes(
        const NodeId id, 
        const FuncCode func_code,
        const std::span<const uint8_t> bytes
    );
private:
    Option<hal::Uart &> uart_;
    Option<hal::Can &> can_;

    static void can_write_bytes(
        hal::Can & can, 
        const NodeId id, 
        const FuncCode func_code,
        const std::span<const uint8_t> bytes
    );

    static void uart_write_bytes(
        hal::Uart & uart, 
        const NodeId id, 
        const FuncCode func_code,
        const std::span<const uint8_t> bytes
    );
};


}