#pragma once

// 参考来源：

// 无许可证
// https://github.com/mokhwasomssi/stm32_hal_icm20948/blob/master/stm32f411_fw_icm20948/ICM20948/icm20948.h
//  * 注意：本实现为完全原创，未使用上述项目的任何代码。
//  * 参考仅用于理解问题领域，未复制任何具体实现。

#include "core/io/regs.hpp"
#include "core/utils/Errno.hpp"
#include "drivers/IMU/IMU.hpp"

#include "drivers/IMU/details/InvensenseIMU.hpp"

namespace ymd::drivers{

struct ICM20948_Prelude{
    using Error = ImuError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    /* ICM-20948 Registers */
    static constexpr uint8_t ICM20948_ID = 0xEA;
    static constexpr uint8_t REG_BANK_SEL = 0x7F;


    /* Typedefs */
    enum class [[nodiscard]] BankKind:uint8_t{
        _0 = 0 << 4,
        _1 = 1 << 4,
        _2 = 2 << 4,
        _3 = 3 << 4
    } ;

    enum class [[nodiscard]] GyrFs:uint8_t{
        _250dps,
        _500dps,
        _1000dps,
        _2000dps
    };

    enum class [[nodiscard]] AccFs:uint8_t{
        _2g,
        _4g,
        _8g,
        _16g
    };


    enum class [[nodiscard]] OperationMode:uint8_t{
        PowerDown = 0,
        SingleShot = 1,
        _10hz = 2,
        _20hz = 4,
        _50hz = 6,
        _100hz = 8
    };

    // USER BANK 0
    static constexpr uint8_t B0_WHO_AM_I = 0x00;		
    static constexpr uint8_t B0_USER_CTRL = 0x03;
    static constexpr uint8_t B0_LP_CONFIG = 0x05;
    static constexpr uint8_t B0_PWR_MGMT_1 = 0x06;
    static constexpr uint8_t B0_PWR_MGMT_2 = 0x07;
    static constexpr uint8_t B0_INT_PIN_CFG = 0x0F;		
    static constexpr uint8_t B0_INT_ENABLE = 0x10;
    static constexpr uint8_t B0_INT_ENABLE_1 = 0x11;
    static constexpr uint8_t B0_INT_ENABLE_2 = 0x12;
    static constexpr uint8_t B0_INT_ENABLE_3 = 0x13;
    static constexpr uint8_t B0_I2C_MST_STATUS = 0x17;		
    static constexpr uint8_t B0_INT_STATUS = 0x19;		
    static constexpr uint8_t B0_INT_STATUS_1 = 0x1A;
    static constexpr uint8_t B0_INT_STATUS_2 = 0x1B;
    static constexpr uint8_t B0_INT_STATUS_3 = 0x1C;
    static constexpr uint8_t B0_DELAY_TIMEH = 0x28;
    static constexpr uint8_t B0_DELAY_TIMEL = 0x29;
    static constexpr uint8_t B0_ACCEL_XOUT_H = 0x2D;		
    static constexpr uint8_t B0_ACCEL_XOUT_L = 0x2E;		
    static constexpr uint8_t B0_ACCEL_YOUT_H = 0x2F;		
    static constexpr uint8_t B0_ACCEL_YOUT_L = 0x30;		
    static constexpr uint8_t B0_ACCEL_ZOUT_H = 0x31;		
    static constexpr uint8_t B0_ACCEL_ZOUT_L = 0x32;	
    static constexpr uint8_t B0_GYRO_XOUT_H = 0x33;	
    static constexpr uint8_t B0_GYRO_XOUT_L = 0x34;
    static constexpr uint8_t B0_GYRO_YOUT_H = 0x35;
    static constexpr uint8_t B0_GYRO_YOUT_L = 0x36;
    static constexpr uint8_t B0_GYRO_ZOUT_H = 0x37;
    static constexpr uint8_t B0_GYRO_ZOUT_L = 0x38;
    static constexpr uint8_t B0_TEMP_OUT_H = 0x39;		
    static constexpr uint8_t B0_TEMP_OUT_L = 0x3A;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_00 = 0x3B;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_01 = 0x3C;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_02 = 0x3D;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_03 = 0x3E;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_04 = 0x3F;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_05 = 0x40;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_06 = 0x41;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_07 = 0x42;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_08 = 0x43;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_09 = 0x44;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_10 = 0x45;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_11 = 0x46;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_12 = 0x47;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_13 = 0x48;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_14 = 0x49;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_15 = 0x4A;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_16 = 0x4B;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_17 = 0x4C;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_18 = 0x4D;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_19 = 0x4E;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_20 = 0x4F;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_21 = 0x50;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_22 = 0x51;
    static constexpr uint8_t B0_EXT_SLV_SENS_DATA_23 = 0x52;
    static constexpr uint8_t B0_FIFO_EN_1 = 0x66;	
    static constexpr uint8_t B0_FIFO_EN_2 = 0x67;
    static constexpr uint8_t B0_FIFO_RST = 0x68;
    static constexpr uint8_t B0_FIFO_MODE = 0x69;
    static constexpr uint8_t B0_FIFO_COUNTH = 0X70;
    static constexpr uint8_t B0_FIFO_COUNTL = 0X71;
    static constexpr uint8_t B0_FIFO_R_W = 0x72;
    static constexpr uint8_t B0_DATA_RDY_STATUS = 0x74;
    static constexpr uint8_t B0_FIFO_CFG = 0x76;	

