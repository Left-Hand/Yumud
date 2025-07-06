#pragma once

// 相当优秀的磁力计 调整好一点不带漂

// 这个驱动已经完成编写
// 这个驱动已经完成测试

// https://www.memsic.com/Public/Uploads/uploadfile/files/20220119/MMC5983MADatasheetRevA.pdf
// https://github.com/kriswiner/MMC5983MA/blob/master/LSM6DSM_MMC5983MA_LPS22HB_Dragonfly/MMC5983MA.cpp

#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"


#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

struct MMC5983_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b0110000);
    using Error = ImuError;
    template<typename T = void>
    using IResult = Result<T, Error>;

    using RegAddress = uint8_t;
    
    enum class BandWidth:uint8_t{
        _100Hz = 0b00,
        _200Hz = 0b01,
        _400Hz = 0b10,
        _800Hz = 0b11,
    };

    
    enum class Odr:uint8_t{
        SingleShot = 0b000,
        _1Hz = 0b001,
        _10Hz = 0b010,
        _20Hz = 0b011,
        _50Hz = 0b100,
        _100Hz = 0b101,
        _200Hz = 0b110,
        _1000Hz = 0b111,
    };

    enum class PrdSet:uint8_t{
        _1 = 0b000,
        _25 = 0b001,
        _75 = 0b010,
        _100 = 0b011,
        _250 = 0b100,
        _500 = 0b101,
        _1000 = 0b110,
        _2000 = 0b111
    };
};


struct MMC5983_Regs:public MMC5983_Prelude{
    struct alignas(sizeof(8)) DataPacket{
        static constexpr uint8_t address = 0;

        constexpr Vector3<int32_t> to_vec3() const{
            const int32_t x = (int32_t(buf_[0]) << 10) | (int32_t(buf_[1]) << 2) | (buf_[6] >> 6);
            const int32_t y = (int32_t(buf_[2]) << 10) | (int32_t(buf_[3]) << 2) | (buf_[6] >> 4);
            const int32_t z = (int32_t(buf_[4]) << 10) | (int32_t(buf_[5]) << 2) | (buf_[6] >> 2);

            return {x,y,z};
        }

        constexpr q16 to_temp() const {
            // Temperature output, unsigned format. The range is -75~125°C, about 0.8°C/LSB, 00000000 stands for -75°C
            // return q16(buf_[7] * 0.8_q16 - 75);
            return buf_[7];
        }
        constexpr std::span<uint8_t> as_bytes() {return std::span(buf_);}
    private:
        using Buf = std::array<uint8_t, 8>;

        Buf buf_;
    } data_packet_;

    static_assert(sizeof(DataPacket) == 8);

    struct R8_Status:public Reg8<>{
        static constexpr RegAddress address = 0x08;
        uint8_t meas_mag_done:1;
        uint8_t meas_temp_done:1;
        const uint8_t __resv1__:2 = 0;
        uint8_t otp_read_done:1;
        const uint8_t __resv2__:3 = 0;
    }DEF_R8(status_reg)

    struct R8_InternalControl0:public Reg8<>{
        static constexpr RegAddress address = 0x09;
        uint8_t tm_m:1;
        uint8_t tm_t:1;
        uint8_t int_meas_done_en:1;
        uint8_t set:1;
        uint8_t reset:1;
        uint8_t auto_sr_en:1;
        uint8_t otp_read:1;
        uint8_t :1;
    }DEF_R8(internal_control_0_reg)

    struct R8_InternalControl1:public Reg8<>{
        static constexpr RegAddress address = 0x0A;
        BandWidth bw:2;
        uint8_t x_inhibit:1;
        uint8_t yz_inhibit:2;
        const uint8_t __resv__:2 = 0;
        uint8_t sw_rst:1;
    }DEF_R8(internal_control_1_reg)


    struct R8_InternalControl2:public Reg8<>{
        static constexpr RegAddress address = 0x0B;
        Odr data_rate:3;
        uint8_t cmm_en:1;
        PrdSet prd_set:3;
        uint8_t en_prd_set:1 = 0;
    }DEF_R8(internal_control_2_reg)

    struct R8_ProductID:public Reg8<>{
        static constexpr RegAddress address = 0x2F;
        static constexpr uint8_t KEY = 0b00110000;
        uint8_t product_id:8;
    }DEF_R8(product_id_reg)
};

