#include "core/math/realmath.hpp"
#include "core/utils/Option.hpp"

#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"

#include "types/regions/range2/range2.hpp"

namespace ymd::drivers{


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
        LapsSwitch = 0x03
    };

    static constexpr auto DEFAULT_VERIFY_METHOD = VerifyMethod::X6B;

    struct Bytes2CanMsgIterator{
        using Input = std::span<const uint8_t>;
        using Output = hal::CanMsg;

        explicit constexpr Bytes2CanMsgIterator(
            const NodeId nodeid, 
            const FuncCode func_code,
            const Input payload
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
                payload_.subspan(offset_, CANMSG_PAYLOAD_MAX_LENGTH)
            ); 

            offset_ += CANMSG_PAYLOAD_MAX_LENGTH;

            return Some(msg);
        }

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

        static constexpr hal::CanStdId map_nodeid_and_piececnt_to_canstdid(
            const NodeId nodeid, 
            const uint8_t piece
        ){
            return hal::CanStdId::from_raw(
                uint32_t(nodeid.to_u8() << 8) | 
                (piece)
            );
        }

    private:
        NodeId nodeid_;
        FuncCode func_code_;
        Input payload_;
        size_t offset_ = 0;
    };


    struct VerifyUtils final{
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
                    TODO();
                    // return VerifyUtils::by_crc8(func_code, bytes);
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
        //     const FuncCode func_code,
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
    };

    struct PulseCnt final{
        static constexpr PulseCnt from(const real_t position){
            const auto temp = uint32_t(position * 3600);
            return {BSWAP_32(temp)};
        }

        constexpr uint32_t to_u32() const {
            return raw_;
        }

        uint32_t raw_;
    };

    struct AcclerationLevel{
        static constexpr AcclerationLevel from(const real_t acc_per_second){
            // TODO
            return AcclerationLevel{10};
        }
        uint8_t raw_;
    };

    struct Payloads{
        // 地址 + 0xF3 + 0xAB + 使能状态 + 多机同步标志 + 校验字节
        struct Actvation final{
            static constexpr FuncCode func_code = FuncCode::Activation;
            //0
            const uint8_t _0 = 0xab;
            const bool en;
            const bool is_sync;
        }__packed;

        struct SetPosition final{
            static constexpr FuncCode func_code = FuncCode::SetPosition;
            bool is_ccw;
            Rpm rpm;
            AcclerationLevel acc_level;
            PulseCnt pulse_cnt;
            bool is_absolute; //9
            bool is_sync;
        }__packed;

        struct SetSpeed final{
            static constexpr FuncCode func_code = FuncCode::SetSpeed;

            bool is_ccw;//2
            Rpm rpm;//3-4
            AcclerationLevel acc_level;
            bool is_absolute; //9
            bool is_sync; //10
        }__packed;

        struct SetSubDivides{
            // 01 84 8A 01 07 6B
            //  0x84 + 0x8A + 是否存储标志 + 细分值 + 校验字节

            static constexpr FuncCode func_code = FuncCode::SetSubDivide;

            uint8_t _0 = 0x8A;
            bool is_burned = true;
            uint8_t subdivides;
        }__packed;

        struct Brake final{
            static constexpr FuncCode func_code = FuncCode::Brake;
            const uint8_t _0 = 0x98;
            const bool is_sync;
        }__packed;

        struct TrigCali final{
            static constexpr FuncCode func_code = FuncCode::TrigCali;
            const uint8_t _1 = 0x45;

            static constexpr TrigCali from_default(){
                return TrigCali{};
            }
        }__packed;

        struct SetCurrent{
            static constexpr FuncCode func_code = FuncCode::SetCurrent;//1

            bool is_ccw;//2
            Rpm rpm;//3-4
            PulseCnt pulse_cnt;//5-8
            bool is_absolute; //9
            bool is_sync; //10
        }__packed;

        struct TrigHomming final{
            static constexpr FuncCode func_code = FuncCode::TrigHomming;

            HommingMode homming_mode;
            bool is_sync;
        }__packed;

        struct QueryHommingParaments final{
            static constexpr FuncCode func_code = FuncCode::QueryHommingParaments;
        }__packed;

        static constexpr size_t a = sizeof(QueryHommingParaments);
        
        template<typename T>
        struct sizeof_with_zero_impl{
            [[no_unique_address]] T _;
            uint8_t _2;
        };

        template<typename T>
        static constexpr size_t sizeof_with_zero_v = sizeof(sizeof_with_zero_impl<T>) - 1;


        template<typename Raw, typename T = std::decay_t<Raw>>
        static std::span<const uint8_t> serialize(
            Raw && obj
        ){
            return std::span(
                reinterpret_cast<const uint8_t *>(&obj),
                sizeof_with_zero_v<T>
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
    ){
        // DEBUG_PRINTLN(std::hex, "tx", bytes);
        if(uart_.is_some()){
            uart_write_bytes(
                uart_.unwrap(), 
                id, func_code, bytes
            );
        }else if(can_.is_some()){
            can_write_bytes(
                can_.unwrap(), 
                id, func_code, bytes
            );
        }else{
            PANIC();
        }
    }
private:
    Option<hal::Uart &> uart_;
    Option<hal::Can &> can_;

    static void can_write_bytes(
        hal::Can & can, 
        const NodeId id, 
        const FuncCode func_code,
        const std::span<const uint8_t> bytes
    ){
        auto iter = Bytes2CanMsgIterator(id, func_code, bytes);

        while(true){
            const auto may_msg = iter.next();
            if(may_msg.is_none()) break;
            can.write(may_msg.unwrap());
        }
    }

    static void uart_write_bytes(
        hal::Uart & uart, 
        const NodeId id, 
        const FuncCode func_code,
        const std::span<const uint8_t> bytes
    ){
        uart.write1(id.to_u8());
        uart.write1(std::bit_cast<uint8_t>(func_code));
        uart.writeN(reinterpret_cast<const char *>(
            bytes.data()), bytes.size());

        DEBUG_PRINTLN(id.to_u8(), std::bit_cast<uint8_t>(func_code), bytes);
    }
};



class ZdtMotor final:
    public ZdtMotor_Collections{
public:
    struct Config{
        NodeId nodeid;
    };


    ZdtMotor(const Config & cfg, Some<hal::Can *> && can) : 
        phy_(std::move(can)
    ){
        reconf(cfg);
    }

    ZdtMotor(const Config & cfg, Some<hal::Uart *> && uart) : 
        phy_(std::move(uart)
    ){
        reconf(cfg);
    }


    void reconf(const Config & cfg){
        nodeid_ = cfg.nodeid;
    }

    void set_target_position(const real_t pos){
        write_payload(Payloads::SetPosition{
            .is_ccw = pos < 0,
            .rpm = Rpm::from(0.07_r),
            .pulse_cnt = PulseCnt::from(ABS(pos)),
            .is_absolute = false,
            .is_sync = is_sync_
        });
    }

    void set_target_speed(const real_t spd){
        write_payload(Payloads::SetSpeed{
            .is_ccw = spd < 0,
            .rpm = Rpm::from(20),
            .acc_level = AcclerationLevel::from(0),
            .is_absolute = false,
            .is_sync = is_sync_
        });     
    }

    void brake(){
        write_payload(Payloads::Brake{
            .is_sync = is_sync_
        });
    }

    void set_subdivides(const uint16_t subdivides){
        if(subdivides > 256) PANIC();
        write_payload(Payloads::SetSubDivides{
            .subdivides = uint8_t(subdivides & 0xff)
        });
    }

    void enable(const Enable en = EN){
        write_payload(Payloads::Actvation{
            .en = en == EN,
            .is_sync = is_sync_
        });
    }


    void trigger_cali(){
        write_payload(Payloads::TrigCali::from_default());  
    }

    void query_homming_paraments(){
        write_payload(Payloads::QueryHommingParaments{});
    }

    void trig_homming(const HommingMode mode){
        write_payload(Payloads::TrigHomming{
            .homming_mode = mode,
            .is_sync = is_sync_
        });
    }
private:
    using Phy = ZdtMotorPhy;
    Phy phy_;

    static constexpr auto DEFAULT_NODE_ID = NodeId::from_u8(0x01);
    NodeId nodeid_ = DEFAULT_NODE_ID;

    bool is_sync_ = false;
    VerifyMethod verify_method_ = DEFAULT_VERIFY_METHOD;



    template<typename Raw, typename T = std::decay_t<Raw>>
    static constexpr Buf map_payload_to_bytes(
        const VerifyMethod verify_method,
        Raw && obj
    ){
        Buf buf;

        const auto bytes = Payloads::serialize(obj);

        buf.append(bytes);
        buf.append(VerifyUtils::get_verify_code(
            verify_method,
            T::func_code,
            bytes
        ));
        
        return buf;
    }

    template<typename T>
    void write_payload(const T & obj){
        const auto buf = map_payload_to_bytes(verify_method_, obj);
        const auto bytes = buf.to_span();

        phy_.write_bytes(
            nodeid_, 
            T::func_code, 
            bytes
        );
    }

};

}