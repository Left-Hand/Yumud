#include "bh1750.hpp"

using namespace yumud::drivers;

void BH1750::changeMeasureTime(const uint16_t ms){
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

void BH1750::startConv(){
    if(continuous){
        sendCommand(0x10 | (uint8_t)currentMode);
    }else{
        sendCommand(0x20 | (uint8_t)currentMode);
    }
}

void BH1750::sendCommand(const uint8_t cmd){
    // i2c_drv.write(cmd);
}


int BH1750::getLx(){
    uint8_t data[2] = {0};
    // i2c_drv.read(data, 2);
    return lsb * (int)((data[0] << 8) | data[1]);
}