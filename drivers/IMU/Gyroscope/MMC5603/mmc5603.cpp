#include "mmc5603.hpp"

using namespace ymd::drivers;

#define WRITE_REG(reg)     writeReg(reg.address, reg);
#define READ_REG(reg)     readReg(reg.address, reg);

void MMC5603::update(){
    auto & reg = x_reg;
    requestPool(reg.address_x, &reg.data_h, 6);
}

void MMC5603::reset(){
    auto & reg = ctrl0_reg;
    reg.do_reset = true;
    WRITE_REG(reg)
    reg.do_reset = false;
}

bool MMC5603::verify(){
    auto & reg = product_id_reg;
    READ_REG(reg)
    if(reg != reg.correct_id) return false;

    setSelfTestThreshlds(0,0,0);//TODO change

    return true;
}

void MMC5603::setDataRate(const DataRate dr){
    {
        auto & reg = odr_reg;
        reg = uint8_t(dr);
        WRITE_REG(reg);
    }

    {
        auto & reg = ctrl2_reg;
        if(reg.high_pwr != 1){
            reg.high_pwr = 1;
            WRITE_REG(reg);
        }
    }

    enableContious(true);
}


void MMC5603::setBandWidth(const BandWidth bw){
    auto & reg = ctrl1_reg;
    reg.bandwidth = uint8_t(bw);
    WRITE_REG(reg)
}

void MMC5603::enableContious(const bool en){
    auto & reg = ctrl2_reg;
    reg.cont_en = en;
    WRITE_REG(reg)
}


std::tuple<real_t, real_t, real_t> MMC5603::getMagnet(){
    return {
        s16_to_uni(int16_t(x_reg)),
        s16_to_uni(int16_t(y_reg)),
        s16_to_uni(int16_t(z_reg))
    };
}

void MMC5603::setSelfTestThreshlds(uint8_t x, uint8_t y, uint8_t z){
    x_st_reg = x;
    y_st_reg = y;
    z_st_reg = z;

    WRITE_REG(x_st_reg)
    WRITE_REG(y_st_reg)
    WRITE_REG(z_st_reg)
}

void MMC5603::inhibitChannels(bool x, bool y, bool z){
    auto & reg = ctrl1_reg;

    reg.x_inhibit = x;
    reg.y_inhibit = y;
    reg.z_inhibit = z;

    WRITE_REG(reg)
}