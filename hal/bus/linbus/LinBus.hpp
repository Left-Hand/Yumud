#pragma once

#include "hwspec/chip.hpp"
#include <span>

namespace ymd::hal{
using namespace chip;

// https://blog.csdn.net/Hgrjtz/article/details/106243026
class LinBus{
protected:
    USART_Def * inst;

    static constexpr uint8_t calculate_pid(const uint8_t id){
        const uint8_t P0 = (((id)^(id>>1)^(id>>2)^(id>>4))&0x01)<<6 ;
        const uint8_t P1 = ((~((id>>1)^(id>>3)^(id>>4)^(id>>5)))&0x01)<<7 ;
        return P0 | P1 | id;
    }

    static constexpr uint8_t checksum(uint8_t id , const std::span<uint8_t, 8> data){
        uint16_t sum ;

        sum = data[0];
        if(id == 0x3c)			// 如果是诊断帧，用经典校验
        {
            for(size_t t=1;t<8;t++){
                sum += data[t];
                if(sum&0xff00)
                {
                    sum&=0x00ff;
                    sum+=1;
                }
            }
            sum = ~sum;	
            
            return (uint8_t)sum ;
        }
        
        for(size_t t=1;t<8;t++){
            sum += data[t];
            if(sum&0xff00)
            {
                sum&=0x00ff;
                sum+=1;
            }
        }

        sum += calculate_pid(id);

        if(sum&0xff00){
            sum&=0x00ff;
            sum+=1;
        }

        sum = ~sum;	
        return uint8_t(sum);
    }
    void send(const uint16_t data){
        inst->send(data);
    }

public:
    void sendBreak(){
        inst->send_break();
    }

    void sendSync(){
        inst->send(0x55);
        inst->wait_transmit_complete();
    }

    void sendHead(const uint8_t id){
        sendBreak();
        sendSync();
        send(calculate_pid(id));
        inst->wait_transmit_complete();
    }

    void sendData(const std::span<uint8_t, 8> data){
        for(size_t t=0;t<8;t++){
            send(data[t]);
            inst->wait_transmit_complete();
        }
    }

    void sendAnswer(const uint8_t id, const std::span<uint8_t, 8> data){
        sendData(data);
        send(checksum(id, data));
        inst->wait_transmit_complete();
    }

    // void 
};

}