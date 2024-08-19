#pragma once

#include <optional>

#include "../drivers/device_defs.h"
#include "../drivers/IMU/IMU.hpp"
#include "types/uint24_t.h"

#pragma pack(push, 1)
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

#define REG16(x) (*reinterpret_cast<uint16_t *>(&x))
#define REG8(x) (*reinterpret_cast<uint8_t *>(&x))

#define BMI160_DEBUG

#ifdef BMI160_DEBUG
#undef BMI160_DEBUG
#define BMI160_DEBUG(...) DEBUG_PRINTLN(SpecToken::Space, std::hex, ##__VA_ARGS__, "\t|", __PRETTY_FUNCTION__);
#else
#define BMI160_DEBUG(...)
#endif

class BMI160:public Axis6{
public:

    enum class DPS:uint8_t{
        _250, _500, _1000, _2000
    };

    enum class G:uint8_t{
        _2, _4, _8, _16
    };

protected:
    std::optional<I2cDrv> i2c_drv;
    std::optional<SpiDrv> spi_drv;

    using RegAddress = uint8_t;
    struct ChipIdReg{
        static constexpr RegAddress address = 0x00;
        static constexpr uint8_t correct = 0x00;
        uint8_t data;
    };

    struct ErrReg{
        static constexpr RegAddress address = 0x02;
        uint8_t fatal_err:1;
        uint8_t err_code:4;
        uint8_t i2c_fail_err:1;
        uint8_t drop_cmd_err:1;
        uint8_t mag_drdy_err:1;
    };

    struct RhallReg{
        static constexpr RegAddress address = 0x0A;
    };

    struct Vector3i16Reg{
        static constexpr RegAddress mag_address = 0x04;
        static constexpr RegAddress mag_x_address = 0x04;
        static constexpr RegAddress mag_y_address = 0x06;
        static constexpr RegAddress mag_z_address = 0x08;
        
        static constexpr RegAddress gyro_address = 0x04;
        static constexpr RegAddress gyro_x_address = 0x04;
        static constexpr RegAddress gyro_y_address = 0x06;
        static constexpr RegAddress gyro_z_address = 0x08;

        static constexpr RegAddress acc_address = 0x04;
        static constexpr RegAddress acc_x_address = 0x04;
        static constexpr RegAddress acc_y_address = 0x06;
        static constexpr RegAddress acc_z_address = 0x08;
        
        int16_t x;
        int16_t y;
        int16_t z;
    };

    struct SensorTimeReg{
        static constexpr RegAddress address = 0x18; // SENSOR_TIME_2
        uint32_t time: 24; // Assuming 8 bits for sensor time
    };

    struct StatusReg{
        static constexpr RegAddress address = 0x1B;
        uint8_t __resv__:1;
        uint8_t gyr_self_test_ok:1;
        uint8_t mag_man_op:1;
        uint8_t foc_rdy:1;
        uint8_t nvm_rdy:1;
        uint8_t drdy_mag: 1;
        uint8_t drdy_gyr: 1;
        uint8_t drdy_acc: 1;
    };

    struct IntStatus0Reg{
        static constexpr RegAddress address = 0x1c; // INT_STATUS_3
        uint8_t step_int:1;
        uint8_t sigmot_int:1;
        uint8_t anym_int:1;
        uint8_t pmu_trigger_int:1;
        uint8_t d_tap_int:1;
        uint8_t s_tap_int:1;
        uint8_t orient_int:1;
        uint8_t flat_int:1;
    };

    struct IntStatus1Reg{
        static constexpr RegAddress address = 0x1d; // INT_STATUS_3
        uint8_t __resv__:2;
        uint8_t highg_int:1;
        uint8_t lowg_int:1;
        uint8_t drdy_int:1;
        uint8_t ffull_int:1;
        uint8_t sfwm_int:1;
        uint8_t nomo_int:1;
    };

    struct IntStatus2Reg{
        static constexpr RegAddress address = 0x1d; // INT_STATUS_3
        
        uint8_t anym_first_x: 1;
        uint8_t anym_first_y: 1;
        uint8_t anym_first_z: 1;
        uint8_t anym_sign: 1;

        uint8_t tap_first_z: 1;
        uint8_t tap_first_x: 1;
        uint8_t tap_first_y: 1;
        uint8_t tap_sign: 1;
    };


    struct IntStatus3Reg{
        static constexpr RegAddress address = 0x1e; // INT_STATUS_3
        uint8_t high_first_x: 1;
        uint8_t high_first_y: 1;
        uint8_t high_first_z: 1;
        uint8_t high_sign: 1;

        uint8_t orient_2: 1;
        uint8_t orient_1_0: 2;
        uint8_t flat: 1;
    };

    struct TemperatureReg{
        static constexpr RegAddress address = 0x21; // TEMPERATURE_1
        uint16_t temperature: 16; // Assuming 8 bits for temperature
    };

    struct FifoLengthReg{
        static constexpr RegAddress address = 0x23; // FIFO_LENGTH_1
        uint16_t fifo_byte_counter: 11; // Assuming 8 bits for temperature
        uint16_t __resv__:5;
    };
    
