

#include "core/io/regs.hpp"
#include "core/utils/Errno.hpp"
#include "drivers/IMU/IMU.hpp"

#include "drivers/IMU/details/InvensenseIMU.hpp"

// https://github.com/Seeed-Studio/Seeed_Arduino_LSM6DS3/blob/master/LSM6DS3.h

namespace ymd::drivers{

struct LSM6DS3_Prelude{

    using Error = ImuError;
    template<typename T = void>
    using IResult = Result<T, Error>;


    static constexpr uint8_t LSM6DS3_ACC_GYRO_WHO_AM_I  			= 0X69;
    static constexpr uint8_t LSM6DS3_C_ACC_GYRO_WHO_AM_I         = 0x6A;

    static constexpr size_t RAM_SIZE = 4096u;

    enum class RegAddr:uint8_t{
        /************** Device Register  *******************/
        TEST_PAGE  			= 0X00,
        RAM_ACCESS  			= 0X01,
        SENSOR_SYNC_TIME  		= 0X04,
        SENSOR_SYNC_EN  		= 0X05,
        FIFO_CTRL1  			= 0X06,
        FIFO_CTRL2  			= 0X07,
        FIFO_CTRL3  			= 0X08,
        FIFO_CTRL4  			= 0X09,
        FIFO_CTRL5  			= 0X0A,
        ORIENT_CFG_G  			= 0X0B,
        REFERENCE_G  			= 0X0C,
        INT1_CTRL  			= 0X0D,
        INT2_CTRL  			= 0X0E,
        WHO_AM_I_REG  			= 0X0F,
        CTRL1_XL  			= 0X10,
        CTRL2_G  			= 0X11,
        CTRL3_C  			= 0X12,
        CTRL4_C  			= 0X13,
        CTRL5_C  			= 0X14,
        CTRL6_C  			= 0X15,
        CTRL7_G  			= 0X16,
        CTRL8_XL  			= 0X17,
        CTRL9_XL  			= 0X18,
        CTRL10_C  			= 0X19,
        MASTER_CONFIG  		= 0X1A,
        WAKE_UP_SRC  			= 0X1B,
        TAP_SRC  			= 0X1C,
        D6D_SRC  			= 0X1D,
        STATUS_REG  			= 0X1E,
        OUT_TEMP_L  			= 0X20,
        OUT_TEMP_H  			= 0X21,
        OUTX_L_G  			= 0X22,
        OUTX_H_G  			= 0X23,
        OUTY_L_G  			= 0X24,
        OUTY_H_G  			= 0X25,
        OUTZ_L_G  			= 0X26,
        OUTZ_H_G  			= 0X27,
        OUTX_L_XL  			= 0X28,
        OUTX_H_XL  			= 0X29,
        OUTY_L_XL  			= 0X2A,
        OUTY_H_XL  			= 0X2B,
        OUTZ_L_XL  			= 0X2C,
        OUTZ_H_XL  			= 0X2D,
        SENSORHUB1_REG  		= 0X2E,
        SENSORHUB2_REG  		= 0X2F,
        SENSORHUB3_REG  		= 0X30,
        SENSORHUB4_REG  		= 0X31,
        SENSORHUB5_REG  		= 0X32,
        SENSORHUB6_REG  		= 0X33,
        SENSORHUB7_REG  		= 0X34,
        SENSORHUB8_REG  		= 0X35,
        SENSORHUB9_REG  		= 0X36,
        SENSORHUB10_REG  		= 0X37,
        SENSORHUB11_REG  		= 0X38,
        SENSORHUB12_REG  		= 0X39,
        FIFO_STATUS1  			= 0X3A,
        FIFO_STATUS2  			= 0X3B,
        FIFO_STATUS3  			= 0X3C,
        FIFO_STATUS4  			= 0X3D,
        FIFO_DATA_OUT_L  		= 0X3E,
        FIFO_DATA_OUT_H  		= 0X3F,
        TIMESTAMP0_REG  		= 0X40,
        TIMESTAMP1_REG  		= 0X41,
        TIMESTAMP2_REG  		= 0X42,
        STEP_COUNTER_L  		= 0X4B,
        STEP_COUNTER_H  		= 0X4C,
        FUNC_SRC  			= 0X53,
        TAP_CFG1  			= 0X58,
        TAP_THS_6D  			= 0X59,
        INT_DUR2  			= 0X5A,
        WAKE_UP_THS  			= 0X5B,
        WAKE_UP_DUR  			= 0X5C,
        FREE_FALL  			= 0X5D,
        MD1_CFG  			= 0X5E,
        MD2_CFG  			= 0X5F,

        /************** Access Device RAM  *******************/
        ADDR0_TO_RW_RAM         = 0x62,
        ADDR1_TO_RW_RAM         = 0x63,
        DATA_TO_WR_RAM          = 0x64,
        DATA_RD_FROM_RAM        = 0x65,

        /************** Embedded functions register mapping  *******************/
    };

    enum class EmbededRegAddr:uint8_t{
        SLV0_ADD                     = 0x02,
        SLV0_SUBADD                  = 0x03,
        SLAVE0_CONFIG                = 0x04,
        SLV1_ADD                     = 0x05,
        SLV1_SUBADD                  = 0x06,
        SLAVE1_CONFIG                = 0x07,
        SLV2_ADD                     = 0x08,
        SLV2_SUBADD                  = 0x09,
        SLAVE2_CONFIG                = 0x0A,
        SLV3_ADD                     = 0x0B,
        SLV3_SUBADD                  = 0x0C,
        SLAVE3_CONFIG                = 0x0D,
        DATAWRITE_SRC_MODE_SUB_SLV0  = 0x0E,
        CONFIG_PEDO_THS_MIN          = 0x0F,
        CONFIG_TILT_IIR              = 0x10,
        CONFIG_TILT_ACOS             = 0x11,
        CONFIG_TILT_WTIME            = 0x12,
        SM_STEP_THS                  = 0x13,
        MAG_SI_XX                    = 0x24,
        MAG_SI_XY                    = 0x25,
        MAG_SI_XZ                    = 0x26,
        MAG_SI_YX                    = 0x27,
        MAG_SI_YY                    = 0x28,
        MAG_SI_YZ                    = 0x29,
        MAG_SI_ZX                    = 0x2A,
        MAG_SI_ZY                    = 0x2B,
        MAG_SI_ZZ                    = 0x2C,
        MAG_OFFX_L                   = 0x2D,
        MAG_OFFX_H                   = 0x2E,
        MAG_OFFY_L                   = 0x2F,
        MAG_OFFY_H                   = 0x30,
        MAG_OFFZ_L                   = 0x31,
        MAG_OFFZ_H                   = 0x32,
    };
};


}