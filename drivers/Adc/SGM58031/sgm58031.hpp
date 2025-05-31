#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

#include "concept/analog_channel.hpp"


namespace ymd::drivers{

struct SGM58031_Collections{
    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0b0100000);
    enum class Error_Kind{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class DataRate:uint8_t{
        DR6_25 = 0,DR12_5, DR25, DR50, DR100, DR200, DR400, DR800,
        DR7_5 = 0b1000, DR15, DR30, DR60, DR120, DR240, DR480, DR960
    };

    enum class MUX:uint8_t{
        P0N1 = 0, P0N3, P1N3, P2N3, P0NG, P1NG, P2NG, P3NG
    };

    class FS{
    public:
        enum Kind{
            FS6_144 = 0, FS4_096, FS2_048, FS1_024, FS0_512, FS0_256
        };

        constexpr FS(Kind kind):kind_(kind){;}
        constexpr real_t to_real() const{
            switch(kind_){
                case FS::FS0_256:
                    return real_t(0.256);
                case FS::FS0_512:
                    return real_t(0.512f);
                case FS::FS1_024:
                    return real_t(1.024f);
                case FS::FS2_048:
                    return real_t(2.048f);
                case FS::FS4_096:
                    return real_t(4.096f);
                case FS::FS6_144:
                    return real_t(6.144f);
                default:
                    __builtin_unreachable();
            }
        }

        constexpr uint8_t as_u8() const {
            return uint8_t(kind_);
        }
    private:
        Kind kind_;
    };

    enum class PGA:uint8_t{
        RT2_3 = 0, RT1, RT2, RT4, RT8, RT16
    };

    enum class RegAddress:uint8_t{
        Conv = 0,
        Config,LowThr, HighThr, Config1, DeviceID,Trim
    };
};

struct SGM58031_Regs:public SGM58031_Collections{
    struct ConfigReg:public Reg16<>{
        static constexpr auto address = RegAddress::Config;
        uint8_t compQue : 2;
        uint8_t compLat : 1;
        uint8_t compPol : 1;
        uint8_t compMode :1;
        uint8_t dataRate :3;
        uint8_t mode:   1;
        uint8_t pga:    3;
        uint8_t mux:    3;
        uint8_t os:     1;
        
    };

    struct Config1Reg:public Reg16<>{
        static constexpr auto address = RegAddress::Config1;        
        uint8_t __resv1__    :3;
        uint8_t extRef      :1;
        uint8_t busFlex     :1;
        uint8_t __resv2__   :1;
        uint8_t burnOut     :1;
        uint8_t drSel       :1;
        uint8_t pd          :1;
        uint8_t __resv3__   :7;
        
    };

    struct DeviceIdReg:public Reg16<>{
        static constexpr auto address = RegAddress::DeviceID;

        uint8_t __resv1__   :5;
        uint8_t ver         :3;
        uint8_t id          :5;
        uint8_t __resv2__   :3;
        
    };

    struct TrimReg:public Reg16<>{
        static constexpr auto address = RegAddress::Trim;
        uint16_t gn         :11;
        uint8_t __resv__    :5;
        
    };

    struct ConvReg:public Reg16<>{
        static constexpr auto address = RegAddress::Conv;
        uint16_t data;
    };

    struct LowThrReg:public Reg16<>{
        static constexpr auto address = RegAddress::LowThr;
        uint16_t data;
    };

    struct HighThrReg:public Reg16<>{
        static constexpr auto address = RegAddress::HighThr;
        uint16_t data;
    };

    ConvReg conv_reg;
    ConfigReg config_reg;
    LowThrReg low_thr_reg;
    HighThrReg high_thr_reg;
    Config1Reg config1_reg;
    DeviceIdReg device_id_reg;
    TrimReg trim_reg;
};

class SGM58031 final:public SGM58031_Regs{
public:


    SGM58031(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    SGM58031(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    SGM58031(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){};

    IResult<> init();

    IResult<> get_device_id(){
        return read_reg(device_id_reg);
    }

    IResult<bool> is_idle(){
        if(const auto res = read_reg(config_reg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok(bool(config_reg.os));
    }

    IResult<> start_conv(){
        {
            auto reg = RegCopy(config1_reg);
            reg.pd = true;
            if(const auto res = write_reg(reg);
                res.is_err()) return Err(res.unwrap_err());
        }

        {
            auto reg = RegCopy(config_reg);
            reg.os = true;
            return write_reg(reg);
        }
    }

    IResult<int16_t> get_conv_data(){
        if(const auto res = read_reg(conv_reg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok(conv_reg.as_val());
    }

    IResult<real_t> get_conv_voltage(){
        return get_conv_data()
            .map([&](const int16_t x) -> real_t{
                return (x * fullScale) >> 15; 
            });
    }
    IResult<> set_cont_mode(const bool continuous){
        auto reg = RegCopy(config_reg);
        reg.mode = continuous;
        return write_reg(reg);
    }

    IResult<> set_datarate(const DataRate _dr);

    IResult<> set_mux(const MUX _mux){
        auto reg = RegCopy(config_reg);
        reg.mux = (uint8_t)_mux;
        return write_reg(reg);
    }

    IResult<> set_fs(const FS fs);

    IResult<> set_fs(const real_t _fs, const real_t _vref);

    IResult<> set_trim(const real_t _trim);
    IResult<> enable_ch3_as_ref(const Enable en){
        auto reg = RegCopy(config1_reg);
        reg.extRef = en == EN;
        return write_reg(reg);
    }

private:
    hal::I2cDrv i2c_drv_;

    real_t fullScale;

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = i2c_drv_.write_reg(
            uint8_t(reg.address), reg.as_val(), MSB);
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    
    template<typename T>
    IResult<> read_reg(T & reg){
        if(const auto res = i2c_drv_.read_reg(
            uint8_t(reg.address), reg.as_ref(), MSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

};

}