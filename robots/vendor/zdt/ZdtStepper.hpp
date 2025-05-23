#include "core/math/realmath.hpp"
#include "core/utils/Option.hpp"

#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"

#include "types/range/range.hpp"

namespace ymd::drivers{


struct ZdtMotor_Collections{
    using NodeId = uint8_t;
    using Buf = sstl::vector<uint8_t, 16>;
    enum class VerifyType:uint8_t{
        X6B,
        XOR,
        CRC8
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


    void write_data(const NodeId id, const std::span<const uint8_t>buf){

        if(uart_.is_some()){
            uart_write_data(uart_.unwrap(), id, std::span<const uint8_t>(buf));
        }else if(can_.is_some()){
            can_write_data(can_.unwrap(), id, std::span<const uint8_t>(buf));
        }
    }
private:
    ymd::Option<hal::Uart &> uart_;
    ymd::Option<hal::Can &> can_;

    void can_write_data(
            hal::Can & can, 
            const NodeId id, 
            const std::span<const uint8_t> buf){
        Range2_t<size_t> store_window = Range2u{0,buf.size()};
        Range2_t<size_t> op_window = {0,0};
    
        do{
            op_window = store_window.grid_forward(op_window, 8);
            if(op_window.length() != 0){
                const auto msg = hal::CanMsg::from_bytes(
                    hal::CanStdId(uint32_t(id << 8) | (uint32_t(op_window.from) / 8)), 
                    std::span(buf.begin() + op_window.from, op_window.length()));
                // DEBUG_PRINTLN(msg);
                (void)can.write(msg);
            }
        }while(op_window.length());
    }

    static void uart_write_data(
            hal::Uart & uart, 
            const NodeId id, 
            const std::span<const uint8_t> buf){
        uart.write1(id);
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
        struct Frame{
            const uint8_t head = 0xfb;//1
            uint8_t is_neg;//2
            uint16_t spd_data;//3-4
            uint32_t pos_data;//5-8
            uint8_t is_relative; //9
            uint8_t sync_flag; //10
        }__packed;

        write_data(Frame{
            .is_neg = pos < 0,
            .spd_data = speed_to_speed_data_msb(real_t(2000) / 60),
            .pos_data = position_to_position_data_msb(pos),
            .is_relative = false,
            .sync_flag = sync_flag
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
        // write_data(frame);  
    }

    void enable(const bool en = true){
        struct Frame{
            const uint8_t head = 0xf3;//1
            const uint8_t head2 = 0xab;
            uint8_t en;
        }__packed;

        write_data(Frame{
            .en = en
        });
    }


    void trigger_cali(){
        write_data(std::array<uint8_t, 3>{0x06, 0x45, 0x6b});  
    }
private:
    using Phy = ZdtMotorPhy;
    Phy phy_;

    NodeId id_ = 0x01;

    uint8_t sync_flag = 0x00;


    VerifyType verify_type = VerifyType::X6B;

    
    template<typename T>
    void write_data(const T & obj){
        Buf buf;
        array_append(buf, std::span(reinterpret_cast<const uint8_t *>(&obj), sizeof(T)));
        array_append(buf, get_verify_code(verify_type, std::span(
            reinterpret_cast<const uint8_t *>(&obj), 
            sizeof(T)))
        );

        phy_.write_data(id_, {buf.data(), buf.size()});
    }

    static inline uint8_t get_verify_code(const VerifyType type, std::span<const uint8_t> pdata ){
        switch(type){
            default:
                PANIC();
            case VerifyType::X6B:
                return uint8_t{0x6b};
            case VerifyType::XOR:{
                uint8_t code{0};
                for(size_t i = 0; i < pdata.size(); i++){
                    code ^= pdata[i];
                };
                return code;
            }
            case VerifyType::CRC8:{
                uint16_t crc = 0xffff;
                for(size_t i = 0; i < pdata.size(); i++){
                    crc ^= (uint16_t)(pdata[i]) << 8;
                    for(uint8_t j = 0; j < 8; j++){
                        if(crc & 0x8000) crc ^= 0x1021;
                        crc <<= 1;
                    }
                }
                return uint8_t(crc >> 8);
            }
        }
    }

    static inline void array_append(Buf & dst, std::span<const uint8_t> pdata){
        for(size_t i = 0; i < pdata.size(); i++){
            dst.push_back(pdata[i]);
        }
    }
    
    static inline void array_append(Buf & dst, const uint8_t data){
        dst.push_back(data);
    }

    
    scexpr uint16_t speed_to_speed_data_msb(const real_t speed){
        uint16_t speed_data = (uint16_t)(speed * 600);
        return BSWAP_16(speed_data);
    }

    scexpr uint32_t position_to_position_data_msb(const real_t position){
        uint32_t position_deg = uint32_t(position * 3600);
        return BSWAP_32(position_deg);
    }
};

}