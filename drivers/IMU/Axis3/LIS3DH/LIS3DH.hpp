#pragma once

//这个驱动还在推进

#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"
#include "drivers/IMU/details/STMicroIMU.hpp"

enum FormerLatter:bool{
    Former,
    Latter
};

namespace ymd::drivers{


struct _LIS3DH_Regs{
    enum class SelfTestMode{
        Normal,
        PositiveSign,
        NegtiveSign
    };

    enum class FifoMode:uint8_t{
        Bypass      =   0b000,
        Fifo        =   0b001,
        ContinuousUntilTrigger  =   0b011,
        BypassUntilTrigger = 0b100,
        Continuous = 0b110
    };

    using RegAddress = uint8_t;

    struct R8_WhoAmI:public Reg8<>{
        scexpr RegAddress address = 0x0F; 
        scexpr uint8_t key = 0x11;
        uint8_t data;
    }DEF_R8(whoami_reg)


    struct R8_Ctrl1:public Reg8<>{
        scexpr RegAddress address = 0x20;
        uint8_t bdu:1;
        uint8_t drdy_pulse:1;
        uint8_t sw_reset:1;
        uint8_t boot:1;
        uint8_t __resv__:2;
        uint8_t if_add_inc:1;
        uint8_t norm_mod_en:1;
    }DEF_R8(ctrl1_reg)

    struct _R8_Int1Ctrl:public Reg8<>{

        uint8_t __resv__:2;
        uint8_t int_ext:1;
        uint8_t int_fth:1;
        uint8_t int_fss5:1;
        uint8_t int_ovr:1;
        uint8_t int_boot:1;
        uint8_t int_drdy:1;
    };
    struct R8_Int1Ctrl:public _R8_Int1Ctrl{
        scexpr RegAddress address = 0x21;
    }DEF_R8(int1_ctrl_reg)

    struct R8_Int2Ctrl:public _R8_Int1Ctrl{
        scexpr RegAddress address = 0x22;
    }DEF_R8(int2_ctrl_reg)

    struct R8_Ctrl4:public Reg8<>{
        scexpr RegAddress address = 0x23;
        
        uint8_t __resv__:1;
        uint8_t fifo_en:1;
        uint8_t pp_od_int1:1;
        uint8_t pp_od_int2:1;
        uint8_t selftest_mode:2;
        uint8_t dsp_bw_sel:1;
        uint8_t dsp_lp_type:1;
    }DEF_R8(ctrl4_reg)

    struct R8_Ctrl5:public Reg8<>{
        scexpr RegAddress address = 0x24;
        uint8_t fifo_spi_hs_on:1;
        uint8_t __resv__:7;
    }DEF_R8(ctrl5_reg)

    REG16_QUICK_DEF(0x25, R16_OutTemp, out_temp_reg)

    struct R8_Status:public Reg8<>{
        scexpr RegAddress address = 0x27;
        uint8_t x_drdy:1;
        uint8_t y_drdy:1;
        uint8_t z_drdy:1;
        uint8_t zyx_drdy:1;
        uint8_t x_overrun:1;
        uint8_t y_overrun:1;
        uint8_t z_overrun:1;
        uint8_t zyx_overrun:1;
    }DEF_R8(status_reg)

    REG16_QUICK_DEF(0x28, R16_OutX, out_x_reg)
    REG16_QUICK_DEF(0x2A, R16_OutY, out_y_reg)
    REG16_QUICK_DEF(0x2C, R16_OutZ, out_z_reg)

    struct R8_FifoCtrl:public Reg8<>{
        scexpr RegAddress address = 0x2E;
        uint8_t fifo_threshold:5;
        uint8_t fifo_mode:3;
    }DEF_R8(fifo_ctrl_reg)

    struct R8_FifoSrc:public Reg8<>{
        scexpr RegAddress address = 0x27;
        uint8_t fss:6;
        uint8_t ovrn:1;
        uint8_t fth:1;
    }DEF_R8(fifo_src_reg)
};

class LIS3DH:public AccelerometerIntf{
public:
    using Regs      =  _LIS3DH_Regs;
    using Phy       = StmicroImu_Phy;
    using SelfTestMode = Regs::SelfTestMode;
    using FifoMode     = Regs::FifoMode;

