#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "hal/bus/can/can.hpp"

#include "types/range/range.hpp"
#include "core/math/realmath.hpp"
#include "hal/bus/uart/uarthw.hpp"


class ZdtMotor{
protected:
    using NodeId = uint8_t;

    Uart * uart_;
    Can * can_;

    
    NodeId id = 0x01;

    bool b_extid = false;
    uint8_t sync_flag = 0x00;

    enum class VerifyType:uint8_t{
        X6B,
        XOR,
        CRC8
    };
    VerifyType verify_type = VerifyType::X6B;
    
    using Buf = sstl::vector<std::byte, 16>;

    static inline std::byte get_verify_code(const VerifyType type, std::span<const std::byte> pdata ){
        switch(type){
            default:
                PANIC();
            case VerifyType::X6B:
                return std::byte{0x6b};
            case VerifyType::XOR:{
                std::byte code{0};
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
                return std::byte(crc >> 8);
            }
        }
    }

    static inline void array_append(Buf & dst, std::span<const std::byte> pdata){
        for(size_t i = 0; i < pdata.size(); i++){
            dst.push_back(pdata[i]);
        }
    }
    
    static inline void array_append(Buf & dst, const std::byte data){
        dst.push_back(data);
    }

    template<typename T>
    void write_data(const T & obj){

        Buf buf;
        array_append(buf, std::span(reinterpret_cast<const std::byte *>(&obj), sizeof(T)));
        // array_append(buf, sync_flag);
        array_append(buf, get_verify_code(verify_type, std::span(reinterpret_cast<const std::byte *>(&obj), sizeof(T))));

        if(uart_){
            uart_->write1((char)id);
            uart_->writeN((char *)buf.begin(), (size_t)buf.size());
        }
        
        else if(can_){
            Range_t<size_t> store_window = Rangei{0,buf.size()};
            Range_t<size_t> op_window = {0,0};
        
            do{
                op_window = store_window.grid_forward(op_window, 8);
                if(op_window){
                    CanMsg msg = CanMsg::from_bytes(hal::CanStdId(uint32_t(id << 8) | (uint32_t(op_window.from) / 8)), 
                                        std::span(buf.begin() + op_window.from, op_window.length()));
                    msg.set_ext(b_extid);
                    DEBUG_PRINTLN(msg);
                    if(can_)can_->write(msg);
                }
            }while(op_window);
        }
    }

    scexpr uint16_t speed_to_speed_data_msb(const real_t speed){
        uint16_t speed_data = (uint16_t)(speed * 600);
        return BSWAP_16(speed_data);
    }

    scexpr uint32_t position_to_position_data_msb(const real_t position){
        uint32_t position_deg = uint32_t(position * 3600);
        return BSWAP_32(position_deg);
    }
public:
    ZdtMotor(Can & _can) : 
        uart_(nullptr),
        can_(&_can)
        {}

    ZdtMotor(Uart & _uart) : 
        uart_(&_uart),
        can_(nullptr)
        {}

    void init(){
        
    }

    void setTargetPosition(const real_t pos){
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

    void setTargetSpeed(const real_t spd){
        
    }

    void setTargetCurrent(const real_t curr){
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

    void triggerCali(){
        write_data(std::array<uint8_t, 3>{0x06, 0x45, 0x6b});  
    }
};


void zdt_main(UartHw & logger){
    logger.init(576000, CommStrategy::Blocking);
    DEBUGGER.retarget(&logger);
    DEBUGGER.setEps(4);
    
    can1.init(1_M);

    ZdtMotor motor{can1};
    
    
    delay(10);
    motor.enable();
    delay(10);
    motor.triggerCali();
    while(true){
        motor.enable();
        motor.setTargetPosition(sin(time()));    
        delay(10);
        if(can1.available()) DEBUG_PRINTLN(can1.read());
        // DEBUG_PRINTLN(can1.pending(), can1.getRxErrCnt(), can1.getTxErrCnt());
    }
}