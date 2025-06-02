#pragma once


#include "hal/bus/i2c/i2cdrv.hpp"
#include "core/math/fraction.hpp"

namespace ymd::drivers{

class BH1750{
public:
    enum class Mode:uint8_t{
        HMode = 0,
        HMode2 = 1,
        LMode = 3
    };

protected:
    hal::I2cDrv i2c_drv_;

    enum Command:uint8_t{
        PowerDown = 0,
        PowerOn = 1,
        Reset = 7,
        ChangeMeasureTimeH = 0x40,
        ChangeMeasureTimeL = 0x60,
    };

    Fraction lsb = {
        .numerator = 69,
        .denominator = 69
    };

    Mode currentMode = Mode::LMode;
    bool continuous = false;

    void send_command(const uint8_t cmd);

public:
    BH1750(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    BH1750(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}

    // void power_on(){
    //     send_command(Command::PowerOn);
    //     MIN
    // }

    void power_down(){
        send_command(Command::PowerDown);
    }

    void reset(){
        send_command(Command::Reset);
    }

    void set_mode(const Mode & mode){
        currentMode = mode;
    }

    void enable_continuous(const Enable en = EN){
        continuous = en == EN;
    }

    void start_conv();

    void change_measure_time(const uint16_t ms);

    int get_lx();
};

};