    // USER BANK 1
    static constexpr uint8_t B1_SELF_TEST_X_GYRO = 0x02;	
    static constexpr uint8_t B1_SELF_TEST_Y_GYRO = 0x03;
    static constexpr uint8_t B1_SELF_TEST_Z_GYRO = 0x04;
    static constexpr uint8_t B1_SELF_TEST_X_ACCEL = 0x0E;	
    static constexpr uint8_t B1_SELF_TEST_Y_ACCEL = 0x0F;
    static constexpr uint8_t B1_SELF_TEST_Z_ACCEL = 0x10;
    static constexpr uint8_t B1_XA_OFFS_H = 0x14;	
    static constexpr uint8_t B1_XA_OFFS_L = 0x15;
    static constexpr uint8_t B1_YA_OFFS_H = 0x17;
    static constexpr uint8_t B1_YA_OFFS_L = 0x18;
    static constexpr uint8_t B1_ZA_OFFS_H = 0x1A;
    static constexpr uint8_t B1_ZA_OFFS_L = 0x1B;
    static constexpr uint8_t B1_TIMEBASE_CORRECTION_PLL = 0x28;	

    // USER BANK 2
    static constexpr uint8_t B2_GYRO_SMPLRT_DIV = 0x00;	
    static constexpr uint8_t B2_GYRO_CONFIG_1 = 0x01;	
    static constexpr uint8_t B2_GYRO_CONFIG_2 = 0x02;
    static constexpr uint8_t B2_XG_OFFS_USRH = 0x03;	
    static constexpr uint8_t B2_XG_OFFS_USRL = 0x04;
    static constexpr uint8_t B2_YG_OFFS_USRH = 0x05;
    static constexpr uint8_t B2_YG_OFFS_USRL = 0x06;
    static constexpr uint8_t B2_ZG_OFFS_USRH = 0x07;
    static constexpr uint8_t B2_ZG_OFFS_USRL = 0x08;
    static constexpr uint8_t B2_ODR_ALIGN_EN = 0x09;	
    static constexpr uint8_t B2_ACCEL_SMPLRT_DIV_1 = 0x10;	
    static constexpr uint8_t B2_ACCEL_SMPLRT_DIV_2 = 0x11;		
    static constexpr uint8_t B2_ACCEL_INTEL_CTRL = 0x12;		
    static constexpr uint8_t B2_ACCEL_WOM_THR = 0x13;
    static constexpr uint8_t B2_ACCEL_CONFIG = 0x14;
    static constexpr uint8_t B2_ACCEL_CONFIG_2 = 0x15;
    static constexpr uint8_t B2_FSYNC_CONFIG = 0x52;
    static constexpr uint8_t B2_TEMP_CONFIG = 0x53;
    static constexpr uint8_t B2_MOD_CTRL_USR = 0X54;

