#pragma once

#include "details/ICM20948_Prelude.hpp"

namespace ymd::drivers{
class ICM20948:
    public ICM20948_Prelude
{
public:
    /* Main Functions */

    // sensor init function.
    // if sensor id is wrong, it is stuck in while.
    IResult<> init();
    // void ak09916_init();

    // 16 bits ADC value. raw data.
    // IResult<> gyro_read(axises* data);	
    // IResult<> accel_read(axises* data);
    // bool ak09916_mag_read(axises* data); 

    // Convert 16 bits ADC value to their unit.
    // IResult<> gyro_read_dps(axises* data); 
    // IResult<> accel_read_g(axises* data);
    // bool ak09916_mag_read_uT(axises* data);


    /* Sub Functions */
    IResult<> validate();
    bool ak09916_who_am_i();

    IResult<> device_reset();
    void ak09916_soft_reset();

    IResult<> wakeup();
    IResult<> sleep();

    IResult<> spi_slave_enable();

    IResult<> i2c_master_reset();
    IResult<> i2c_master_enable();
    IResult<> i2c_master_clk_frq(uint8_t config); // 0 - 15

    IResult<> clock_source(uint8_t source);
    IResult<> odr_align_enable();

    IResult<> gyro_low_pass_filter(uint8_t config); // 0 - 7
    IResult<> accel_low_pass_filter(uint8_t config); // 0 - 7

    // Output Data Rate = 1.125kHz / (1 + divider)
    IResult<> gyro_sample_rate_divider(uint8_t divider);
    IResult<> accel_sample_rate_divider(uint16_t divider);
    IResult<> ak09916_operation_mode_setting(OperationMode mode);

    // // Calibration before select full scale.
    // IResult<> gyro_calibration();
    // IResult<> accel_calibration();

    IResult<> set_gyr_fs(GyrFs gyrfs);
    IResult<> set_acc_fs(AccFs accfs);
private:
    InvensenseImu_Transport transport_;
    ICM20948_Regs regs_;

    iq16 gyro_scale_factor;
    iq16 accel_scale_factor;


    IResult<> select_bank(BankKind bank);

    IResult<> read_single_reg(BankKind bank, uint8_t reg_addr, uint8_t & reg_val);

    IResult<> write_single_reg(BankKind bank, uint8_t reg_addr, uint8_t reg_val);

    IResult<> read_multiple_reg(BankKind bank, uint8_t reg_addr, std::span<uint8_t> pbuf);

    IResult<> write_multiple_reg(BankKind bank, uint8_t reg_addr, std::span<const uint8_t> pbuf);

    IResult<> read_single_ak09916_reg(uint8_t reg_addr);

    IResult<> write_single_ak09916_reg(uint8_t reg_addr, uint8_t reg_val);

    IResult<> read_multiple_ak09916_reg(uint8_t reg_addr, std::span<uint8_t> pbuf);
};



}

