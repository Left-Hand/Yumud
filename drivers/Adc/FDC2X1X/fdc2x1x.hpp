#pragma once

#include "../../hal/bus/i2c/i2cdrv.hpp"

#ifdef FDC2X1X_DEBUG
#define FDC2X1X_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define FDC2X1X_DEBUG(...)
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

#define REG16(x) (*reinterpret_cast<uint16_t *>(&x))
#define REG8(x) (*reinterpret_cast<uint8_t *>(&x))

class FDC2X1X{
public:
    enum class Package:uint8_t{
        FDC2112,
        FDC2114,
        FDC2212,
        FDC2214
    };

    enum class DataRate:uint8_t{
        _20 = 0, _45, _90, _175, _330, _600, _1000
    };

protected:
    I2cDrv i2c_drv;

    using RegAddress =uint8_t;

    struct ConversionDataHighReg{
        static constexpr RegAddress address =0x00;
        uint16_t data_msb:12;
        bool err_aw:1;
        bool err_wd:1;
        uint16_t __resv__:2;
    };

    struct ConversionDataLowReg{
        static constexpr RegAddress address =0x01;
        uint16_t data_lsb:16;
    };

    struct ConversionDataReg{
        ConversionDataHighReg high;
        ConversionDataLowReg low;
    };

    struct ReferenceCountReg{
        static constexpr RegAddress address =0x08;
        uint8_t count;
    };

    struct OffsetReg{
        static constexpr RegAddress address =0x0C;
        uint8_t offset;
    };

    struct SettleCountReg{
        static constexpr RegAddress address =0x10;
        uint8_t settle_count;
    };

    struct ClockDividerReg{
        static constexpr RegAddress address =0x14;
        uint16_t fref_divider:10;
        uint16_t __resv1__:2;
        uint16_t fin_sel:2;
        uint16_t __resv2__:2;
    };

    struct StatusReg{
        static constexpr RegAddress address =0x18;
        bool ch3_unread_conv:1;
        bool ch2_unread_conv:1;
        bool ch1_unread_conv:1;
        bool ch0_unread_conv:1;
        uint16_t __resv1__:2;
        bool data_ready:1;
        bool amp_low_warning:1;
        bool amp_high_warning:1;
        bool wtd_timeout_error:1;
        uint16_t __resv2__:2;
        uint16_t channel_error:2;
    };

    struct StatusConfigReg{
        static constexpr RegAddress address =0x19;
        bool data_ready_to_int:1;
        uint16_t __resv__:4;
        uint16_t wtd_timeout_error_to_int:1;
        bool amp_low_warning_to_out:1;
        bool amp_high_warning_to_out:1;
        bool wtd_timeout_error_to_out:1;
        uint16_t __resv2__:2;
    };

    struct ConfigReg{
        static constexpr RegAddress address =0x1a;
        uint16_t __resv1__:6;
        bool high_current_drive:1;
        bool intb_disen:1;
        uint16_t __resv2__:1;
        bool lower_power_en:1;
        uint16_t __resv3__:1;
        bool sleep_mode_en:1;
        uint16_t active_channel_sel:2;
    };

    enum class BandWidth:uint8_t{
        _1MHz   = 0b001,
        _3_3MHz = 0b100,
        _10MHz  = 0b101,
        _33MHz  = 0b111
    };

    enum class AutoScanConfig:uint8_t{
        C0C1 = 0b00,
        C0C1C2 = 0b01,
        C0C1C2C3 = 0b10
    };

    struct MuxConfigReg{
        static constexpr RegAddress address =0x1b;
        BandWidth bandwidth:3;
        uint16_t __resv__:10;
        AutoScanConfig auto_scan_config:2;
        bool auto_scan_en:1;

    };

    enum class Gain:uint8_t{
        _1 = 0b00,
        _4 = 0b01,
        _8 = 0b10,
        _16 = 0b11
    };

    struct ResetDevReg{
        static constexpr RegAddress address =0x1c;
        uint16_t __resv1__:9;
        Gain gain:2;
        uint16_t __resv2__:4; 
        bool reset_dev:1;
    };

