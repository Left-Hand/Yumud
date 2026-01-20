#pragma once

//这个驱动还在开发中

#include "core/io/regs.hpp"

#include "drivers/IMU/IMU.hpp"
#include "drivers/IMU/details/STMicroIMU.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"


namespace ymd::drivers{


struct LIS2DW12_Prelude{
    enum class [[nodiscard]] DPS:uint8_t{
        _250, _500, _1000, _2000
    };

    enum class [[nodiscard]] G:uint8_t{
        _2, _4, _8, _16
    };

    enum class [[nodiscard]] AccOdr:uint8_t{
        _25_32 = 0b0001,
        _25_16,
        _25_8,
        _25_4,
        _25_2,
        _25,
        
        _50,
        _100,
        _200,
        _400,
        _800,
        _1600
    };

    enum class [[nodiscard]] GyrOdr:uint8_t{
        _25 = 0b0110,
        
        _50,
        _100,
        _200,
        _400,
        _800,
        _1600,
        _3200
    };
    
    enum class [[nodiscard]] AccFs:uint8_t{
        _2G     =   0b0011,
        _4G     =   0b0101,
        _8G     =   0b1000,
        _16G    =   0b1100
    };

    enum class [[nodiscard]] GyrFs:uint8_t{
        _2000deg = 0b0000,
        _1000deg,
        _500deg,
        _250deg,
        _125deg
    };

    enum class [[nodiscard]] Command:uint8_t{
        START_FOC = 0x04,
        ACC_SET_PMU = 0b0001'0000,
        GYR_SET_PMU = 0b0001'0100,
        MAG_SET_PMU = 0b0001'1000,
        FIFO_FLUSH = 0xB0,
        RESET_INTERRUPT =0xB1,
        SOFT_RESET = 0xB1,
        STEP_CNT_CLR = 0xB2
    };

    enum class [[nodiscard]] PmuType{
        Acc,
        Gyr,
        Mag
    };

    enum class [[nodiscard]] PmuMode{
        Suspend = 0b00,
        Normal = 0b01,
        LowPower = 0b10,
        FastSetup =  0b11
    };

    using Error = ImuError;

    template<typename T = void>
    using IResult = Result<T, Error>;
    

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b1101001);
};


struct LIS2DW12_Regs:public LIS2DW12_Prelude{
    using RegAddr = uint8_t;
    struct TempOutReg:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x0D; uint8_t bits;};
    struct WhoAmIReg:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x0F; uint8_t bits;};
    struct Ctrl1Reg:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x20;
        uint8_t lp_mode:2;
        uint8_t mode:2;
        uint8_t odr:4;    
    };

    struct Ctrl2Reg:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x21;
        uint8_t sim:1;
        uint8_t i2c_disable:1;
        uint8_t if_add_inc:1;
        uint8_t bdu:1;
        uint8_t cs_pu_disc:1;
        uint8_t :1;
        uint8_t soft_reset:1;
        uint8_t boot:1;
    };

    struct Ctrl3Reg:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x22;
        uint8_t slp_mode_1:1;
        uint8_t slp_mode_sel:1;
        uint8_t :1;
        uint8_t h_active:1;
        uint8_t lir:1;
        uint8_t pp_od:1;
        uint8_t selftest_mode:2;
    };

    struct Ctrl4Reg:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x23;
        uint8_t int1_drdy:1;
        uint8_t int1_fth:1;
        uint8_t int1_diff5:1;
        uint8_t int1_tap:1;
        uint8_t int1_ff:1;
        uint8_t int1_wu:1;
        uint8_t int1_single_tap:1;
        uint8_t int1_6d:1;
    };

    struct Ctrl5Reg:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x24;
        uint8_t int2_drdy:1;
        uint8_t int2_fth:1;
        uint8_t int2_diff5:1;
        uint8_t int2_ovr:1;
        uint8_t int2_drdy_t:1;
        uint8_t int2_boot:1;
        uint8_t int2_sleep_chg:1;
        uint8_t int2_sleep_state:1;
    };

    struct Ctrl6Reg:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x25;
        uint8_t :2;
        uint8_t low_noise:1;
        uint8_t fds:1;
        uint8_t fs:2;
        uint8_t bw_filt:2;
    };

    struct TempOut8Reg:public Reg16<>{static constexpr RegAddr ADDRESS = 0x26; uint8_t bits;};

    struct StatusReg:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x27;
        uint8_t drdy:1;
        uint8_t ff_ia:1;
        uint8_t _6d_ia:1;
        uint8_t single_tap:1;
        uint8_t double_tap:1;
        uint8_t sleep_state:1;
        uint8_t wu_ia:1;
        uint8_t fifo_ths:1;
    };

    struct OutputXReg:public Reg16<>{static constexpr RegAddr ADDRESS = 0x28; int16_t bits;};
    struct OutputYReg:public Reg16<>{static constexpr RegAddr ADDRESS = 0x2A; int16_t bits;};
    struct OutputZReg:public Reg16<>{static constexpr RegAddr ADDRESS = 0x2C; int16_t bits;};

    struct FifoCtrlReg:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x2E;
        uint8_t fth:5;
        uint8_t fmode:3;
    };

    struct FifoSamplesReg:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x2F;
        uint8_t diff0:6;
        uint8_t fifo_ovr:1;
        uint8_t fifo_fth:1;
    };
};


