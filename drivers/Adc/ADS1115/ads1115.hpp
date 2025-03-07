#pragma once

#include "drivers/device_defs.h"
#include "sys/utils/rustlike/Result.hpp"
#include "sys/utils/rustlike/Optional.hpp"


namespace ymd::drivers{



class ADS111X{
public:

    using DeviceResult = Result<void, BusError>;
    __inline DeviceResult make_result(const BusError res){
        if(res.ok()) return Ok();
        else return Err(res); 
    }


    enum class Package:uint8_t{
        ADS1113,
        ADS1114,
        ADS1115
    };

    enum class DataRate:uint8_t{
        _8 = 0b000,
        _16, _32, _64, _128, _250, _475, _860
    };

    enum class MUX:uint8_t{
        P0N1 = 0, P0N3, P1N3, P2N3, P0NG, P1NG, P2NG, P3NG
    };

    enum class PGA:uint8_t{
        _6_144V = 0, _4_096V, _2_048V, _1_024V, _0_512V, _0_256V
    };

protected:
    hal::I2cDrv i2c_drv_;

    using RegAddress = uint8_t;

    struct ConversionReg:public Reg16<>{
        scexpr RegAddress address = 0b00; 
        int16_t data;
    };

    struct ConfigReg:public Reg16<>{
        scexpr RegAddress address = 0b01; 
        uint16_t comp_que:2;
        uint16_t comp_latch:1;
        uint16_t comp_pol:1;

        uint16_t comp_mode_is_window:1;
        uint16_t data_rate:3;

        uint16_t oneshot_en:1;
        uint16_t pga:3;

        uint16_t mux:3;
        uint16_t busy:1;
    };

    struct LowThreshReg:public Reg16i<>{
        scexpr RegAddress address = 0b10;
        int16_t data;
    };

    struct HighThreshReg:public Reg16i<>{
        scexpr RegAddress address = 0b11; 
        int16_t data;
    };

    ConversionReg conversion_reg = {};
    ConfigReg config_reg = {};
    LowThreshReg low_thresh_reg = {};
    HighThreshReg high_thresh_reg = {};


    [[nodiscard]] DeviceResult readReg(const RegAddress addr, uint16_t & data);
    [[nodiscard]] DeviceResult writeReg(const RegAddress addr, const uint16_t data); 
    struct ConfigBuilder{

        // ADS111X & owner_;
        MUX mux_;
        DataRate datarate_;
        PGA pga_;
            
        __pure [[nodiscard]]
        static constexpr Option<MUX> singleend(const size_t N){
            switch(N){
                case 0: return Some(MUX::P0NG);
                case 1: return Some(MUX::P1NG);
                case 2: return Some(MUX::P2NG);
                case 3: return Some(MUX::P3NG);
            }
            return None;
        }

        __pure [[nodiscard]] 
        static constexpr Option<MUX> differential(const size_t P, const size_t N){

            constexpr std::array mappings{
                std::tuple{0UL,1UL,MUX::P0N1},
                std::tuple{0UL,3UL,MUX::P0N3},
                std::tuple{1UL,3UL,MUX::P1N3},
                std::tuple{2UL,3UL,MUX::P2N3}
            };
        
            // 使用范围遍历+模式匹配
            for (const auto& [valid_P, valid_N, mux_val] : mappings) {
                if (P == valid_P && N == valid_N) {
                    return Some(mux_val);
                }
            }
            return None;
        }

        __pure [[nodiscard]] 
        static constexpr Option<DataRate> datarate(const size_t dr){
            switch(dr){
                case 8: return Some(DataRate::_8);
                case 16: return Some(DataRate::_16);
                case 32: return Some(DataRate::_32);
                case 64: return Some(DataRate::_64);
                case 128: return Some(DataRate::_128);
                case 250: return Some(DataRate::_250);
                case 475: return Some(DataRate::_475);
                case 860: return Some(DataRate::_860);
            }
            return None;
        }

        Result<void, void> apply(){
            return Ok{};
        }
    };

public:
    // ADDR PIN CONNECTION SLAVE ADDRESS
    // GND 1001000
    // VDD 1001001
    // SDA 1001010
    // SCL 1001011
    scexpr uint8_t default_i2c_addr = 0b10010000;
    ADS111X(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    ADS111X(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    ADS111X(hal::I2c & i2c, const uint8_t addr = default_i2c_addr):i2c_drv_(hal::I2cDrv(i2c, addr)){};

    void startConv();

    bool isBusy();

    void setThreshold(int16_t low, int16_t high);

    void enableContMode(bool en = true);

    void setPga(const PGA pga);

    void setMux(const MUX mux);

    void setDataRate(const DataRate data_rate);

    bool ready();

    Option<real_t> result();

    DeviceResult verify();

    [[nodiscard]] constexpr auto builder(){return ConfigBuilder{};}

};

using ADS1113 = ADS111X;
using ADS1114 = ADS111X;
using ADS1115 = ADS111X;


};