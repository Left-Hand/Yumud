#include "core/math/realmath.hpp"
#include "core/utils/Option.hpp"

#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"

#include "types/regions/range2/range2.hpp"

namespace ymd::drivers{


struct ZdtMotor_Collections{

    struct NodeId{
        static constexpr NodeId from(const uint8_t raw) {
            return NodeId{raw};
        }

        constexpr uint8_t to() const {
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

        void append(const uint8_t data){
            ASSERT(size_ + 1 <= N);
            buf_[size_] = data;
            size_ = size_ + 1;
        }

        void append(const std::span<const uint8_t> pbuf){
            ASSERT(size_ + pbuf.size() <= N);
            for(size_t i = 0; i < pbuf.size(); i++){
                buf_[size_ + i] = pbuf[i];
            }
            size_ += pbuf.size();
        }

        constexpr std::span<const uint8_t> to_span() const {
            return std::span(buf_);
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
        SetPosition = 0xfd,
        Stop = 0xfe,
        MultiAxisSync = 0xff, 
        TrigCali = 0x06
    };

    static constexpr auto DEFAULT_VERIFY_METHOD = VerifyMethod::X6B;

    struct Bytes2CanMsgIterator{
        using Input = std::span<const uint8_t>;
        using Output = hal::CanMsg;

        explicit constexpr Bytes2CanMsgIterator(
            const NodeId nodeid, 
            const FuncCode func_code,
            const Input bytes
        ):
            nodeid_(nodeid),
            func_code_(func_code),
            bytes_(bytes){;}


        constexpr Option<hal::CanMsg> next(){
            constexpr size_t CANMSG_PAYLOAD_LENGTH = 7;

            const auto msg_len = MIN(
                bytes_.size() - offset_, 
                CANMSG_PAYLOAD_LENGTH);

            if(msg_len == 0) return None;

            const auto msg = hal::CanMsg::from_bytes(
                map_nodeid_to_canid(nodeid_, offset_ / CANMSG_PAYLOAD_LENGTH),
                std::span<const uint8_t>(bytes_.subspan(offset_, CANMSG_PAYLOAD_LENGTH))
            );

            offset_ += CANMSG_PAYLOAD_LENGTH;

            return Some(msg);
        }

        static constexpr hal::CanStdId map_nodeid_to_canid(
            const NodeId id, 
            const uint8_t piece
        ){
            return hal::CanStdId::from_raw(
                uint32_t(id.to() << 8) | 
                (piece)
            );
        }

    private:
        NodeId nodeid_;
        FuncCode func_code_;
        Input bytes_;
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

    struct Payloads{
        struct Actvation final{
            static constexpr FuncCode func_code = FuncCode::Activation;
            //0
            const uint8_t payload = 0xab;
            //1
            uint8_t en;
            //2
        }__packed;

        struct SetPosition final{
            static constexpr FuncCode func_code = FuncCode::SetPosition;

            bool is_neg;//2
            uint16_t spd_data;//3-4
            uint32_t pos_data;//5-8
            bool is_relative; //9
            uint8_t sync_flag; //10
        }__packed;

        struct TrigCali{
            static constexpr FuncCode func_code = FuncCode::TrigCali;
            const uint8_t _1 = 0x45;
            const uint8_t _2 = 0x6b;

            static constexpr TrigCali from_default(){
                return TrigCali{};
            }
        };

        template<typename Raw, typename T = std::decay_t<Raw>>
        static std::span<const uint8_t> serialize(
            Raw && obj
        ){
            return std::span(
                reinterpret_cast<const uint8_t *>(&obj),
                sizeof(T)
            );
        }
    };

    struct SpeedData final{

    };

    struct PositionData final{

    };
};

class ZdtMotorPhy final:
    public ZdtMotor_Collections{
public:
    ZdtMotorPhy(Some<hal::Can *> && can) : 
        uart_(ymd::None),
        can_(std::move(can))
        {;}

    ZdtMotorPhy(Some<hal::Uart *> && uart) : 
        uart_(std::move(uart)),
        can_(ymd::None)
        {;}


    void write_bytes(
        const NodeId id, 
        const FuncCode func_code,
        const std::span<const uint8_t>buf
    ){

        if(uart_.is_some()){
            uart_write_bytes(uart_.unwrap(), id, func_code, std::span<const uint8_t>(buf));
        }else if(can_.is_some()){
            can_write_bytes(can_.unwrap(), id, func_code, std::span<const uint8_t>(buf));
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
        const std::span<const uint8_t> buf
    ){
        auto iter = Bytes2CanMsgIterator(id, func_code, buf);

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
        const std::span<const uint8_t> buf
    ){
        uart.write1(id.to());
        uart.writeN(reinterpret_cast<const char *>(buf.data()), buf.size());
    }
};

class ZdtMotor final:
    public ZdtMotor_Collections{
public:
    ZdtMotor(Some<hal::Can *> &&  can) : 
        phy_(std::move(can)){;}

    ZdtMotor(Some<hal::Uart *> && uart) : 
        phy_(std::move(uart)){;}
    void init(){
        
    }

    void set_target_position(const real_t pos){
        write_payload(Payloads::SetPosition{
            .is_neg = pos < 0,
            .spd_data = speed_to_speed_data_msb(real_t(2000) / 60),
            .pos_data = position_to_position_data_msb(pos),
            .is_relative = false,
            .sync_flag = sync_flag_
        });        
    }


    void set_target_speed(const real_t spd){
        
    }

    void set_target_current(const real_t curr){
        struct Frame{
            uint8_t head = 0xfb;//1
            uint8_t head2 = 0xf5;//1
            uint8_t is_neg;//2
            uint16_t spd_data;//3-4
            uint32_t pos_data;//5-8
            uint8_t is_relative; //9
            uint8_t sync = 0; //10
            uint8_t tail = 0x6b;//11
        }__packed;

        // Frame frame;
        // frame.is_neg = pos < 0;
        // frame.spd_data = speed_to_speed_data_msb(real_t(2000) / 60);
        // frame.pos_data = position_to_position_data_msb(pos);
        // frame.is_relative = false;
        // write_payload(frame);  
    }

    void enable(const Enable en = EN){


        write_payload(Payloads::Actvation{
            .en = en == EN
        });
    }


    void trigger_cali(){
        write_payload(Payloads::TrigCali::from_default());  
    }
private:
    using Phy = ZdtMotorPhy;
    Phy phy_;

    static constexpr auto DEFAULT_NODE_ID = NodeId::from(0x01);
    NodeId id_ = DEFAULT_NODE_ID;

    uint8_t sync_flag_ = 0x00;
    VerifyMethod verify_method_ = DEFAULT_VERIFY_METHOD;



    template<typename Raw, typename T = std::decay_t<Raw>>
    static constexpr std::span<const uint8_t> map_payload_to_bytes(
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
        
        return buf.to_span();
    }

    template<typename T>
    void write_payload(const T & obj){

        phy_.write_bytes(
            id_, 
            T::func_code, 
            map_payload_to_bytes(verify_method_, obj)
        );
    }

    
    static constexpr uint16_t speed_to_speed_data_msb(const real_t speed){
        uint16_t speed_data = uint16_t(speed * 600);
        return BSWAP_16(speed_data);
    }

    static constexpr uint32_t position_to_position_data_msb(const real_t position){
        uint32_t position_deg = uint32_t(position * 3600);
        return BSWAP_32(position_deg);
    }
};

}