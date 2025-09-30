#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

#include "concept/analog_channel.hpp"


namespace ymd::drivers{

struct SGM58031_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b0100000 >> 1);
    enum class Error_Kind{
        WrongChipId
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class DataRate:uint8_t{
        _6_25Hz = 0,
        _12_5Hz, 
        _25Hz, 
        _50Hz, 
        _100Hz, 
        _200Hz, 
        _400Hz, 
        _800Hz,

        _7_5Hz = 0b1000, 
        _15Hz, 
        _30Hz, 
        _60Hz, 
        _120Hz, 
        _240Hz, 
        _480Hz, 
        _960Hz
    };

    enum class MUX:uint8_t{
        P0N1 = 0, P0N3, P1N3, P2N3, P0NG, P1NG, P2NG, P3NG
    };

    enum class PGA:uint8_t{
        _2_3 = 0, _1, _2, _4, _8, _16
    };

    class FS{
    public:
        enum Kind:uint8_t{
            _6_144 = 0, _4_096, _2_048, _1_024, _0_512, _0_256
        };

        constexpr FS(Kind kind):kind_(kind){;}
        constexpr real_t to_real() const{
            switch(kind_){
                case FS::_0_256:
                    return real_t(0.256);
                case FS::_0_512:
                    return real_t(0.512f);
                case FS::_1_024:
                    return real_t(1.024f);
                case FS::_2_048:
                    return real_t(2.048f);
                case FS::_4_096:
                    return real_t(4.096f);
                case FS::_6_144:
                    return real_t(6.144f);
                default:
                    __builtin_unreachable();
            }
        }

        constexpr uint8_t as_u8() const {
            return uint8_t(kind_);
        }

        constexpr auto as_pga() const {
            return std::bit_cast<PGA>(kind_);
        }
    private:
        Kind kind_;
    };


    enum class RegAddress:uint8_t{
        Conv = 0,
        Config,
        LowThr, 
        HighThr, 
        Config1,
        DeviceID,
        Trim
    };
};

struct SGM58031_Regs:public SGM58031_Prelude{
    struct R16_Config:public Reg16<>{
        static constexpr auto ADDRESS = RegAddress::Config;
        uint8_t compQue : 2;
        uint8_t compLat : 1;
        uint8_t compPol : 1;
        uint8_t compMode :1;
        uint8_t dataRate :3;
        uint8_t mode:   1;
        PGA pga:    3;
        MUX mux:    3;
        uint8_t os:     1;
    }DEF_R16(config_reg)

    struct R16_Config1:public Reg16<>{
        static constexpr auto ADDRESS = RegAddress::Config1;        
        uint8_t __resv1__    :3;
        uint8_t extRef      :1;
        uint8_t busFlex     :1;
        uint8_t __resv2__   :1;
        uint8_t burnOut     :1;
        uint8_t drSel       :1;
        uint8_t pd          :1;
        uint8_t __resv3__   :7;
    }DEF_R16(config1_reg)

    struct R16_DeviceId:public Reg16<>{
        static constexpr auto ADDRESS = RegAddress::DeviceID;
        static constexpr uint16_t KEY = 0x0080; 
        uint8_t __resv1__   :5;
        uint8_t ver         :3;
        uint8_t id          :5;
        uint8_t __resv2__   :3;
        
    }DEF_R16(device_id_reg)

    struct R16_Trim:public Reg16<>{
        static constexpr auto ADDRESS = RegAddress::Trim;
        uint16_t gn         :11;
        uint8_t __resv__    :5;
        
    }DEF_R16(trim_reg)

    struct R16_Conv:public Reg16<>{
        static constexpr auto ADDRESS = RegAddress::Conv;
        uint16_t data;
    }DEF_R16(conv_reg)

    struct R16_LowThr:public Reg16<>{
        static constexpr auto ADDRESS = RegAddress::LowThr;
        uint16_t data;
    }DEF_R16(low_thr_reg)

    struct R16_HighThr:public Reg16<>{
        static constexpr auto ADDRESS = RegAddress::HighThr;
        uint16_t data;
    }DEF_R16(high_thr_reg)
};

class SGM58031 final:public SGM58031_Regs{
public:
    explicit SGM58031(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit SGM58031(hal::I2cDrv && i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit SGM58031(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){};

    IResult<> init();
    IResult<> validate();
    IResult<bool> is_idle();
    IResult<> start_conv();
    IResult<int16_t> get_conv_data();
    IResult<real_t> get_conv_voltage();
    IResult<> enable_cont_mode(const Enable en);
    IResult<> set_datarate(const DataRate _dr);
    IResult<> set_mux(const MUX _mux);
    IResult<> set_fs(const FS fs);
    IResult<> set_fs(const real_t _fs, const real_t _vref);
    IResult<> set_trim(const real_t _trim);
    IResult<> enable_ch3_as_ref(const Enable en);
private:
    hal::I2cDrv i2c_drv_;

    real_t full_scale_ = 0;

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = i2c_drv_.write_reg(
            uint8_t(T::ADDRESS), reg.as_val(), MSB);
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    
    template<typename T>
    IResult<> read_reg(T & reg){
        if(const auto res = i2c_drv_.read_reg(
            uint8_t(T::ADDRESS), reg.as_ref(), MSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    static constexpr PGA ratio2pga(const real_t ratio){
        if(ratio >= 3){
            return PGA::_2_3;
        }else if(ratio >= 2){
            return PGA::_1;
        }else if(ratio >= 1){
            return PGA::_2;
        }else if(ratio >= real_t(0.5)){
            return PGA::_4;
        }else if(ratio >= real_t(0.25)){
            return PGA::_8;
        }else{
            return PGA::_16;
        }
    }

};

}