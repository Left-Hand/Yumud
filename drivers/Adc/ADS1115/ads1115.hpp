#pragma once

#include "ads1115_prelude.hpp"

namespace ymd::drivers{


class ADS111X final:
    public ADS111X_Regs{
public:
    explicit ADS111X(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit ADS111X(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit ADS111X(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){};

    [[nodiscard]] IResult<> start_conv();

    [[nodiscard]] IResult<> set_threshold(int16_t low, int16_t high);

    [[nodiscard]] IResult<> enable_cont_mode(const Enable en);

    [[nodiscard]] IResult<> set_pga(const PGA pga);

    [[nodiscard]] IResult<> set_mux(const MUX mux);

    [[nodiscard]] IResult<> set_data_rate(const DataRate data_rate);

    [[nodiscard]] IResult<bool> is_busy();

    [[nodiscard]] Option<real_t> get_voltage();

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] constexpr auto builder(){return ConfigBuilder{};}
private:
    hal::I2cDrv i2c_drv_;

    [[nodiscard]] IResult<> read_reg(const RegAddr addr, uint16_t & data);

    [[nodiscard]] IResult<> write_reg(const RegAddr addr, const uint16_t data); 

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        const auto res = write_reg(T::ADDRESS, reg.as_val());
        if(res.is_ok()) reg.apply();
        return res;
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return read_reg(T::ADDRESS, reg.as_ref());
    }

    struct ConfigBuilder{

        // ADS111X & owner_;
        MUX mux_;
        DataRate datarate_;
        PGA pga_;
            
        [[nodiscard]]
        static constexpr Option<MUX> singleend(const size_t N){
            switch(N){
                case 0: return Some(MUX::P0NG);
                case 1: return Some(MUX::P1NG);
                case 2: return Some(MUX::P2NG);
                case 3: return Some(MUX::P3NG);
            }
            return None;
        }

        [[nodiscard]] 
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

        [[nodiscard]] 
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