class MMC5983_Phy: public MMC5983_Prelude{
public:
    MMC5983_Phy(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv), spi_drv_(std::nullopt){;}
    MMC5983_Phy(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr):
        MMC5983_Phy(hal::I2cDrv{i2c, addr}){;}
    MMC5983_Phy(const hal::SpiDrv & spi_drv):
        i2c_drv_(std::nullopt), spi_drv_(spi_drv){;}
    MMC5983_Phy(Some<hal::Spi *> spi, const hal::SpiSlaveIndex index):
        spi_drv_(hal::SpiDrv{spi, index}){;}

    [[nodiscard]] __fast_inline
    IResult<> write_reg(const uint8_t addr, const uint8_t data){
        if(i2c_drv_){
            return IResult<>(i2c_drv_->write_reg(addr, data));
        }else if(spi_drv_){
            return Err(Error::SpiPhyIsNotSupportedYet);
        }else{
            return Err(Error::NoAvailablePhy);
        }
    }

    [[nodiscard]] __fast_inline
    IResult<> read_reg(const uint8_t addr, uint8_t & data){
        if(i2c_drv_){
            return IResult<>(i2c_drv_->read_reg(uint8_t(addr), data));
        }else if(spi_drv_){
            return Err(Error::SpiPhyIsNotSupportedYet);
        }else{
            return Err(Error::NoAvailablePhy);
        }
    }

    [[nodiscard]] __fast_inline
    IResult<> read_burst(const uint8_t addr, std::span<uint8_t> pbuf){
        if(i2c_drv_){
            return IResult<>(i2c_drv_->read_burst<uint8_t>(uint8_t(addr), pbuf));
        }else if(spi_drv_){
            return Err(Error::SpiPhyIsNotSupportedYet);
        }else{
            return Err(Error::NoAvailablePhy);
        }
    }

    
private:
    std::optional<hal::I2cDrv> i2c_drv_;
    std::optional<hal::SpiDrv> spi_drv_;
};

class MMC5983:
    public MagnetometerIntf,
    public MMC5983_Regs{
public:
    struct Config{
        PrdSet prd_set = PrdSet::_100;
        BandWidth bandwidth = BandWidth::_200Hz;
        Odr data_rate = Odr::_200Hz;
    };

    MMC5983(const hal::I2cDrv & i2c_drv):
        phy_(i2c_drv){;}
    MMC5983(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        phy_(hal::I2cDrv{i2c, addr}){;}
    MMC5983(const hal::SpiDrv & spi_drv):
        phy_(spi_drv){;}
    MMC5983(Some<hal::Spi *> spi, const hal::SpiSlaveIndex index):
        phy_(hal::SpiDrv{spi, index}){;}

    [[nodiscard]] IResult<> init(const Config & cfg);
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<> set_odr(const Odr odr);
    [[nodiscard]] IResult<> set_bandwidth(const BandWidth bw);
    [[nodiscard]] IResult<> enable_x(const Enable en);
    [[nodiscard]] IResult<> enable_yz(const Enable en);
    
    [[nodiscard]] IResult<Vector3<q24>> read_mag();
    [[nodiscard]] IResult<q16> read_temp();
    
    [[nodiscard]] IResult<bool> is_mag_meas_done();
    [[nodiscard]] IResult<bool> is_temp_meas_done();
    
    
    [[nodiscard]] IResult<> set_prd_magset(const PrdSet prdset);
    [[nodiscard]] IResult<> enable_magset(const Enable en);
    [[nodiscard]] IResult<> enable_magreset(const Enable en);
    [[nodiscard]] IResult<Vector3<q24>> do_magset();
    [[nodiscard]] IResult<Vector3<q24>> do_magreset();
    [[nodiscard]] IResult<> enable_auto_mag_sr(const Enable en);
    
    [[nodiscard]] IResult<> enable_mag_meas(const Enable en);
    [[nodiscard]] IResult<> enable_temp_meas(const Enable en);
private:    
    using Phy = MMC5983_Phy;
    Phy phy_;

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        const auto res = phy_.write_reg(reg.address, reg.as_val());
        if(res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return res;
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return phy_.read_reg(reg.address, reg.as_ref());
    }

    [[nodiscard]] IResult<> read_burst(const uint8_t addr, std::span<uint8_t> pbuf){
        return phy_.read_burst(addr, pbuf);
    }

};

}