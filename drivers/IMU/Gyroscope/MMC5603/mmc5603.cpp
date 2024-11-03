#include "mmc5603.hpp"

using namespace yumud::drivers;

void MMC5603::update(){
    auto & reg = x_reg;
    requestPool(reg.address_x, &reg.data_h, 6);
}

void MMC5603::reset(){
    auto & reg = ctrl0_reg;
    reg.do_reset = true;
    writeReg(reg.address, reg);
    reg.do_reset = false;
}

bool MMC5603::verify(){
    auto & reg = product_id_reg;
    readReg(reg.address, reg);
    if(reg == reg.correct_id) return true;
    return false;
}

void MMC5603::setDataRate(const uint dr){

    enableContious(true);
}

void MMC5603::enableContious(const bool en){

}


std::tuple<real_t, real_t, real_t> MMC5603::getMagnet(){
    real_t x,y,z;

    s16_to_uni(int16_t(x_reg.data_h << 8 | x_reg.data_l), x);
    s16_to_uni(int16_t(y_reg.data_h << 8 | y_reg.data_l), y);
    s16_to_uni(int16_t(z_reg.data_h << 8 | z_reg.data_l), z);

    return {x,y,z};
}
#pragma GCC diagnostic pop