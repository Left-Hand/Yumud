#pragma once

#include "details/ICM20948_collections.hpp"

namespace ymd::drivers{
class ICM20948:
    public AccelerometerIntf,
    public GyroscopeIntf,
    public MagnetometerIntf,
    public ICM20948_Regs
{
public:
    /* Main Functions */

    // sensor init function.
    // if sensor id is wrong, it is stuck in while.
    [[nodiscard]] IResult<void> init();
    void ak09916_init();

    // 16 bits ADC value. raw data.
    // [[nodiscard]] IResult<void> gyro_read(axises* data);	
    // [[nodiscard]] IResult<void> accel_read(axises* data);
    // bool ak09916_mag_read(axises* data); 

    // Convert 16 bits ADC value to their unit.
    // [[nodiscard]] IResult<void> gyro_read_dps(axises* data); 
    // [[nodiscard]] IResult<void> accel_read_g(axises* data);
    bool ak09916_mag_read_uT(axises* data);


    /* Sub Functions */
    [[nodiscard]] IResult<bool> who_am_i();
    bool ak09916_who_am_i();

    [[nodiscard]] IResult<void> device_reset();
    void ak09916_soft_reset();

    [[nodiscard]] IResult<void> wakeup();
    [[nodiscard]] IResult<void> sleep();

    [[nodiscard]] IResult<void> spi_slave_enable();

    [[nodiscard]] IResult<void> i2c_master_reset();
    [[nodiscard]] IResult<void> i2c_master_enable();
    [[nodiscard]] IResult<void> i2c_master_clk_frq(uint8_t config); // 0 - 15

    [[nodiscard]] IResult<void> clock_source(uint8_t source);
    [[nodiscard]] IResult<void> odr_align_enable();

    [[nodiscard]] IResult<void> gyro_low_pass_filter(uint8_t config); // 0 - 7
    [[nodiscard]] IResult<void> accel_low_pass_filter(uint8_t config); // 0 - 7

    // Output Data Rate = 1.125kHz / (1 + divider)
    [[nodiscard]] IResult<void> gyro_sample_rate_divider(uint8_t divider);
    [[nodiscard]] IResult<void> accel_sample_rate_divider(uint16_t divider);
    [[nodiscard]] IResult<void> ak09916_operation_mode_setting(operation_mode mode);

    // // Calibration before select full scale.
    // [[nodiscard]] IResult<void> gyro_calibration();
    // [[nodiscard]] IResult<void> accel_calibration();

    [[nodiscard]] IResult<void> gyro_full_scale_select(gyro_full_scale full_scale);
    [[nodiscard]] IResult<void> accel_full_scale_select(accel_full_scale full_scale);
private:
    InvensenseSensor_Phy phy_;

    real_t gyro_scale_factor;
    real_t accel_scale_factor;


    IResult<> select_user_bank(userbank ub);

    uint8_t read_single_reg(userbank ub, uint8_t reg);

    IResult<> write_single_reg(userbank ub, uint8_t reg, uint8_t val);

    uint8_t* read_multiple_reg(userbank ub, uint8_t reg, uint8_t len);

    IResult<> write_multiple_reg(userbank ub, uint8_t reg, uint8_t* val, uint8_t len);

    uint8_t read_single_ak09916_reg(uint8_t reg);

    IResult<> write_single_ak09916_reg(uint8_t reg, uint8_t val);

    uint8_t* read_multiple_ak09916_reg(uint8_t reg, uint8_t len);
};



}

