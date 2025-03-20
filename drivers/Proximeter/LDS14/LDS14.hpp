#pragma once

#include "drivers/device_defs.h"
#include "hal/bus/uart/uart.hpp"

namespace ymd::drivers{

class LDS14{
protected:
    hal::Uart & m_uart;

    static constexpr uint8_t header_token = 0x54;
    static constexpr uint16_t verlen_token = 0x55;
    static constexpr size_t point_per_pack = 12;

    #pragma pack(push, 1)

    struct LidarPoint{
        uint16_t distance;
        uint8_t intensity;
    };


    struct LidarFrame{
        uint8_t header;
        uint8_t verlen;
        uint16_t speed;
        uint16_t start_angle;
        LidarPoint points[point_per_pack];
        uint16_t end_angle;
        uint16_t timestamp;
        uint8_t crc8;

    protected:
        uint8_t calc_crc() const;

    public:
        bool is_valid() const {return crc8 == calc_crc();}
    };

    LidarFrame m_frame;

    #pragma pack(pop)

    size_t m_data_cnt;

public:
    LDS14(hal::Uart & uart):m_uart(uart){;}

    void init(){
        m_uart.init(115200);
        m_uart.set_rx_strategy(CommStrategy::Dma);
    }

    void run();
};

}