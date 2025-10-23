#pragma once

// https://github.com/hnguy169/RM3100-Arduino
// https://github.com/lmnop100/RM3100-Driver/blob/master/QRM3100.h

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/gpio/vport.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "concept/pwm_channel.hpp"
#include "types/vectors/vector3.hpp"


namespace ymd::drivers{

struct RM3100_Prelude{

static constexpr uint8_t QRM3100_CMM  		= 0x01; //initiates continuous measurement mode
static constexpr uint8_t QRM3100_CCXMSB  		= 0x04; //cycle count regsiter for x-axis (most significant bit)
static constexpr uint8_t QRM3100_CCXLSB			= 0x05;
static constexpr uint8_t QRM3100_CCYMSB  		= 0x06; // ... for y-axis
static constexpr uint8_t QRM3100_CCYLSB  		= 0x07;
static constexpr uint8_t QRM3100_CCZMSB  		= 0x08; // ... for z-axis
static constexpr uint8_t QRM3100_CCZLSB  		= 0x09;
static constexpr uint8_t QRM3100_NOS_REG			= 0x0A;
static constexpr uint8_t QRM3100_TMRC 		= 0x0B; //sets continuous measurement mode data rate

static constexpr uint8_t QRM3100_MX2   		= 0xA4; // measurement results for x-axis
static constexpr uint8_t QRM3100_MX1   		= 0xA5;
static constexpr uint8_t QRM3100_MX0   		= 0xA6;
static constexpr uint8_t QRM3100_MY2   		= 0xA7; // measurement results for y-axis
static constexpr uint8_t QRM3100_MY1   		= 0xA8;
static constexpr uint8_t QRM3100_MY0   		= 0xA9;
static constexpr uint8_t QRM3100_MZ2   		= 0xAA; // measurement results for z-axis
static constexpr uint8_t QRM3100_MZ1   		= 0xAB;
static constexpr uint8_t QRM3100_MZ0   		= 0xAC;
static constexpr uint8_t QRM3100_STATUS_REG   	= 0xB4; //status of DRDY
static constexpr uint8_t QRM3100_I2C_ADDRESS   	= 0x20; //Address of slave device
static constexpr uint8_t QRM3100_POLL		= 0x00; //poll

static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x20); // Hexadecimal slave address for RM3100 with Pin 2 and Pin 4 set to LOW

//internal register values without the R/W bit
static constexpr uint8_t RM3100_REVID_REG       = 0x36; // Hexadecimal address for the Revid internal register
static constexpr uint8_t RM3100_POLL_REG        = 0x00; // Hexadecimal address for the Poll internal register
static constexpr uint8_t RM3100_CMM_REG         = 0x01; // Hexadecimal address for the CMM internal register
static constexpr uint8_t RM3100_STATUS_REG      = 0x34; // Hexadecimal address for the Status internal register

static constexpr uint8_t RM3100_CCX1_REG        = 0x04; // Hexadecimal address for Cycle Count X1 internal register
static constexpr uint8_t RM3100_CCX0_REG        = 0x05; // Hexadecimal address for the Cycle Count X0 internal register
static constexpr uint8_t RM3100_CCY1_REG        = 0x04; // Hexadecimal address for Cycle Count Y1 internal register
static constexpr uint8_t RM3100_CCY0_REG        = 0x05; // Hexadecimal address for the Cycle Count Y0 internal register
static constexpr uint8_t RM3100_CCZ1_REG        = 0x04; // Hexadecimal address for Cycle Count Z1 internal register
static constexpr uint8_t RM3100_CCZ0_REG        = 0x05; // Hexadecimal address for the Cycle Count Z0 internal register

//options
static constexpr auto initialCC = 200; // Set the cycle count
static constexpr auto singleMode = 0; //0 = use continuous measurement mode; 1 = use single measurement mode
static constexpr auto useDRDYPin = 1; //0 = not using DRDYPin ; 1 = using DRDYPin to wait for data

//   gain = (0.3671 * (float)cycleCount) + 1.5; //linear equation to calculate the gain from cycle count

enum class Error_Kind:uint8_t{
    WrongChipId,
    IndexOutOfRange
};

DEF_FRIEND_DERIVE_DEBUG(Error_Kind)
DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

template<typename T = void>
using IResult = Result<T, Error>;

using RegAddr = uint8_t;

};


struct RM3100_Phy:public RM3100_Prelude{
    explicit RM3100_Phy(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){}

    IResult<> write_reg(RegAddr addr, uint8_t data){
        if(const auto res = i2c_drv_.write_reg(addr, data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_reg(RegAddr addr, uint8_t & data){
        if(const auto res = i2c_drv_.read_reg(addr, data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_burst(RegAddr addr, std::span<uint8_t> pbuf){
        if(const auto res = i2c_drv_.read_burst<uint8_t>(addr, pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
private:
    hal::I2cDrv i2c_drv_;
};

struct RM3100:public RM3100_Prelude{
    explicit RM3100(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        phy_(hal::I2cDrv(i2c, addr)){}


    IResult<bool> is_conv_done(){
        uint8_t status;
        if(const auto res = phy_.read_reg(RM3100_STATUS_REG, status);
            res.is_err()) return Err(res.unwrap_err());
        return Ok(status & 0x80);
    }

    IResult<Vec3<int32_t>> get_mag_i32(){
        uint8_t buf[9];

        if(const auto res = phy_.read_burst(0x24, std::span(buf));
            res.is_err()) return Err(res.unwrap_err());

        auto [x2,x1,x0,y2,y1,y0,z2,z1,z0] = buf;
        int32_t x, y, z;
        //special bit manipulation since there is not a 24 bit signed int data type
        if (x2 & 0x80){
            x = 0xFF;
        }
        if (y2 & 0x80){
            y = 0xFF;
        }
        if (z2 & 0x80){
            z = 0xFF;
        }

        //format results into single 32 bit signed value
        x = (x << 24) | (int32_t)(x2) << 16 | (uint16_t)(x1) << 8 | x0;
        y = (y << 24) | (int32_t)(y2) << 16 | (uint16_t)(y1) << 8 | y0;
        z = (z << 24) | (int32_t)(z2) << 16 | (uint16_t)(z1) << 8 | z0;

        return Ok(Vec3<int32_t>(x,y,z));
    }
private:
    RM3100_Phy phy_;
};

}
