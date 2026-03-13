#pragma once

#include "ymd::ral/chip.hpp"
#include <span>

namespace ymd::hal{
using namespace chip;

// https://blog.csdn.net/Hgrjtz/article/details/106243026
class LinBus{
public:
    void send_break(){
        inst->send_break();
    }

    void send_sync(){
        inst->send(0x55);
        inst->wait_transmit_complete();
    }

    void send_head(const uint8_t id){
        sendBreak();
        sendSync();
        send(calculate_pid(id));
        inst->wait_transmit_complete();
    }

    void send_frame(const std::span<uint8_t, 8> bytes){
        for(size_t t=0;t<8;t++){
            send(bytes[t]);
            inst->wait_transmit_complete();
        }
    }

    void send_answer(const uint8_t id, const std::span<uint8_t, 8> bytes){
        send_frame(bytes);
        send(checksum(id, bytes));
        inst->wait_transmit_complete();
    }

private:
    USART_Def * inst;

    [[nodiscard]] static constexpr uint8_t calculate_pid(const uint8_t id){
        const uint8_t P0 = (((id)^(id>>1)^(id>>2)^(id>>4))&0x01)<<6 ;
        const uint8_t P1 = ((~((id>>1)^(id>>3)^(id>>4)^(id>>5)))&0x01)<<7 ;
        return P0 | P1 | id;
    }

    [[nodiscard]] static constexpr uint8_t 
    checksum(uint8_t id , const std::span<uint8_t, 8> bytes){
        uint16_t sum = static_cast<uint16_t>(bytes[0]);
        if(id == 0x3c)			// 如果是诊断帧，用经典校验
        {
            for(size_t t=1;t<8;t++){
                sum += bytes[t];
                if(sum&0xff00)
                {
                    sum&=0x00ff;
                    sum+=1;
                }
            }
            sum = ~sum;	
            
            return (uint8_t)sum ;
        }
        
        for(size_t t = 1; t < 8; t++){
            sum += bytes[t];
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
    void send(const uint16_t bytes){
        inst->send(bytes);
    }
};

}