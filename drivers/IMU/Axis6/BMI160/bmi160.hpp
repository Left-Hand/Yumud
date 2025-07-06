#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/details/BoschIMU.hpp"

namespace ymd::drivers{

struct BMI160_Prelude{
    using Error = ImuError;

    template<typename T = void>
    using IResult = Result<T, Error>;



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
    
    enum class AccFs:uint8_t{
        _2G     =   0b0011,
        _4G     =   0b0101,
        _8G     =   0b1000,
        _16G    =   0b1100
    };

    enum class GyrFs:uint8_t{
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

    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0b11010010);

    struct Config{
        AccOdr acc_odr = AccOdr::_800;
        AccFs acc_fs = AccFs::_8G;
        GyrOdr gyr_odr = GyrOdr::_800;
        GyrFs gyr_fs = GyrFs::_1000deg;
    };
};


struct BMI160_Regs:public BMI160_Prelude{

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
    AccFsReg acc_range_reg = {};
    GyrConfReg gyr_conf_reg = {};
    GyrFsReg gyr_range_reg = {};
    // #pragma pack(pop)
};

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
