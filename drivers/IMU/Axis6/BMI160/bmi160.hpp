#pragma once

#include "bmi160_prelude.hpp"

namespace ymd::drivers{
class BMI160 final:
    public AccelerometerIntf, 
    public GyroscopeIntf,
    public BMI160_Prelude{
public:
    BMI160(const hal::SpiDrv & spi_drv):
        phy_(spi_drv){;}
    BMI160(hal::SpiDrv && spi_drv):
        phy_(std::move(spi_drv)){;}
    BMI160(Some<hal::Spi *> spi, const hal::SpiSlaveIndex index):
        phy_(hal::SpiDrv(spi, index)){;}

    [[nodiscard]] IResult<> init(const Config & cfg);
    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> reset();
    
    [[nodiscard]] IResult<> set_acc_odr(const AccOdr odr);
    [[nodiscard]] IResult<> set_acc_fs(const AccFs range);
    [[nodiscard]] IResult<> set_gyr_odr(const GyrOdr odr);
    [[nodiscard]] IResult<> set_gyr_fs(const GyrFs range);
    [[nodiscard]] IResult<> set_pmu_mode(const PmuType pum, const PmuMode mode);

    [[nodiscard]] IResult<PmuMode> get_pmu_mode(const PmuType pum);

    [[nodiscard]] IResult<Vector3<q24>> read_acc();
    [[nodiscard]] IResult<Vector3<q24>> read_gyr();

private:
    using  Regs = BMI160_Regs;
    Regs regs_;
    BoschSensor_Phy phy_;

    real_t acc_scale_ = 0;
    real_t gyr_scale_ = 0;

    [[nodiscard]] IResult<> self_test_acc();
    [[nodiscard]] IResult<> self_test_gyr();
    [[nodiscard]] static constexpr real_t calculate_acc_scale(const AccFs fs){
        constexpr auto g = real_t(9.806);
        switch(fs){
            case AccFs::_2G:    return g * 4;
            case AccFs::_4G:    return g * 8;
            case AccFs::_8G:    return g * 16;
            case AccFs::_16G:   return g * 32;
            default: __builtin_unreachable();
        }
    }

    [[nodiscard]] static real_t calculate_gyr_scale(const GyrFs fs){
        switch(fs){
            case GyrFs::_125deg:    return ANGLE2RAD(125);
            case GyrFs::_250deg:    return ANGLE2RAD(250);
            case GyrFs::_500deg:    return ANGLE2RAD(500);
            case GyrFs::_1000deg:   return ANGLE2RAD(1000);
            case GyrFs::_2000deg:   return ANGLE2RAD(2000);
            default: __builtin_unreachable();
        }
    }

    [[nodiscard]] constexpr
    Option<GyrOdr> from_gyr_odr(const real_t odr){
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
    Option<AccOdr> from_acc_odr(const real_t odr){
        constexpr std::array odr_map = {
            real_t(25.0/32),
            real_t(25.0/16),
            real_t(25.0/8),
            real_t(25.0/4),
            real_t(25.0/2),
            real_t(25), 
            real_t(50), 
            real_t(100), 
            real_t(200), 
            real_t(400), 
            real_t(800),
            real_t(1600)
        };
    
        auto it = std::lower_bound(odr_map.begin(), odr_map.end(), (odr));
    
        if (it != odr_map.end()) {
            return Some(AccOdr(std::distance(odr_map.begin(), it)));
        }
        return None;


    }
};

}