    // USER BANK 3
    static constexpr uint8_t B3_I2C_MST_ODR_CONFIG = 0x00;
    static constexpr uint8_t B3_I2C_MST_CTRL = 0x01;
    static constexpr uint8_t B3_I2C_MST_DELAY_CTRL = 0x02;	
    static constexpr uint8_t B3_I2C_SLV0_ADDR = 0x03;
    static constexpr uint8_t B3_I2C_SLV0_REG = 0x04;		
    static constexpr uint8_t B3_I2C_SLV0_CTRL = 0x05;
    static constexpr uint8_t B3_I2C_SLV0_DO = 0x06;
    static constexpr uint8_t B3_I2C_SLV1_ADDR = 0x07;		
    static constexpr uint8_t B3_I2C_SLV1_REG = 0x08;		
    static constexpr uint8_t B3_I2C_SLV1_CTRL = 0x09;
    static constexpr uint8_t B3_I2C_SLV1_DO = 0x0A;
    static constexpr uint8_t B3_I2C_SLV2_ADDR = 0x0B;		
    static constexpr uint8_t B3_I2C_SLV2_REG = 0x0C;		
    static constexpr uint8_t B3_I2C_SLV2_CTRL = 0x0D;
    static constexpr uint8_t B3_I2C_SLV2_DO = 0x0E;
    static constexpr uint8_t B3_I2C_SLV3_ADDR = 0x0F;		
    static constexpr uint8_t B3_I2C_SLV3_REG = 0x10;		
    static constexpr uint8_t B3_I2C_SLV3_CTRL = 0x11;
    static constexpr uint8_t B3_I2C_SLV3_DO = 0x12;
    static constexpr uint8_t B3_I2C_SLV4_ADDR = 0x13;	
    static constexpr uint8_t B3_I2C_SLV4_REG = 0x14;		
    static constexpr uint8_t B3_I2C_SLV4_CTRL = 0x15;
    static constexpr uint8_t B3_I2C_SLV4_DO = 0x16;
    static constexpr uint8_t B3_I2C_SLV4_DI = 0x17;
        

    /* AK09916 Registers */
    static constexpr uint8_t AK09916_ID = 0x09;
    static constexpr uint8_t MAG_SLAVE_ADDR = 0x0C;

    static constexpr uint8_t MAG_WIA2 = 0x01;
    static constexpr uint8_t MAG_ST1 = 0x10;
    static constexpr uint8_t MAG_HXL = 0x11;
    static constexpr uint8_t MAG_HXH = 0x12;
    static constexpr uint8_t MAG_HYL = 0x13;
    static constexpr uint8_t MAG_HYH = 0x14;
    static constexpr uint8_t MAG_HZL = 0x15;
    static constexpr uint8_t MAG_HZH = 0x16;
    static constexpr uint8_t MAG_ST2 = 0x18;
    static constexpr uint8_t MAG_CNTL2 = 0x31;
    static constexpr uint8_t MAG_CNTL3 = 0x32;
    static constexpr uint8_t MAG_TS1 = 0x33;
    static constexpr uint8_t MAG_TS2 = 0x34;

    static constexpr uint8_t READ_CMD = 0x80;  
    static constexpr uint8_t WRITE_CMD = 0x00;
};

struct ICM20948_Regs:public ICM20948_Prelude{



        
};


}