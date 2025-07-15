#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/utils/Option.hpp"

#include "concept/analog_channel.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{


struct ADS111X_Prelude{

    enum class Error_Kind:uint8_t{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)
    FRIEND_DERIVE_DEBUG(Error_Kind)

    using RegAddress = uint8_t;

    // ADDR PIN CONNECTION SLAVE ADDRESS
    // GND 1001000
    // VDD 1001001
    // SDA 1001010
    // SCL 1001011
    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b1001000);

    template<typename T = void>
    using IResult = Result<T, Error>;

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

};

struct ADS111X_Regs:public ADS111X_Prelude{ 

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
};

class ADS111X final:
    public ADS111X_Regs{
public:
    ADS111X(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    ADS111X(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    ADS111X(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){};

    [[nodiscard]] IResult<> start_conv();

    [[nodiscard]] IResult<> set_threshold(int16_t low, int16_t high);

    [[nodiscard]] IResult<> enable_cont_mode(bool en = true);

    [[nodiscard]] IResult<> set_pga(const PGA pga);

    [[nodiscard]] IResult<> set_mux(const MUX mux);

    [[nodiscard]] IResult<> set_data_rate(const DataRate data_rate);

    [[nodiscard]] IResult<bool> is_busy();

    [[nodiscard]] Option<real_t> get_voltage();

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] constexpr auto builder(){return ConfigBuilder{};}
private:
    hal::I2cDrv i2c_drv_;

    [[nodiscard]] IResult<> read_reg(const RegAddress addr, uint16_t & data);

    [[nodiscard]] IResult<> write_reg(const RegAddress addr, const uint16_t data); 

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        const auto res = write_reg(reg.address, reg.as_val());
        if(res.is_ok()) reg.apply();
        return res;
    }

    [[nodiscard]] IResult<> read_reg(auto & reg){
        return read_reg(reg.address, reg.as_ref());
    }

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
};

using ADS1113 = ADS111X;
using ADS1114 = ADS111X;
using ADS1115 = ADS111X;


};