#include "bh1750.hpp"

using namespace ymd::drivers;

void BH1750::change_measure_time(const uint16_t ms){
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

    send_command((uint8_t)(Command::ChangeMeasureTimeH) | (x >> 5));
    send_command((uint8_t)(Command::ChangeMeasureTimeL) | (x & 31));
}

void BH1750::start_conv(){
    if(continuous){
        send_command(0x10 | (uint8_t)currentMode);
    }else{
        send_command(0x20 | (uint8_t)currentMode);
    }
}

void BH1750::send_command(const uint8_t cmd){
    // i2c_drv.write(cmd);
}


int BH1750::get_lx(){
    uint8_t data[2] = {0};
    // i2c_drv.read(data, 2);
    return lsb * (int)((data[0] << 8) | data[1]);
}