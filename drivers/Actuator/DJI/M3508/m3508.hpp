#pragma once

#include "hal/bus/can/can.hpp"

class M3508Port{
    #define M3508_CHECK_INDEX if(index > size or index == 0) CREATE_FAULT; 
    scexpr size_t max_size = 8;
    
    Can & can;
    size_t size = 4;

    using CurrData_t = int16_t;
    using AngleData_t = uint16_t;
    using SpeedData_t = int16_t;
    
    struct TxData{
        int16_t curr_data[4];  
    };

    struct RxData{
        uint16_t angle_8192;
        int16_t curr_data;
        int16_t speed_rpm;
        uint8_t temp;
        uint8_t __resv__;
    };

    static constexpr int16_t curr_to_currdata(const real_t curr){
        return int(curr * 16384 / 20);
    }

    static constexpr real_t currdata_to_curr(const CurrData_t currdata){
        return currdata * 20 / 16384;
    };

    
    class M3508{
    protected:
        M3508Port & port;
        const size_t index;
        
        real_t lap_position = 0;
        real_t curr = 0;
        real_t speed = 0;
        real_t temperature = 0;
        
        friend class M3508Port;
        M3508(M3508Port & _port, const size_t _index):
            port(_port), index(_index){;}

        auto & operator = (const M3508 & other) = delete;
        auto & operator = (M3508 && other) = delete;
    public:

        void tick();
        void setTargetCurrent(const real_t curr){port.setTargetCurrent(curr, index);}
        void setTargetSpeed(const real_t spd);
        void setTargetPosition(const real_t pos);
        real_t getCurrent() const {return curr;}
        real_t getSpeed() const {return speed;}
        real_t getTemperature() const {return temperature;}
    };

    std::array<M3508, max_size> inst_ = {
        M3508(*this, 1),
        M3508(*this, 2),
        M3508(*this, 3),
        M3508(*this, 4),
        M3508(*this, 5),
        M3508(*this, 6),
        M3508(*this, 7),
        M3508(*this, 8),
    };
    
    union{
        uint16_t curr_cache[8];
        struct{
            TxData tx_datas[2];
        };
    };
    
    void setTargetCurrent(const real_t curr, const size_t index){
        M3508_CHECK_INDEX
        curr_cache[index] = curr_to_currdata(curr);
    }

    void updateInst(const CanMsg & msg, const size_t index){
        M3508_CHECK_INDEX
        auto rx_data = RxData(msg);
        auto & inst = inst_[index];

        inst.lap_position = rx_data.angle_8192 / 8192;
        inst.curr = currdata_to_curr(rx_data.curr_data);
        inst.speed = rx_data.speed_rpm / 60;
        inst.temperature = rx_data.temp;

        inst.tick();
    }
public:
    M3508Port(Can & _can):can(_can){}

    void tick(){
        while(can.available()){
            const auto & msg = can.front();
            if(msg.id() > 0x200 and msg.id() <= 0x208){
                can.read();
                size_t index = msg.id() - 0x200 - 1;
                M3508_CHECK_INDEX
                updateInst(msg, index);
            }
        }

        if(can.pending() < 2){
            if(size > 0){
                can.write(CanMsg{0x200, tx_datas[0]});
            }

            if(size > 4){
                can.write(CanMsg{0x1FF, tx_datas[1]});
            }
        }
    }

    void setSize(const size_t _size){
        if(size > max_size) CREATE_FAULT
        size = _size;
    }

    M3508 & operator[](const size_t index){
        M3508_CHECK_INDEX
        return inst_[index - 1];
    }

    auto & inst(){
        return inst_;
    }

    #undef M3508_CHECK_INDEX
};