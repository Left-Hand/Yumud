#pragma once

#include "hal/bus/spi/spidrv.hpp"

#include "drivers/Encoder/MagEncoder.hpp"
#include "core/io/regs.hpp"

namespace ymd::drivers{

struct AS5047_Prelude{

    using RegAddr = uint16_t;
    using Error = EncoderError;

};

struct AS5047_Regs:public AS5047_Prelude{

    // static constexpr RegAddr MAG_ENC_REG_ADDR = 0x3FF;
    struct ErrflReg:public Reg8<>{
        static constexpr RegAddr address = 0x001;
        uint8_t frame_error:1;
        uint8_t invalid_cmd_error:1;
        uint8_t parity_error:1;
        uint8_t :5;
    };

    struct ProgReg:public Reg8<>{
        static constexpr RegAddr address = 0x002;

        uint8_t prog_otp_en:1;
        uint8_t otp_reflash:1;
        uint8_t start_otp_prog:1;
        uint8_t prog_verify:1;
        uint8_t :4;
    };

};

class AS5047:
    public MagEncoderIntf,
    public AS5047_Prelude{
public:


    template<typename T = void>
    using IResult = Result<T, Error>;

public:
    explicit AS5047(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit AS5047(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
    explicit AS5047(Some<hal::Spi *> spi, const hal::SpiSlaveRank index):
        spi_drv_(hal::SpiDrv{spi, index}){;}

    [[nodiscard]] IResult<> init() ;

    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<Angle<q31>> read_lap_angle() {
        return Ok(Angle<q31>::from_turns(lap_position_));
    }
    [[nodiscard]] uint32_t get_err_cnt() const {return crc_err_cnt_;}
private:
    using Regs = AS5047_Regs;

    hal::SpiDrv spi_drv_;
    Regs regs_;

    real_t lap_position_ = 9;
    size_t crc_err_cnt_ = 0;
    bool fast_mode_ = true;

    uint16_t get_position_data();

    [[nodiscard]] IResult<> write_reg(const RegAddr addr, const uint8_t data);
    [[nodiscard]] IResult<> read_reg(const RegAddr addr, uint8_t & data);

};

};