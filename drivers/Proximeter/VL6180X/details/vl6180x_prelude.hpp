#pragma once


// https://github.com/pololu/vl6180x-arduino/blob/master/VL6180X.h

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"



namespace ymd::drivers{

struct VL6180X_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b0101001);
    // using RegAddress = uint8_t;
    enum class RegAddress:uint16_t{
        IDENTIFICATION__MODEL_ID              = 0x000,
        IDENTIFICATION__MODEL_REV_MAJOR       = 0x001,
        IDENTIFICATION__MODEL_REV_MINOR       = 0x002,
        IDENTIFICATION__MODULE_REV_MAJOR      = 0x003,
        IDENTIFICATION__MODULE_REV_MINOR      = 0x004,
        IDENTIFICATION__DATE_HI               = 0x006,
        IDENTIFICATION__DATE_LO               = 0x007,
        IDENTIFICATION__TIME                  = 0x008, // 16-bit

        SYSTEM__MODE_GPIO0                    = 0x010,
        SYSTEM__MODE_GPIO1                    = 0x011,
        SYSTEM__HISTORY_CTRL                  = 0x012,
        SYSTEM__INTERRUPT_CONFIG_GPIO         = 0x014,
        SYSTEM__INTERRUPT_CLEAR               = 0x015,
        SYSTEM__FRESH_OUT_OF_RESET            = 0x016,
        SYSTEM__GROUPED_PARAMETER_HOLD        = 0x017,

        SYSRANGE__START                       = 0x018,
        SYSRANGE__THRESH_HIGH                 = 0x019,
        SYSRANGE__THRESH_LOW                  = 0x01A,
        SYSRANGE__INTERMEASUREMENT_PERIOD     = 0x01B,
        SYSRANGE__MAX_CONVERGENCE_TIME        = 0x01C,
        SYSRANGE__CROSSTALK_COMPENSATION_RATE = 0x01E, // 16-bit
        SYSRANGE__CROSSTALK_VALID_HEIGHT      = 0x021,
        SYSRANGE__EARLY_CONVERGENCE_ESTIMATE  = 0x022, // 16-bit
        SYSRANGE__PART_TO_PART_RANGE_OFFSET   = 0x024,
        SYSRANGE__RANGE_IGNORE_VALID_HEIGHT   = 0x025,
        SYSRANGE__RANGE_IGNORE_THRESHOLD      = 0x026, // 16-bit
        SYSRANGE__MAX_AMBIENT_LEVEL_MULT      = 0x02C,
        SYSRANGE__RANGE_CHECK_ENABLES         = 0x02D,
        SYSRANGE__VHV_RECALIBRATE             = 0x02E,
        SYSRANGE__VHV_REPEAT_RATE             = 0x031,

        SYSALS__START                         = 0x038,
        SYSALS__THRESH_HIGH                   = 0x03A,
        SYSALS__THRESH_LOW                    = 0x03C,
        SYSALS__INTERMEASUREMENT_PERIOD       = 0x03E,
        SYSALS__ANALOGUE_GAIN                 = 0x03F,
        SYSALS__INTEGRATION_PERIOD            = 0x040,

        RESULT__RANGE_STATUS                  = 0x04D,
        RESULT__ALS_STATUS                    = 0x04E,
        RESULT__INTERRUPT_STATUS_GPIO         = 0x04F,
        RESULT__ALS_VAL                       = 0x050, // 16-bit
        RESULT__HISTORY_BUFFER_0              = 0x052, // 16-bit
        RESULT__HISTORY_BUFFER_1              = 0x054, // 16-bit
        RESULT__HISTORY_BUFFER_2              = 0x056, // 16-bit
        RESULT__HISTORY_BUFFER_3              = 0x058, // 16-bit
        RESULT__HISTORY_BUFFER_4              = 0x05A, // 16-bit
        RESULT__HISTORY_BUFFER_5              = 0x05C, // 16-bit
        RESULT__HISTORY_BUFFER_6              = 0x05E, // 16-bit
        RESULT__HISTORY_BUFFER_7              = 0x060, // 16-bit
        RESULT__RANGE_VAL                     = 0x062,
        RESULT__RANGE_RAW                     = 0x064,
        RESULT__RANGE_RETURN_RATE             = 0x066, // 16-bit
        RESULT__RANGE_REFERENCE_RATE          = 0x068, // 16-bit
        RESULT__RANGE_RETURN_SIGNAL_COUNT     = 0x06C, // 32-bit
        RESULT__RANGE_REFERENCE_SIGNAL_COUNT  = 0x070, // 32-bit
        RESULT__RANGE_RETURN_AMB_COUNT        = 0x074, // 32-bit
        RESULT__RANGE_REFERENCE_AMB_COUNT     = 0x078, // 32-bit
        RESULT__RANGE_RETURN_CONV_TIME        = 0x07C, // 32-bit
        RESULT__RANGE_REFERENCE_CONV_TIME     = 0x080, // 32-bit

        RANGE_SCALER                          = 0x096, // 16-bit - see STSW-IMG003 core/inc/vl6180x_def.h

        READOUT__AVERAGING_SAMPLE_PERIOD      = 0x10A,
        FIRMWARE__BOOTUP                      = 0x119,
        FIRMWARE__RESULT_SCALER               = 0x120,
        I2C_SLAVE__DEVICE_ADDRESS             = 0x212,
        INTERLEAVED_MODE__ENABLE              = 0x2A3,

    };

    enum class Error_Kind:uint8_t{
        WrongWhoAmI,
        InvalidScaling,
        RangeDataNotReady,
        AmbientDataNotReady
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

};

static_assert(sizeof(VL6180X_Prelude) == 1);

class VL6180X_Phy final:public VL6180X_Prelude{
public:
    VL6180X_Phy(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    VL6180X_Phy(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const uint16_t command, const T data){
        const auto res = i2c_drv_.write_reg(command, data, MSB);
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
    
    template<typename T>
    [[nodiscard]] IResult<> read_reg(const uint16_t command, T & data){
        const auto res = i2c_drv_.read_reg(command, data, MSB);
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
private:
    hal::I2cDrv i2c_drv_;
};

}