    enum class DriveCurrent : uint8_t {
        _0016uA  = 0b00000, // 00000: 0.016 mA
        _0018uA  = 0b00001, // 00001: 0.018 mA
        _0021uA  = 0b00010, // 00010: 0.021 mA
        _0025uA  = 0b00011, // 00011: 0.025 mA
        _0028uA  = 0b00100, // 00100: 0.028 mA
        _0033uA  = 0b00101, // 00101: 0.033 mA
        _0038uA  = 0b00110, // 00110: 0.038 mA
        _0044uA  = 0b00111, // 00111: 0.044 mA
        _0052uA  = 0b01000, // 01000: 0.052 mA
        _0060uA  = 0b01001, // 01001: 0.060 mA
        _0069uA  = 0b01010, // 01010: 0.069 mA
        _0081uA  = 0b01011, // 01011: 0.081 mA
        _0093uA  = 0b01100, // 01100: 0.093 mA
        _0108uA  = 0b01101, // 01101: 0.108 mA
        _0126uA  = 0b01110, // 01110: 0.126 mA
        _0146uA  = 0b01111, // 01111: 0.146 mA
        _0169uA  = 0b10000, // 10000: 0.169 mA
        _0196uA  = 0b10001, // 10001: 0.196 mA
        _0228uA  = 0b10010, // 10010: 0.228 mA
        _0264uA  = 0b10011, // 10011: 0.264 mA
        _0307uA  = 0b10100, // 10100: 0.307 mA
        _0356uA  = 0b10101, // 10101: 0.356 mA
        _0413uA  = 0b10110, // 10110: 0.413 mA
        _0479uA  = 0b10111, // 10111: 0.479 mA
        _0555uA  = 0b11000, // 11000: 0.555 mA
        _0644uA  = 0b11001, // 11001: 0.644 mA
        _0747uA  = 0b11010, // 11010: 0.747 mA
        _0867uA  = 0b11011, // 11011: 0.867 mA
        _1006uA  = 0b11100, // 11100: 1.006 mA
        _1167uA  = 0b11101, // 11101: 1.167 mA
        _1354uA  = 0b11110, // 11110: 1.354 mA
        _1571uA  = 0b11111  // 11111: 1.571 mA
    };

    struct DriveCurrentReg{
        static constexpr RegAddress address =0x1e;
        uint16_t __resv__:11;
        DriveCurrent drive_current:5;
    };

    struct ManufacturerIdReg{
        static constexpr RegAddress address =0x7E;
        static constexpr uint16_t correct = 0x5449;
        uint16_t id;
    };

    struct DeviceIdReg{
        static constexpr RegAddress address =0x7F;
        static constexpr uint16_t correct = 03054;
        uint16_t id;
    };

    struct{
        ConversionDataReg conv_data_regs[4];
        ReferenceCountReg ref_count_regs[4];
        OffsetReg offset_regs[4];
        SettleCountReg settle_count_regs[4];
        ClockDividerReg clock_divider_regs[4];
        StatusReg status_reg;
        StatusConfigReg status_config_reg;
        ConfigReg config_reg;
        MuxConfigReg mux_config_reg;
        ResetDevReg reset_dev_reg;
        DriveCurrentReg drive_current_regs[4];
        ManufacturerIdReg manufacturer_id_reg;
        DeviceIdReg device_id_reg;
    };

    void readReg(const RegAddress addr, auto & data){
        i2c_drv.readReg(uint8_t(addr), data);
    }

    void writeReg(const RegAddress addr, const auto & data){
        i2c_drv.writeReg(uint8_t(addr), data);
    }
public:
    static constexpr uint8_t default_i2c_addr = 0x54;

    FDC2X1X(const I2cDrv & _i2c_drv):i2c_drv(_i2c_drv){;}
    FDC2X1X(I2cDrv && _i2c_drv):i2c_drv(_i2c_drv){;}
    FDC2X1X(I2c & _i2c, const uint8_t _addr = default_i2c_addr):i2c_drv(I2cDrv(_i2c, _addr)){};

    void init();

    bool isConvDone(){
        readReg(StatusReg::address, REG16(status_reg));
        return status_reg.data_ready;
    }

    bool isConvDone(uint8_t channel){
        channel = MIN(channel, 3);
        readReg(StatusReg::address, REG16(status_reg));
        switch(channel){
            case 0: return status_reg.ch0_unread_conv;
            case 1: return status_reg.ch1_unread_conv;
            case 2: return status_reg.ch2_unread_conv;
            case 3: return status_reg.ch3_unread_conv;
            default: return false;
        }
    }

    void reset(){
        reset_dev_reg.reset_dev = true;
        writeReg(ResetDevReg::address,REG16(reset_dev_reg));
        reset_dev_reg.reset_dev = false;
    }

    uint32_t getData(uint8_t channel){
        channel = MIN(channel, 3);
        uint32_t ret = 0;
        auto & highreg = conv_data_regs[channel].high;
        auto & lowreg = conv_data_regs[channel].low;

        readReg(highreg.address, REG16(highreg));
        ret |= (highreg.data_msb << 16);
        readReg(lowreg.address, REG16(lowreg));
        ret |= lowreg.data_lsb;

        return ret;
    }
};