class LIS2DW12:
    public LIS2DW12_Prelude{
public:
    explicit LIS2DW12(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR):
        transport_(hal::I2cDrv{i2c, i2c_addr}){;}
    explicit LIS2DW12(const hal::I2cDrv & i2c_drv):
        transport_(i2c_drv){;}
    explicit LIS2DW12(hal::I2cDrv && i2c_drv):
        transport_(std::move(i2c_drv)){;}
    explicit LIS2DW12(const hal::SpiDrv & spi_drv):
        transport_(spi_drv){;}
    explicit LIS2DW12(hal::SpiDrv && spi_drv):
        transport_(std::move(spi_drv)){;}
    explicit LIS2DW12(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        transport_(hal::SpiDrv{spi, rank}){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<> set_acc_odr(const AccOdr odr);
    [[nodiscard]] IResult<> set_acc_fs(const AccFs range);
    
    [[nodiscard]] IResult<> set_pmu_mode(const PmuType pum, const PmuMode mode);
    [[nodiscard]] IResult<Vec3<iq24>> read_acc();

private:
    using Phy = StmicroImu_Transport;
    Phy transport_;

    iq16 acc_scale = 0;
    iq16 gyr_scale = 0;

    [[nodiscard]] IResult<> write_reg(uint8_t reg_addr, uint8_t reg_val){
        if(const auto res = transport_.write_reg(reg_addr, reg_val);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_reg(uint8_t reg_addr, uint8_t & reg_val){
        if(const auto res = transport_.read_reg(reg_addr, reg_val);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    // [[nodiscard]] IResult<> read_burst(uint8_t reg_addr, std::span<uint8_t> pbuf){
    //     if(const auto res = transport_.read_burst(reg_addr, pbuf, std::endian::little);
    //         res.is_err()) return Err(res.unwrap_err());
    //     return Ok();
    // }

    [[nodiscard]] static constexpr iq24 calc_gyr_scaler(const GyrFs fs){
        switch(fs){
            case GyrFs::_2000deg  :      return iq24(DEG2RAD_RATIO) * 2 * 2000;
            case GyrFs::_1000deg  :      return iq24(DEG2RAD_RATIO) * 2 * 1000;
            case GyrFs::_500deg   :      return iq24(DEG2RAD_RATIO) * 2 * 500;
            case GyrFs::_250deg   :      return iq24(DEG2RAD_RATIO) * 2 * 250;
            case GyrFs::_125deg   :      return iq24(DEG2RAD_RATIO) * 2 * 125;
        }
        __builtin_unreachable();
    }

    [[nodiscard]] static constexpr iq24 calc_acc_scaler(const AccFs fs){
        switch(fs){
            case AccFs::_16G    :       return GRAVITY_ACC<iq24> * 32;
            case AccFs::_8G     :       return GRAVITY_ACC<iq24> * 16;
            case AccFs::_4G     :       return GRAVITY_ACC<iq24> * 8;
            case AccFs::_2G     :       return GRAVITY_ACC<iq24> * 4;
        }
        __builtin_unreachable();
    }

};

}
