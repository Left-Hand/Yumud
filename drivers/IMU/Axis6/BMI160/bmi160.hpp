#pragma once

#include "bmi160_prelude.hpp"
#include "core/utils/angle.hpp"

namespace ymd::drivers{
class BMI160 final:
    public AccelerometerIntf, 
    public GyroscopeIntf,
    public BMI160_Prelude{
public:
    explicit BMI160(const hal::SpiDrv & spi_drv):
        phy_(spi_drv){;}
    explicit BMI160(hal::SpiDrv && spi_drv):
        phy_(std::move(spi_drv)){;}
    explicit BMI160(Some<hal::Spi *> spi, const hal::SpiSlaveIndex index):
        phy_(hal::SpiDrv(spi, index)){;}

    [[nodiscard]] IResult<> init(const Config & cfg);
    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> reset();
    
    [[nodiscard]] IResult<> set_acc_odr(const AccOdr odr);
    [[nodiscard]] IResult<> set_acc_fs(const AccFs range);
    [[nodiscard]] IResult<> set_gyr_odr(const GyrOdr odr);
    [[nodiscard]] IResult<> set_gyr_fs(const GyrFs range);
    [[nodiscard]] IResult<> set_pmu_mode(const PmuType pmu, const PmuMode mode);

    [[nodiscard]] IResult<PmuMode> get_pmu_mode(const PmuType pmu);

    [[nodiscard]] IResult<Vec3<q24>> read_acc();
    [[nodiscard]] IResult<Vec3<q24>> read_gyr();

private:
    using  Regs = BMI160_Regs;
    BoschSensor_Phy phy_;
    Regs regs_;

    q20 acc_scale_ = 0;
    q20 gyr_scale_ = 0;

    [[nodiscard]] IResult<> write_command(uint8_t cmd){
        return phy_.write_command(std::bit_cast<uint8_t>(cmd));
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        return phy_.write_reg(reg);
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return phy_.read_reg(reg);
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(const uint8_t addr, T & data){
        return phy_.read_reg(addr, data);
    }

    [[nodiscard]] IResult<> self_test_acc();
    [[nodiscard]] IResult<> self_test_gyr();
    [[nodiscard]] static constexpr 
    q20 accfs_to_scale(const AccFs fs){
        switch(fs){
            case AccFs::_2G:    return GRAVITY_ACC<q20> * 4;
            case AccFs::_4G:    return GRAVITY_ACC<q20> * 8;
            case AccFs::_8G:    return GRAVITY_ACC<q20> * 16;
            case AccFs::_16G:   return GRAVITY_ACC<q20> * 32;
            default: __builtin_unreachable();
        }
    }

    [[nodiscard]] static constexpr 
    q20 gyrfs_to_scale(const GyrFs fs){
        switch(fs){
            case GyrFs::_125deg:    return (2 * 125_deg).to_radians();
            case GyrFs::_250deg:    return (2 * 250_deg).to_radians();
            case GyrFs::_500deg:    return (2 * 500_deg).to_radians();
            case GyrFs::_1000deg:   return (2 * 1000_deg).to_radians();
            case GyrFs::_2000deg:   return (2 * 2000_deg).to_radians();
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
