//这个驱动尚未完成
//这个驱动尚未测试

//LD19是一款2D激光雷达 半径12M

// https://ws.dfrobot.com.cn/Fr1DgkvHXX4PYVgKpZgQIsL_lkEn

#pragma once

#include "hal/bus/uart/uart.hpp"
#include "core/math/real.hpp"

namespace ymd::hal{
class UartDrv{
public:
    UartDrv(hal::Uart & uart):
        uart_(uart){;}

    size_t available(){
        return uart_.available();
    }
    uint16_t read(){
        char data;
        uart_.read1(data);
        return uint32_t(data);
    }
private:
    hal::Uart & uart_;
};
}

namespace ymd::drivers{



class LD19_Phy final{
public:
    LD19_Phy(hal::Uart & uart):
        uart_drv_(uart){;}
private:
    hal::UartDrv uart_drv_;
};


class LD19 final{
public:
    static constexpr uint8_t HEADER_TOKEN = 0x54;
    static constexpr uint16_t VERLEN_TOKEN = 0x2C;
    static constexpr size_t POINTS_PER_FRAME = 12;
    static constexpr size_t DEFAULT_UART_BAUD = 230400;

    struct LidarPoint{
        uint16_t distance_mm;
        uint8_t intensity;

        real_t distance() const {
            return distance_mm * 0.001_r;
        }
    };

    class LidarSpinSpeed{
    public:
        LidarSpinSpeed(uint16_t data):data_(data){;}

        real_t to_radian() const{
            static constexpr real_t LSB_TO_RADIAN = real_t(TAU / 360);
            return LSB_TO_RADIAN * data_;
        }
    private:
        uint16_t data_;
    };

    class LidarAngle{
    public:
        LidarAngle(uint16_t data):data_(data){;}

        real_t to_radian() const{
            static constexpr auto LSB_TO_RADIAN = q24(TAU / 360 * 0.01);
            return LSB_TO_RADIAN * data_;
        }

    private:
        uint16_t data_;
    };

    struct LidarFrame final{

        #pragma pack(push, 1)
        uint8_t header;
        uint8_t verlen;
        LidarSpinSpeed spin_speed;
        LidarAngle start_angle;
        std::array<LidarPoint, POINTS_PER_FRAME> points;
        LidarAngle end_angle;
        uint16_t timestamp;
        uint8_t crc8;
        #pragma pack(pop)
    public:
        uint8_t calc_crc() const;
    };
private:
    LD19_Phy phy_;
};
}