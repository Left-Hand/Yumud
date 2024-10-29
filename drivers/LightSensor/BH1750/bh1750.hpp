#pragma once

#include "../drivers/device_defs.h"

class BH1750{
public:
    enum class Mode:uint8_t{
        HMode = 0,
        HMode2 = 1,
        LMode = 3
    };

protected:
    I2cDrv bus_drv;

    enum class Command{
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

    void sendCommand(const Command & cmd){
        bus_drv.write((uint8_t)cmd);
    }

    void sendCommand(const uint8_t cmd){
        bus_drv.write(cmd);
    }

public:
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

    void startConv(){
        if(continuous){
            sendCommand(0x10 | (uint8_t)currentMode);
        }else{
            sendCommand(0x20 | (uint8_t)currentMode);
        }
    }

    void changeMeasureTime(const uint16_t ms){
        uint8_t x;
        if(currentMode == Mode::HMode || currentMode == Mode::HMode2){
            x = CLAMP(ms * 69 / 120, 31, 254);
            lsb.numerator = 5 * 69;
            lsb.denominator = 6 * x;
            if(currentMode == Mode::HMode2){
                lsb.denominator *= 2;
            }
        }else{
            x = CLAMP(ms * 69 / 16, 31, 254);
            lsb.numerator = 5 * 69 * 4;
            lsb.denominator = 6 * x;
        }

        sendCommand((uint8_t)(Command::ChangeMeasureTimeH) | (x >> 5));
        sendCommand((uint8_t)(Command::ChangeMeasureTimeL) | (x & 31));
    }

    int getLx(){
        uint8_t data[2];
        bus_drv.read(data, 2);
        return lsb * (int)((data[0] << 8) | data[1]);
    }
};