    using Error = ImuError;

    template<typename T>
    using Result = Result<T, Error>; 
protected:
    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0b11010010);

    Result<void> write_reg(const auto & reg);

    Result<void> read_reg(auto & reg);

    Result<void> verify_phy();

    Regs regs_ = {};

    Phy phy_;
public:

    LIS3DH(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        phy_(hal::I2cDrv{i2c, addr}){;}
    LIS3DH(const hal::I2cDrv & i2c_drv):phy_(i2c_drv){;}
    LIS3DH(hal::I2cDrv && i2c_drv):phy_(std::move(i2c_drv)){;}
    LIS3DH(const hal::SpiDrv & spi_drv):phy_(spi_drv){;}
    LIS3DH(hal::SpiDrv && spi_drv):phy_(std::move(spi_drv)){;}
    LIS3DH(hal::Spi & spi, const hal::SpiSlaveIndex index):phy_(hal::SpiDrv{spi, index}){;}

    Result<void> init();
    Result<void> update();
    Result<void> validate();
    Result<void> reset();

    Result<void> clear_flag();

    Option<Vector3_t<real_t>> get_acc();

    Result<void> sleep()
        {auto & reg = regs_.ctrl1_reg; reg.norm_mod_en = false; return write_reg(reg);}
    Result<void> wakeup()
        {auto & reg = regs_.ctrl1_reg; reg.norm_mod_en = true; return write_reg(reg);}

    Result<void> enable_drdy_pulse(const Enable en)
        {auto & reg = regs_.ctrl1_reg; reg.drdy_pulse = bool(en); return write_reg(reg);}

    Result<void> block_when_update(const Enable en)
        {auto & reg = regs_.ctrl1_reg; reg.drdy_pulse = bool(en); return write_reg(reg);}


    enum class FilteringStrategy:uint8_t{ FIR, IIR};
    enum class Bandwidth:uint8_t{_440Hz, _235Hz};
    
    Result<void> set_filtering_strategy(const FilteringStrategy st)
        {auto & reg = regs_.ctrl4_reg; reg.dsp_lp_type = bool(st); return write_reg(reg);}

    Result<void> set_bandwidth(const Bandwidth bw)
        {auto & reg = regs_.ctrl4_reg; reg.dsp_bw_sel = bool(bw); return write_reg(reg);}

    Result<void> set_selftest_mode(const SelfTestMode mode)
        {auto & reg = regs_.ctrl4_reg; reg.selftest_mode = uint8_t(mode); return write_reg(reg);}

    Result<void> set_int1_pp_or_od(const FormerLatter sel)
        {auto & reg = regs_.ctrl4_reg; reg.pp_od_int1 = sel; return write_reg(reg);}

    Result<void> set_int2_pp_or_od(const FormerLatter sel)
        {auto & reg = regs_.ctrl4_reg; reg.pp_od_int2 = sel; return write_reg(reg);}
        
    Result<void> enable_fifo(const Enable en)
        {auto & reg = regs_.ctrl4_reg; reg.fifo_en = bool(en); return write_reg(reg);}

    Result<void> enable_spi_hw(const Enable en)
        {auto & reg = regs_.ctrl5_reg; reg.fifo_spi_hs_on = bool(en); return write_reg(reg);}


};

}

namespace ymd::drivers{

LIS3DH::Result<void> LIS3DH::write_reg(const auto & reg){
    return LIS3DH::Result<void>(phy_.write_reg(reg.address, reg.as_val()));
}

LIS3DH::Result<void> LIS3DH::read_reg(auto & reg){
    return LIS3DH::Result<void>(phy_.read_reg(reg.address, reg.as_ref()));
}

LIS3DH::Result<void> LIS3DH::verify_phy(){
    return LIS3DH::Result<void>(phy_.validate());
}

}