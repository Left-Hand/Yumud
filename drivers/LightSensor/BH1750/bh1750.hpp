#pragma once

#include "../drivers/device_defs.h"

namespace ymd::drivers{

class BH1750{
public:
    enum class Mode:uint8_t{
        HMode = 0,
        HMode2 = 1,
        LMode = 3
    };

protected:
    I2cDrv i2c_drv_;

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
    void sendCommand(const uint8_t cmd);

public:
    BH1750(const I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    BH1750(I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}

    void powerOn(){
        sendCommand(Command::PowerOn);
    }

    void powerDown(){
        sendCommand(Command::PowerDown);
    }

    void reset(){
        sendCommand(Command::Reset);
    }

    void setMode(const Mode & mode){
        currentMode = mode;
    }

    void enableContinuous(const bool enabled = true){
        continuous = enabled;
    }

    void startConv();

    void changeMeasureTime(const uint16_t ms);

    int getLx();
};

};