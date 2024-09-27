#include "tb.h"

#include "hal/bus/can/can.hpp"

#include "types/range/range_t.hpp"
#include "sys/debug/debug_inc.h"

class ZdtMotor{
protected:
    using NodeId = uint8_t;

    Can & can_;
    NodeId id = 0x01;

    template<typename T>
    void write_data(const T & obj){
        const uint8_t * data = reinterpret_cast<const uint8_t *>(&obj);
        size_t len = sizeof(obj);
        // size_t i = 0;
        // while(len - i > 0){
        //     auto msg = CanMsg(id << 8, data + i, CLAMP((len - i),0, 8));
        //     msg.setExt(true);
        //     DEBUG_PRINTLN(msg);
        //     i += 8;
        // }

        Range_t<size_t> store_window = Rangei{0,len};
        Range_t<size_t> op_window = {0,0};
        do{
            op_window = store_window.grid_forward(op_window, 8);
            if(op_window){
                // const uint8_t * ptr = data + (op_window.from);
                // DEBUG_PRINTLN(op_window.from, op_window.length());
                // CanMsg msg = CanMsg{uint32_t(id << 8) | (uint32_t(op_window.from) / 8), false};
                CanMsg msg = CanMsg{uint32_t(id << 8) | (uint32_t(op_window.from) / 8), data + op_window.from, op_window.length()};
                // msg.setSize(op_window.length());
                // for(size_t i = 0; i < op_window.length(); i++){
                //     msg[i] = data[i + op_window.from];
                // }
                // msg.setExt(true);
                // WRITE_POOL(op_window.from, ptr, op_window.length());
                DEBUG_PRINTLN(msg);
                // DEBUG_PRINTLN("\r\n");
            }
        }while(op_window);
    
    }

    static constexpr uint16_t speed_to_speed_data_msb(const real_t & speed){
        uint16_t speed_data = (uint16_t)(speed * 600);
        return BSWAP_16(speed_data);
    }

    static constexpr uint32_t position_to_position_data_msb(const real_t & position){
        uint32_t position_deg = uint32_t(position * 3600);
        return BSWAP_32(position_deg);
    }
public:
    ZdtMotor(Can & _can) : can_(_can){
        
    }

    void setTargetPosition(const real_t pos){
        struct Frame{
            uint8_t head = 0xfb;//1
            uint8_t is_neg;//2
            uint16_t spd_data;//3-4
            uint32_t pos_data;//5-8
            uint8_t is_relative; //9
            uint8_t sync = 0; //10
            uint8_t tail = 0x6b;//11
        }__packed;

        Frame frame;
        frame.is_neg = pos < 0;
        frame.spd_data = speed_to_speed_data_msb(real_t(2000) / 60);
        frame.pos_data = position_to_position_data_msb(pos);
        frame.is_relative = false;
        write_data(frame);        
    }

    void enable(const bool en = true){
        struct Frame{
            uint8_t head = 0xf3;//1
            uint8_t head2 = 0xab;
            uint8_t en;
            uint8_t sync = 0; //10
            uint8_t tail = 0x6b;//11
        }__packed;

        Frame frame;
        frame.en = en;
        write_data(frame);
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


void zdt_main(){
    auto & logger = uart1;
    logger.init(576000, CommMethod::Blocking);
    logger.setEps(4);
    
    can1.init(1_M);

    ZdtMotor motor{can1};
    
    
    delay(10);
    motor.enable();
    delay(10);
    motor.triggerCali();
    while(true){
        motor.enable();
        motor.setTargetPosition(sin(t));    
        delay(10);
        if(can1.available()) DEBUG_PRINTLN(can1.read());
        // DEBUG_PRINTLN(can1.pending(), can1.getRxErrCnt(), can1.getTxErrCnt());
    }
}