    struct FifoDataReg{
        static constexpr RegAddress address = 0x25;
        uint8_t data;
    };

    struct AccRangeReg{
        static constexpr RegAddress address = 0x41;
        uint8_t acc_range:4;
        uint8_t __resv__:4;
    };

    struct GyrConfReg{
        static constexpr RegAddress address = 0x42;
        uint8_t gyr_odr:4;
        uint8_t gyr_bwp:2;
        uint8_t __resv__:2;
    };

    struct GyrRangeReg{
        static constexpr RegAddress address = 0x43;
        uint8_t gyr_range:3;
        uint8_t __resv__:5;
    };

    struct MagConfReg{
        static constexpr RegAddress address = 0x44;
        uint8_t mag_odr:4;
        uint8_t __resv__:4;
    };

    struct FifoDownsReg {
        static constexpr uint8_t address = 0x45;
        uint8_t gyr_fifo_down : 3; // 8 bits for acc_fifo_filt_data
        uint8_t gyr_fifo_filt_data:1;
        uint8_t acc_fifo_data : 3; // 8 bits for gyr_fifo_filt_data
        uint8_t acc_fifo_filt_data:1;
    };

    struct FifoConfig0Reg {
        static constexpr uint8_t address = 0x46;
        uint8_t fifo_water_mark : 8; // 8 bits for fifo_water_mark
    };

    struct FifoConfig1Reg {
        static constexpr uint8_t address = 0x47;

        uint8_t __resv__ : 1;   // 1 bit reserved
        uint8_t fifo_time_en : 1; // 1 bit for fifo_time_en
        uint8_t fifo_tag_int2_en : 1; // 1 bit for fifo_tag_int2_en
        uint8_t fifo_tag_int1_en : 1; // 1 bit for fifo_tag_int1_en

        uint8_t fifo_header_en : 1; // 1 bit for fifo_header_en
        uint8_t fifo_mag_en : 1; // 1 bit for fifo_mag_en
        uint8_t fifo_acc_en : 1; // 1 bit for fifo_acc_en
        uint8_t fifo_gyr_en : 1; // 1 bit for fifo_gyr_en
    };

    struct MagIf0Reg {
        static constexpr uint8_t address = 0x4B;
        uint8_t __resv__:1;
        uint8_t i2c_device_addr: 7; // Reserved (no specific fields)
    };

    struct MagIf1Reg {
        static constexpr uint8_t address = 0x4C;
        uint8_t mag_rd_burst:2;
        uint8_t mag_offset:4;
        uint8_t __resv__ : 1;   // 7 bits reserved
        uint8_t mag_manual_en : 1; // 1 bit for mag_manual_en
    };

    struct MagIf2Reg {
        static constexpr uint8_t address = 0x4D;
        uint8_t read_addr : 8; // 8 bits for read_addr
    };

    struct MagIf3Reg {
        static constexpr uint8_t address = 0x4E;
        uint8_t write_addr : 8; // 8 bits for write_addr
    };

    struct MagIf4Reg {
        static constexpr uint8_t address = 0x4F;
        uint8_t write_data : 8; // 8 bits for write_data
    };

    struct IntEn0Reg {
        static constexpr uint8_t address = 0x50;
        
        uint8_t int_anymo_x_en:1;
        uint8_t int_anymo_y_en:1;
        uint8_t int_anymo_z_en:1;
        uint8_t :1;

        uint8_t int_d_tap_en : 1;
        uint8_t int_s_tap_en : 1;
        uint8_t int_orient_en : 1;
        uint8_t int_flat_en : 1;
    };

    struct IntEn1Reg {
        static constexpr uint8_t address = 0x51;
        uint8_t int_highg_x_en : 1;
        uint8_t int_highg_y_en : 1;
        uint8_t int_highg_z_en : 1;
        uint8_t int1_low_en : 1;

        uint8_t int1_drdy_en : 1;
        uint8_t int1_full_en : 1;
        uint8_t int1_fwm_en : 1;
        uint8_t : 1; // Reserved
    };

    struct IntEn2Reg {
        static constexpr uint8_t address = 0x52;
        uint8_t int_nomox_en:1;
        uint8_t int_nomoy_en:1;
        uint8_t int_nomoz_en:1;
        uint8_t int_step_det_en:1;
        uint8_t : 4; // Reserved
    };

    struct IntOutCtrlReg {
        static constexpr uint8_t address = 0x53;
        uint8_t int1_edge_ctrl:1;
        uint8_t int1_lvl:1;
        uint8_t int1_od:1;
        uint8_t int1_output_en;
        uint8_t int2_edge_ctrl:1;
        uint8_t int2_lvl:1;
        uint8_t int2_od:1;
        uint8_t int2_output_en:1;
    };

    struct IntLatchReg {
        static constexpr uint8_t address = 0x54;
        uint8_t int_latch:4;
        uint8_t int1_input_en:1;
        uint8_t int2_input_en:1;
        uint8_t : 2; // Reserved
    };

