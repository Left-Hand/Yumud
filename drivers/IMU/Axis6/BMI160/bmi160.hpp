#pragma once

#include "bmi160_prelude.hpp"
#include "primitive/arithmetic/angular.hpp"

namespace ymd::drivers::bmi160{
class BMI160 final{
public:
    using Self = BMI160;
    using Error = bmi160::Error;


    explicit BMI160(const hal::SpiDrv & spi_drv):
        transport_(spi_drv){;}
    explicit BMI160(hal::SpiDrv && spi_drv):
        transport_(std::move(spi_drv)){;}
    explicit BMI160(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        transport_(hal::SpiDrv(spi, rank)){;}

    explicit BMI160(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR):
        transport_(i2c, i2c_addr){;}


    struct [[nodiscard]] Config{
        AccOdr acc_odr;
        AccFs acc_fs;
        GyrOdr gyr_odr;
        GyrFs gyr_fs;

        static constexpr Config from_default(){
            return Config{
                .acc_odr = AccOdr::_800Hz,
                .acc_fs = AccFs::_8G,
                .gyr_odr = GyrOdr::_800Hz,
                .gyr_fs = GyrFs::_1000deg
            };
        }
    };

    [[nodiscard]] IResult<> init(const Config & cfg);
    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> reset();
    
    [[nodiscard]] IResult<> set_acc_odr(const AccOdr odr);
    [[nodiscard]] IResult<> set_acc_fs(const AccFs fs);
    [[nodiscard]] IResult<> set_gyr_odr(const GyrOdr odr);
    [[nodiscard]] IResult<> set_gyr_fs(const GyrFs fs);
    [[nodiscard]] IResult<> set_pmu_mode(const PmuType pmu, const PmuMode mode);

    [[nodiscard]] IResult<PmuMode> get_pmu_mode(const PmuType pmu);

    [[nodiscard]] IResult<Vec3<iq24>> read_acc();
    [[nodiscard]] IResult<Vec3<iq24>> read_gyr();

private:
    BoschImu_Transport transport_;
    Regset regs_;

    iq20 acc_scale_ = 0;
    iq20 gyr_scale_ = 0;

    [[nodiscard]] IResult<> write_command(uint8_t cmd){
        return transport_.write_command(std::bit_cast<uint8_t>(cmd));
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        return transport_.write_reg(reg);
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return transport_.read_reg(reg);
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(const uint8_t addr, T & data){
        return transport_.read_reg(addr, data);
    }

    [[nodiscard]] IResult<> self_test_acc();
    [[nodiscard]] IResult<> self_test_gyr();
    [[nodiscard]] static constexpr 
    iq20 accfs_to_scale(const AccFs fs){
        switch(fs){
            case AccFs::_2G:    return iq20(9.80665 * 4);
            case AccFs::_4G:    return iq20(9.80665 * 8);
            case AccFs::_8G:    return iq20(9.80665 * 16);
            case AccFs::_16G:   return iq20(9.80665 * 32);
        }
        __builtin_unreachable();
    }

    [[nodiscard]] static constexpr 
    iq20 gyrfs_to_scale(const GyrFs fs){
        switch(fs){
            case GyrFs::_125deg:    return iq20(DEG2RAD_RATIO) * (2 * 125);
            case GyrFs::_250deg:    return iq20(DEG2RAD_RATIO) * (2 * 250);
            case GyrFs::_500deg:    return iq20(DEG2RAD_RATIO) * (2 * 500);
            case GyrFs::_1000deg:   return iq20(DEG2RAD_RATIO) * (2 * 1000);
            case GyrFs::_2000deg:   return iq20(DEG2RAD_RATIO) * (2 * 2000);
        }
        __builtin_unreachable();
    }

    [[nodiscard]] constexpr
    Option<GyrOdr> from_gyr_odr(const iq16 odr){
        constexpr std::array odr_map = {
            25, 50, 100, 200, 400, 800, 1600, 3200
        };
    
        auto it = std::lower_bound(odr_map.begin(), odr_map.end(), (odr));
    
        if (it != odr_map.end()) {
            return Some(GyrOdr( std::distance(odr_map.begin(), it)));
        }
        return None;

    }

    [[nodiscard]] static constexpr
    Option<AccOdr> from_acc_odr(const iq16 odr){
        constexpr std::array odr_map = {
            iq16(25.0/32),
            iq16(25.0/16),
            iq16(25.0/8),
            iq16(25.0/4),
            iq16(25.0/2),
            iq16(25), 
            iq16(50), 
            iq16(100), 
            iq16(200), 
            iq16(400), 
            iq16(800),
            iq16(1600)
        };
    
        auto it = std::lower_bound(odr_map.begin(), odr_map.end(), (odr));
    
        if (it != odr_map.end()) {
            return Some(AccOdr(std::distance(odr_map.begin(), it)));
        }
        return None;


    }
};

}
