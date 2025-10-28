#pragma once

#include <optional>

#include "core/io/regs.hpp"
#include "drivers/IMU/details/BoschIMU.hpp"


namespace ymd::drivers{

struct BMI323_Prelude{
    using Error = ImuError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x68 >> 1);


enum class ErrorStatus:uint16_t{
    // 0b0000 (0x0) Feature engine still inactive
    FeatureEngineStillInactive = 0x0,

    // 0b0001 (0x1) Feature engine activated
    FeatureEngineActived = 0x1,

    // 0b0011 (0x3) Conﬁguration string download failed
    ConfigurationStringDownloadFailed = 0x3,

    // 0b0101 (0x5) No error
    NoError = 0x5,

    // 0b0110 (0x6) Axis map command was not processed because either a sensor was active or
    // self-calibration or self-test was ongoing

    // 0b1000 (0x8)
    // I3C TC-sync error because either I3C TC-sync enable request was sent while
    // auto-low-power feature was active or I3C TC-sync conﬁguration command
    // was sent with invalid TPH, TU and ODR values. For later case, invalid
    // conﬁguration parameters TPH, TU and ODR will not be used.

    // 0b1001 (0x9)
    // Ongoing self-calibration (gyroscope only) or self-test (gyroscope only) was
    // aborted. The command was aborted either due to device movements or due
    // to the abort command (self-calibration only) or due to a request to enable I3C
    // TC-sync feature (self-calibration only).

    // 0b1010 (0xA) Self-calibration (gyroscope only) command ignored because either
    // self-calibration or self-test or I3C TC-sync was ongoing

    // 0b1011 (0xB) Self-test (accelerometer and/or gyroscope) command ignored because either
    // self-calibration or self-test or I3C TC-sync was ongoing

    // 0b1100 (0xC)
    // Self-calibration (gyroscope only) or self-test (accelerometer and/or gyroscope)
    // command was not processed because pre-conditions were not met. Either
    // accelerometer was not conﬁgured correctly (self-test and self-calibration
    // gyroscope only) or auto-low-power feature was active.

    // 0b1101 (0xD)
    // Auto-mode change feature was enabled or illegal sensor conﬁguration change
    // detected in ACC_CONF/GYR_CONF while self-calibration or self-test was
    // ongoing. Self-calibration and self-test results may be inaccurate.
    // 0b1110 (0xE) I3C TC-sync enable request was sent while self-test (accelerometer and/or
    // gyroscope) was ongoing. I3C TC-sync will be enabled at the end of self-test.

    // 0b1111 (0xF)
    // Illegal sensor conﬁguration change detected in ACC_CONF/GYR_CONF
    // while I3C TC-sync was active. Sensors are re-conﬁgured is to requested I3C
    // TC-sync ODR.
};

enum class FeatureState:uint16_t{
    FeatureMode = 0x0,
    SelfCalibration,
    SelfTest,
    ErrorMode
};



enum class DataRate{
    _0_78125Hz  = 0x1,
    _1_5625Hz  = 0x2,
    _3_125Hz   = 0x3,
    _6_25Hz    = 0x4,
    _12_5Hz    = 0x5,
    _25Hz      = 0x6,
    _50Hz      = 0x7,
    _100Hz      = 0x7,
    _200Hz      = 0x8,
    _400Hz      = 0x9,
    _800Hz      = 0xa,
    _1600Hz     = 0xb,
    _3200Hz     = 0xc,
    _6400Hz     = 0xd
};

enum class AccFs:uint16_t{
    _2G = 0x0,
    _4G = 0x1,
    _8G = 0x2,
    _16G = 0x3
};

enum class Bw:uint16_t{
    By2,
    By4
};

enum class AverageTimes:uint16_t{
    NoAvg = 0x0,
    _2 = 0x1,
    _4 = 0x2,
    _8 = 0x3,
    _16 = 0x4,
    _32 = 0x5,
    _64 = 0x6
};

enum class AccMode:uint16_t {
// 0b000 (0x0) Disables the accelerometer
Diable = 0x0,

// 0b011 (0x3) Enables the accelerometer with sensing operated in duty-cycling
DutyCycling = 0x3,

// 0b100 (0x4) Enables the accelerometer in a continuous operation mode with reduced
// current
Continuous = 0x4,

// 0b111 (0x7) Enables the accelerometer in high performance mode
HighPerformance = 0x7
};

enum class GyrMode:uint16_t{
// 0b000 (0x0) Disables the gyroscope
// 0b001 (0x1) Disables the gyroscope but keep the gyroscope drive enabled
// 0b011 (0x3) Enables the gyroscope with sensing operated in duty-cycling
// 0b100 (0x4) Enables the gyroscope in a continuous operation mode with reduced current
// 0b111 (0x7) Enables the gyroscope in high performance mode
};

enum class GyrFs:uint16_t{
    _125Dps = 0x0,
    _250Dps = 0x1,
    _500Dps = 0x2,
    _1000Dps = 0x3,
    _2000Dps = 0x4
};


};


