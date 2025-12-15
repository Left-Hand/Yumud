#pragma once

#include "core/io/regs.hpp"
#include "core/math/real.hpp"


namespace ymd::drivers{
class TM8211{
// private:
public:
    I2sDrv i2s_drv_;

    int16_t left_data;
    int16_t right_data;
    uint32_t distort_mask = 0xFFFFFFFF;
    real_t voltH;
    real_t voltL;
    real_t voltComm;
    real_t voltDiff_2;

    static constexpr int16_t voltage_to_data(real_t volt){
        volt = CLAMP(volt, voltL, voltH);
        real_t k = ((volt - voltComm) / voltDiff_2);
        return (int16_t)(k * 0x7FFF);
    }

    static constexpr int16_t duty_to_data(real_t duty){
        duty = CLAMP(duty, real_t(0), real_t(1));
        real_t k = (duty - 0.5) / 0.5;
        return (int16_t)(k * 0x7FFF);
    }

    void write(const uint32_t data){
        i2s_drv_.write(data);
    }



public:
    TM8211(I2sDrv & i2c_drv):i2s_drv_(i2c_drv){
        setRail(real_t(3.3 * 0.25f), real_t(3.3 * 0.75f)); 
    }
    void set_ch_data(const uint8_t index,const uint16_t data){
        if(index) right_data = data;
        else left_data = data;

        write(((left_data << 16) | right_data) & distort_mask);
    }
    void set_rail(const real_t _voltL, const real_t _voltH){
        voltL = _voltL;
        voltH = _voltH;
        voltComm = (voltL + voltH) / 2;
        voltDiff_2 = (voltH - voltL) / 2;
    }



    void set_ch_voltage(const uint8_t index, const real_t volt){
        setChData(index, VoltageToData(volt));
    }

    void set_distort(uint8_t level){
        uint16_t mask_16 = ~((1 << level) - 1);
        distort_mask = (mask_16 << 16) | mask_16;
    }
    void set_voltage(const real_t left_volt, const real_t right_volt){
        left_data = VoltageToData(left_volt);
        right_data = VoltageToData(right_volt);

        write(((left_data << 16) | right_data) & distort_mask);
    }

    // void setChDuty(const real_t dutycycle{

    // }
};

}
