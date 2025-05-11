#pragma once

// https://www.memsic.com/Public/Uploads/uploadfile/files/20220119/MMC5983MADatasheetRevA.pdf
// https://github.com/kriswiner/MMC5983MA/blob/master/LSM6DSM_MMC5983MA_LPS22HB_Dragonfly/MMC5983MA.cpp

#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"


#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{


struct MMC5983_Collections{
    using Error = ImuError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    using RegAddress = uint8_t;
};


struct MMC5983_Regs:public MMC5983_Collections{
    struct R8_Status:public Reg8<>{
        static constexpr RegAddress address = 0x08;
        uint8_t meas_mag_done:1;
        uint8_t meas_temp_done:1;
        const uint8_t __resv1__:2 = 0;
        uint8_t otp_read_done:1;
        const uint8_t __resv2__:3 = 0;
    }DEF_R8(status_reg)

    struct R8_InternalControl0:public Reg8<>{
        static constexpr RegAddress address = 0x09;
        uint8_t tm_m:1;
        uint8_t tm_t:1;
        uint8_t int_meas_done_en:1;
        uint8_t set:1;
        uint8_t reset:1;
        uint8_t auto_sr_en:1;
        uint8_t otp_read:1;
        uint8_t :1;
    }DEF_R8(internal_control_0_reg)

    enum class BandWidth:uint8_t{
        _100Hz = 0b00,
        _200Hz = 0b01,
        _400Hz = 0b10,
        _800Hz = 0b11,
    };

    struct R8_InternalControl1:public Reg8<>{
        static constexpr RegAddress address = 0x0A;
        BandWidth bw:2;
        uint8_t x_inhibit:1;
        uint8_t yz_inhibit:2;
        const uint8_t __resv__:2 = 0;
        uint8_t sw_rst:1;
    }DEF_R8(internal_control_1_reg)

    enum class DataRate:uint8_t{
        SingleShot = 0b000,
        _1Hz = 0b001,
        _10Hz = 0b010,
        _20Hz = 0b011,
        _50Hz = 0b100,
        _100Hz = 0b101,
        _200Hz = 0b110,
        _1000Hz = 0b111,
    };

    enum class PrdSet:uint8_t{
        _1 = 0b000,
        _25 = 0b001,
        _75 = 0b010,
        _100 = 0b011,
        _250 = 0b100,
        _500 = 0b101,
        _1000 = 0b110,
        _2000 = 0b111
    };

    struct R8_InternalControl2:public Reg8<>{
        static constexpr RegAddress address = 0x0B;
        DataRate data_rate:3;
        uint8_t cmm_en:1;
        uint8_t prd_set:3;
        uint8_t en_prd_set:1 = 0;
    }DEF_R8(internal_control_2_reg)
};

class MMC5983_Phy: public MMC5983_Collections{
private:
};

class MMC5983:
    public MagnetometerIntf,
    public MMC5983_Regs{
public:
    void init();

    Option<Vector3_t<q24>> get_magnet();
private:    
    IResult<> write_reg(const uint8_t address, const uint8_t data);
    IResult<> read_reg(const uint8_t address, uint8_t & data);

};

}