struct BMI323_Regset:public BMI323_Prelude{

//0x00
struct R16_ChipId:public Reg16<>{
    static constexpr uint16_t KEY = 0x33;
    uint16_t chip_id:8;
    uint16_t :8;
};

//0x01
struct R16_Err:public Reg16<>{
    // Fatal Error, chip is not in operational state (Boot-, power-system).
    // This flag will be reset only by power-on-reset or softreset.
    uint16_t fatal_err:1;
    uint16_t :1;
    // Overload of the feature engine detected. 
    uint16_t feat_eng_ovreld:1;
    uint16_t :1;
    uint16_t feat_eng_wd:1;
    uint16_t acc_conf_err:1;
    uint16_t gyr_conf_err:1;
    uint16_t i3c_error0:1;
    uint16_t :2;
    uint16_t i3c_error3:1;
    uint16_t :4;
};

//0x02
struct R16_Status:public Reg16<>{
    uint16_t por_detected:1;
    uint16_t :1;
    uint16_t drdy_temp:1;
    uint16_t drdy_gyr:1;
    uint16_t drdy_acc:1;
    uint16_t :8;
};


//0x0c
struct R16_SatFlags:public Reg16<>{
    uint16_t satf_acc_x:1;
    uint16_t satf_acc_y:1;
    uint16_t satf_acc_z:1;
    uint16_t satf_gyr_x:1;
    uint16_t satf_gyr_y:1;
    uint16_t satf_gyr_z:1;
    uint16_t :10;
};

struct _IntStatusInt:public Reg16<>{
    uint16_t no_motion:1;
    uint16_t any_motion:1;
    uint16_t flat:1;
    uint16_t orientation:1;
    uint16_t step_detector:1;
    uint16_t step_counter:1;
    uint16_t sig_motion:1;
    uint16_t tilt:1;
    uint16_t tap:1;
    uint16_t i3c:1;
    uint16_t err_status:1;
    uint16_t temp_drdy:1;
    uint16_t gyr_drdy:1;
    uint16_t acc_drdy:1;
    uint16_t fwm:1;
    uint16_t ffull:1;
};

//0x0d
struct R16_IntStatusInt1:public _IntStatusInt{
    
};

//0x0e
struct R16_IntStatusInt2:public _IntStatusInt{

};

//0x0f
struct R16_IntStatusIbi:public _IntStatusInt{

};


//0x10
struct R16_FeatureIo0:public Reg16<>{
    uint16_t no_motion_x_en:1;
    uint16_t no_motion_y_en:1;
    uint16_t no_motion_z_en:1;
    uint16_t any_motion_x_en:1;

    uint16_t any_motion_y_en:1;
    uint16_t any_motion_z_en:1;
    uint16_t flat_en:1;
    uint16_t orientation_en:1;

    uint16_t step_detector_en:1;
    uint16_t step_counter_en:1;
    uint16_t sig_motion_en:1;
    uint16_t tilt_en:1;

    uint16_t tap_detector_s_tap_en:1;
    uint16_t tap_detector_d_tap_en:1;
    uint16_t tap_detector_t_tap_en:1;
    uint16_t i3c_sync_en:1;
};




//0x11
struct R16_FeatureIo1:public Reg16<>{
    uint16_t error_status:4;
    uint16_t sc_st_complete:1;
    uint16_t gyro_sc_result:1;
    uint16_t st_result:1;
    uint16_t sample_rate_err:1;
    uint16_t axis_map_complete:1;
    uint16_t state:2;
};

// 0x12
struct R16_FeatureIo2:public Reg16<>{
    uint16_t count;
};

// 0x13
struct R16_FeatureIo3:public Reg16<>{
    uint16_t count;
};

// 0x14
struct R16_FeatureIoStatus:public Reg16<>{
    // On read: data has been written by the feature engine
    // On write: data written by the host shall be sent to the feature engine
    uint16_t feature_io_status:1;
    uint16_t :15;
};

//0x15
struct R16_FifoFillLevel:public Reg16<>{
    uint16_t fifo_fill_level:11;
    uint16_t :5;
};



//0x16 fifo data



//0x20

};
}