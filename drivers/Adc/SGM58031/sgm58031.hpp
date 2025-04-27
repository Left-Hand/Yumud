#pragma once

#include "core/io/regs.hpp"

#include "concept/analog_channel.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"


namespace ymd::drivers{

class SGM58031_Error{
public:
    enum Kind:uint8_t {
        Unspecified = 0xff
    };

    constexpr SGM58031_Error(Kind kind):kind_(kind){}
    constexpr bool operator ==(const SGM58031_Error other) const {
        return kind_ == other.kind_;
    }

    constexpr bool operator ==(const Kind kind) const {
        return kind_ == kind;
    }
private:
    Kind kind_;
};

class SGM58031{
public:
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
protected:
    hal::I2cDrv i2c_drv_;

    struct ConfigReg:public Reg16<>{
        
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
        
        uint8_t __resv1__   :5;
        uint8_t ver         :3;
        uint8_t id          :5;
        uint8_t __resv2__   :3;
        
    };

    struct TrimReg:public Reg16<>{
        
        uint16_t gn         :11;
        uint8_t __resv__    :5;
        
    };

    struct ConvReg:public Reg16<>{
        uint16_t data;
        
    };

    struct LowThrReg:public Reg16<>{
        uint16_t data;
        
    };

    struct HighThrReg:public Reg16<>{
        uint16_t data;
        
    };

    ConvReg convReg;
    ConfigReg configReg;
    LowThrReg lowThrReg;
    HighThrReg highThrReg;
    Config1Reg config1Reg;
    DeviceIdReg deviceIdReg;
    TrimReg trimReg;

    enum class RegAddress:uint8_t{
        Conv = 0,
        Config,LowThr, HighThr, Config1, DeviceID,Trim
    };

    real_t fullScale;
    auto write_reg(const RegAddress addr, const uint16_t data){
        return i2c_drv_.write_reg(uint8_t(addr), data, MSB);
    }

    auto read_reg(const RegAddress addr, uint16_t & data){
        return i2c_drv_.read_reg(uint8_t(addr), data, MSB);
    }

public:
    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0b0100000);

    SGM58031(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    SGM58031(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    SGM58031(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){};

    void init();

    void get_device_id(){
        read_reg(RegAddress::DeviceID, deviceIdReg);
    }

    bool is_idle(){
        read_reg(RegAddress::Config, configReg);
        return configReg.os;
    }

    void start_conv(){
        config1Reg.pd = true;
        write_reg(RegAddress::Config1, config1Reg);
        configReg.os = true;
        write_reg(RegAddress::Config, configReg);
    }

    int16_t get_conv_data(){
        read_reg(RegAddress::Conv, convReg);
        return *(int16_t *)&convReg;
    }

    real_t get_conv_voltage(){
        int16_t data = get_conv_data();
        real_t uni = real_t(data) / (1 << 15);
        return uni * fullScale;
    }
    void set_cont_mode(const bool continuous){
        configReg.mode = continuous;
        write_reg(RegAddress::Config, configReg);
    }

    void set_datarate(const DataRate _dr);

    void set_mux(const MUX _mux){
        configReg.mux = (uint8_t)_mux;
        write_reg(RegAddress::Config, configReg);
    }

    void set_fs(const FS fs);

    void set_fs(const real_t _fs, const real_t _vref);

    void set_trim(const real_t _trim);
    void enable_ch3_as_ref(bool yes){
        config1Reg.extRef = yes;
        write_reg(RegAddress::Config1, config1Reg);
    }
};

}