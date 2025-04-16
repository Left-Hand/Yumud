#pragma once

#include "drivers/IMU/details/InvensenseIMU.hpp"

namespace ymd::drivers{

    
template<size_t N>
class RegAddr_t{
public:
    // explicit constexpr RegAddr_t(const std::bitset<N> i2c_addr):
    //     i2c_addr_(i2c_addr){}

    explicit constexpr RegAddr_t(const uint16_t i2c_addr):
        i2c_addr_(i2c_addr){;}

    uint8_t as_u8() const {return i2c_addr_.to_ulong();}
private:
    std::bitset<N> i2c_addr_;
};

class ICM45686:public Axis6{
public:
    static constexpr I2cSlaveAddr<7> DEFAULT_I2C_ADDR = I2cSlaveAddr<7>{0x68};


    using RegAddr = RegAddr_t<8>;

protected:

    struct R8_Int1Config0:public Reg8<>{
        static constexpr auto address = RegAddr{0x16};

        uint8_t en_fifo_full:1;
        uint8_t en_fifo_ths:1;
        uint8_t en_drdy:1;
        uint8_t en_aux1_drdy:1;
        uint8_t en_ap_fsync:1;
        uint8_t en_ap_agc_rdy:1;
        uint8_t en_aux1_agc_rdy:1;
        uint8_t en_reset_done:1;
    };

    struct R8_Int1Config1:public Reg8<>{
        static constexpr auto address = RegAddr{0x17};

        uint8_t en_pll_rdy:1;
        uint8_t en_wom_x:1;
        uint8_t en_wom_y:1;
        uint8_t en_wom_z:1;
        uint8_t en_i3c_protocol_err:1;
        uint8_t en_i2cm_done:1;
        uint8_t en_apex_event:1;
    };

    InvensenseSensor_Phy phy_;

    real_t lsb_acc_x64;
    real_t lsb_gyr_x256;

    
    Vector3_t<int16_t> acc_data_;
    Vector3_t<int16_t> gyr_data_;
public:
    ICM45686(hal::I2c & i2c, const I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR):phy_(i2c, i2c_addr){;}

    void init();
    
    void update();

    bool verify();

    void reset();

    Option<Vector3_t<real_t>> get_acc();
    Option<Vector3_t<real_t>> get_gyr();
};

}
