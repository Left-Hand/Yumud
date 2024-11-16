

#pragma once

#include "drivers/IMU/IMU.hpp"


#ifdef ICM42688_DEBUG
#undef ICM42688_DEBUG
#define ICM42688_DEBUG(...) DEBUG_PRINTLN(SpecToken::Space, std::hex, ##__VA_ARGS__, "\t|", __PRETTY_FUNCTION__);
#else
#define ICM42688_DEBUG(...)
#endif

namespace ymd::drivers{

class ICM42688:public Axis6{
public:

    enum class AFS:uint8_t{
        _16G,// default
        _8G,
        _4G,
        _2G,
    };

    enum class AODR:uint8_t
    {
        _32000HZ,
        _16000HZ,
        _8000HZ,
        _4000HZ,
        _2000HZ,
        _1000HZ,// default
        _200HZ,
        _100HZ,
        _50HZ,
        _25HZ,
        _12_5HZ,
        _6_25HZ,
        _3_125HZ,
        _1_5625HZ,
        _500HZ,
    };

    enum class GFS:uint8_t{
        _2000DPS,// default
        _1000DPS,
        _500DPS,
        _250DPS,
        _125DPS,
        _62_5DPS,
        _31_25DPS,
        _15_625DPS,
    };

    enum class GODR:uint8_t{
        _32000HZ,
        _16000HZ,
        _8000HZ,
        _4000HZ,
        _2000HZ,
        _1000HZ,// default
        _200HZ,
        _100HZ,
        _50HZ,
        _25HZ,
        _12_5HZ,
        _X0HZ,
        _X1HZ,
        _X2HZ,
        _500HZ,
    };

    enum class RegAddress:uint8_t{
        ID = 0x47,
        DEVICE_CONFIG             = 0x11,
        DRIVE_CONFIG              = 0x13,
        INT_CONFIG                = 0x14,
        FIFO_CONFIG               = 0x16,
        TEMP_DATA1                = 0x1D,
        TEMP_DATA0                = 0x1E,
        ACCEL_DATA_X1             = 0x1F,
        ACCEL_DATA_X0             = 0x20,
        ACCEL_DATA_Y1             = 0x21,
        ACCEL_DATA_Y0             = 0x22,
        ACCEL_DATA_Z1             = 0x23,
        ACCEL_DATA_Z0             = 0x24,
        GYRO_DATA_X1              = 0x25,
        GYRO_DATA_X0              = 0x26,
        GYRO_DATA_Y1              = 0x27,
        GYRO_DATA_Y0              = 0x28,
        GYRO_DATA_Z1              = 0x29,
        GYRO_DATA_Z0              = 0x2A,
        TMST_FSYNCH               = 0x2B,
        TMST_FSYNCL               = 0x2C,
        INT_STATUS                = 0x2D,
        FIFO_COUNTH               = 0x2E,
        FIFO_COUNTL               = 0x2F,
        FIFO_DATA                 = 0x30,
        APEX_DATA0                = 0x31,
        APEX_DATA1                = 0x32,
        APEX_DATA2                = 0x33,
        APEX_DATA3                = 0x34,
        APEX_DATA4                = 0x35,
        APEX_DATA5                = 0x36,
        INT_STATUS2               = 0x37,
        INT_STATUS3               = 0x38,
        SIGNAL_PATH_RESET         = 0x4B,
        INTF_CONFIG0              = 0x4C,
        INTF_CONFIG1              = 0x4D,
        PWR_MGMT0                 = 0x4E,
        GYRO_CONFIG0              = 0x4F,
        ACCEL_CONFIG0             = 0x50,
        GYRO_CONFIG1              = 0x51,
        GYRO_ACCEL_CONFIG0        = 0x52,
        ACCEL_CONFIG1             = 0x53,
        TMST_CONFIG               = 0x54,
        APEX_CONFIG0              = 0x56,
        SMD_CONFIG                = 0x57,
        FIFO_CONFIG1              = 0x5F,
        FIFO_CONFIG2              = 0x60,
        FIFO_CONFIG3              = 0x61,
        FSYNC_CONFIG              = 0x62,
        INT_CONFIG0               = 0x63,
        INT_CONFIG1               = 0x64,
        INT_SOURCE0               = 0x65,
        INT_SOURCE1               = 0x66,
        INT_SOURCE3               = 0x68,
        INT_SOURCE4               = 0x69,
        FIFO_LOST_PKT0            = 0x6C,
        FIFO_LOST_PKT1            = 0x6D,
        SELF_TEST_CONFIG          = 0x70,
        WHO_AM_I                  = 0x75,
        REG_BANK_SEL              = 0x76,
        SENSOR_CONFIG0            = 0x03,
        GYRO_CONFIG_STATIC2       = 0x0B,
        GYRO_CONFIG_STATIC3       = 0x0C,
        GYRO_CONFIG_STATIC4       = 0x0D,
        GYRO_CONFIG_STATIC5       = 0x0E,
        GYRO_CONFIG_STATIC6       = 0x0F,
        GYRO_CONFIG_STATIC7       = 0x10,
        GYRO_CONFIG_STATIC8       = 0x11,
        GYRO_CONFIG_STATIC9       = 0x12,
        GYRO_CONFIG_STATIC10      = 0x13,
        XG_ST_DATA                = 0x5F,
        YG_ST_DATA                = 0x60,
        ZG_ST_DATA                = 0x61,
        TMSTVAL0                  = 0x62,
        TMSTVAL1                  = 0x63,
        TMSTVAL2                  = 0x64,
        INTF_CONFIG4              = 0x7A,
        INTF_CONFIG5              = 0x7B,
        INTF_CONFIG6              = 0x7C
    };
protected:
    std::optional<I2cDrv> i2c_drv_;
    std::optional<SpiDrv> spi_drv_;

    // using RegAddress = uint8_t;


    scexpr uint8_t default_i2c_addr = 0x68;

    real_t lsb_acc_x64;
    real_t lsb_gyro_x256;

    
    struct{
        Vec3i16 accel_data;
        Vec3i16 gyro_data;
    };


    void writeReg(const uint8_t addr, const uint8_t data);

    void readReg(const uint8_t addr, uint8_t & data);
    
    void requestData(const uint8_t addr, int16_t * datas, const size_t len);
    void writeCommand(const uint8_t cmd){
        writeReg(0x7e, cmd);
    }
public:

    ICM42688(const I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    ICM42688(I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    ICM42688(I2c & i2c, const uint8_t i2c_addr = default_i2c_addr):i2c_drv_(I2cDrv(i2c, default_i2c_addr)){;}
    ICM42688(const SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    ICM42688(SpiDrv && spi_drv):spi_drv_(spi_drv){;}
    ICM42688(Spi & spi, const uint8_t index):spi_drv_(SpiDrv(spi, index)){;}

    void init();
    
    void update();

    bool verify();

    void reset();

    std::tuple<real_t, real_t, real_t> getAccel() override;
    std::tuple<real_t, real_t, real_t> getGyro() override;
};

}