    struct IntMap0Reg {
        static constexpr uint8_t address = 0x55;

        uint8_t int1_lowg_step:1;
        uint8_t int1_highg:1;
        uint8_t int1_anymotion:1;
        uint8_t int1_nomotiom:1;

        uint8_t int1_d_tap : 1;
        uint8_t int1_s_tap : 1;
        uint8_t int1_orient : 1;
        uint8_t int1_flat : 1;
    };

    struct IntMap1Reg {
        static constexpr uint8_t address = 0x56;
        uint8_t int2_pmu_trig:1;
        uint8_t int2_fful:1;
        uint8_t int2_fwm:1;
        uint8_t int2_drdy : 1;

        uint8_t int1_pmu_trig:1;
        uint8_t int1_fful:1;
        uint8_t int1_fwm:1;
        uint8_t int1_drdy : 1;
    };

    struct IntMap2Reg {
        static constexpr uint8_t address = 0x57;

        uint8_t int2_lowg_step:1;
        uint8_t int2_highg:1;
        uint8_t int2_anymotion:1;
        uint8_t int2_nomotiom:1;

        uint8_t int2_d_tap : 1;
        uint8_t int2_s_tap : 1;
        uint8_t int2_orient : 1;
        uint8_t int2_flat : 1;
    };

    struct IntData0Reg{
        static constexpr uint8_t address = 0x58;
        uint8_t:3;
        uint8_t int_tap_src:1;
        uint8_t:3;
        uint8_t int_low_high_src:1;
    };

    struct IntData1Reg{
        static constexpr uint8_t address = 0x59;
        uint8_t:7;
        uint8_t int_motion_src:1;
    };

    struct IntLowHigh0Reg{
        static constexpr uint8_t address = 0x5A;
        uint8_t int_low_dur;
    };

    struct IntLowHigh1Reg{
        static constexpr uint8_t address = 0x5B;
        uint8_t int_low_th;
    };

    struct IntLowHigh2Reg{
        static constexpr uint8_t address = 0x5C;
        uint8_t int_low_hy:2;
        uint8_t :4;
        uint8_t int_high_hy:2;
    };

    struct IntLowHigh3Reg{
        static constexpr uint8_t address = 0x5D;
        uint8_t int_high_dur;
    };

    struct IntLowHigh4Reg{
        static constexpr uint8_t address = 0x5E;
        uint8_t int_high_th;
    };

    struct IntMotion0Reg{
        static constexpr uint8_t address = 0x5F;
        uint8_t int_anym_dur:2;
        uint8_t int_slo_nomo_dur:6;
    };

    struct IntMotion1Reg{
        static constexpr uint8_t address = 0x60;
        uint8_t int_anymo_th;
    };

    struct IntMotion2Reg{
        static constexpr uint8_t address = 0x61;
        uint8_t int_slo_nomo_th;
    };

    static constexpr uint8_t default_chip_id = 0;//TODO
    static constexpr uint8_t default_i2c_addr = 0x68;

    struct{
        ChipIdReg chip_id_reg;
        uint8_t __resv1__;
        ErrReg err_reg;
        StatusReg status_reg;
        
    };


    void writeReg(const uint8_t addr, const uint8_t data){
        if(i2c_drv) i2c_drv->writeReg(addr, data);
        if(spi_drv){
            SpiDrv & drv = spi_drv.value();
            drv.write(uint8_t(addr), false);
            drv.write(data);

            BMI160_DEBUG("Wspi", addr, data);

        }
    }

    void readReg(const RegAddress addr, uint8_t & data){
        if(i2c_drv) i2c_drv->readReg((uint8_t)addr, data);
        if(spi_drv){
            SpiDrv & drv = spi_drv.value();
            drv.write(uint8_t(addr), false);
            drv.read(data);
            BMI160_DEBUG("Rspi", addr, data);
        }
    }

    void requestData(const RegAddress addr, auto * datas, const size_t len){
        if(i2c_drv) i2c_drv->readPool(uint8_t(addr), datas, len);
        if(spi_drv){
            SpiDrv & drv = spi_drv.value();
            drv.write(uint8_t(addr), false);
            drv.read(datas, len * sizeof(datas[0]));
        }
    }

public:

    BMI160(const I2cDrv & _bus_drv):i2c_drv(_bus_drv){;}
    BMI160(I2cDrv && _bus_drv):i2c_drv(_bus_drv){;}
    BMI160(I2c & bus):i2c_drv(I2cDrv(bus, default_i2c_addr)){;}
    BMI160(const SpiDrv & _bus_drv):spi_drv(_bus_drv){;}
    BMI160(SpiDrv && _bus_drv):spi_drv(_bus_drv){;}
    BMI160(Spi & bus, const uint8_t index):spi_drv(SpiDrv(bus, index)){;}

    void init();
    void update();

    bool check();


    std::tuple<real_t, real_t, real_t> getAccel() override;
    std::tuple<real_t, real_t, real_t> getGyro() override;
};


#pragma pack(pop)