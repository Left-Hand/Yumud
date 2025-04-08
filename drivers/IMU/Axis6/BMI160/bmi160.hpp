#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/details/BoschIMU.hpp"

namespace ymd::drivers{

class BMI160:public Axis6{
public:
    using Error = details::BoschSensorError;

    enum class DPS:uint8_t{
        _250, _500, _1000, _2000
    };

    enum class G:uint8_t{
        _2, _4, _8, _16
    };

    enum class AccOdr:uint8_t{
        _25_32 = 0b0001,
        _25_16,
        _25_8,
        _25_4,
        _25_2,
        _25,
        
        _50,
        _100,
        _200,
        _400,
        _800,
        _1600
    };

    enum class GyrOdr:uint8_t{
        _25 = 0b0110,
        
        _50,
        _100,
        _200,
        _400,
        _800,
        _1600,
        _3200
    };
    
    enum class AccRange:uint8_t{
        _2G     =   0b0011,
        _4G     =   0b0101,
        _8G     =   0b1000,
        _16G    =   0b1100
    };

    enum class GyrRange:uint8_t{
        _2000deg = 0b0000,
        _1000deg,
        _500deg,
        _250deg,
        _125deg
    };

    enum class Command:uint8_t{
        START_FOC = 0x04,
        ACC_SET_PMU = 0b0001'0000,
        GYR_SET_PMU = 0b0001'0100,
        MAG_SET_PMU = 0b0001'1000,
        FIFO_FLUSH = 0xB0,
        RESET_INTERRUPT =0xB1,
        SOFT_RESET = 0xB1,
        STEP_CNT_CLR = 0xB2
    };

    enum class PmuType{
        ACC,
        GYR,
        MAG
    };

    enum class PmuMode{
        SUSPEND,
        NORMAL,
        LOW_POWER,
        FAST_SETUP
    };

protected:

    BoschSensor_Phy phy_;

    scexpr uint8_t default_i2c_addr = 0b11010010;

    real_t acc_scale = 0;
    real_t gyr_scale = 0;

    // #pragma pack(push, 1)

    #include "regs.ipp"
    ChipIdReg chip_id_reg = {};
    uint8_t __resv1__ = {};
    ErrReg err_reg = {};
    PmuStatusReg pmu_status_reg = {};
    StatusReg status_reg = {};
    Vector3i16Reg mag_reg = {};
    RhallReg rhall_reg = {};

    Vector3i16Reg gyr_reg = {};
    Vector3i16Reg acc_reg = {};
    AccConfReg acc_conf_reg = {};
    AccRangeReg acc_range_reg = {};
    GyrConfReg gyr_conf_reg = {};
    GyrRangeReg gyr_range_reg = {};
    // #pragma pack(pop)

    static real_t calculate_acc_scale(const AccRange range);
    static real_t calculate_gyr_scale(const GyrRange range);
public:
    BMI160(BoschSensor_Phy && phy):phy_(phy){;}

    Result<void, Error> init();
    Result<void, Error> update();
    Result<void, Error> verify();
    Result<void, Error> reset();

    
    Result<void, Error> set_acc_odr(const AccOdr odr);
    Result<void, Error> set_acc_range(const AccRange range);
    Result<void, Error> set_gyr_odr(const GyrOdr odr);
    Result<void, Error> set_gyr_range(const GyrRange range);
    Result<void, Error> set_pmu_mode(const PmuType pum, const PmuMode mode);

    PmuMode get_pmu_mode(const PmuType pum);
    Option<Vector3_t<real_t>> get_acc();
    Option<Vector3_t<real_t>> get_gyr();


    [[nodiscard]] __fast_inline constexpr
    Option<GyrOdr> get_gyr_odr(const real_t odr){
        constexpr std::array odr_map = {
            25, 50, 100, 200, 400, 800, 1600, 3200
        };
    
        auto it = std::lower_bound(odr_map.begin(), odr_map.end(), (odr));
    
        if (it != odr_map.end()) {
            return Some(GyrOdr( std::distance(odr_map.begin(), it)));
        }
        return None;
    }

    [[nodiscard]] __fast_inline constexpr
    Option<AccOdr> get_acc_odr(const real_t odr){
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
