#pragma once

#include "hal/bus/spi/spidrv.hpp"

#include "drivers/Encoder/MagEncoder.hpp"
#include "core/io/regs.hpp"

namespace ymd::drivers{

class AS5047:public MagEncoderIntf{
public:
    using Error = EncoderError;

    template<typename T = void>
    using IResult = Result<T, Error>;

public:
    AS5047(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    AS5047(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
    AS5047(Some<hal::Spi *> spi, const hal::SpiSlaveIndex index):
        spi_drv_(hal::SpiDrv{spi, index}){;}

    [[nodiscard]]IResult<> init() ;

    [[nodiscard]]IResult<> update();
    [[nodiscard]]Result<real_t, Error> get_lap_position() {
        return Ok(lap_position);
    }
    uint32_t get_err_cnt() const {return errcnt;}
private:
protected:

    using RegAddress = uint16_t;

    struct ErrflReg:public Reg8<>{
        scexpr RegAddress address = 0x001;
        uint8_t frame_error:1;
        uint8_t invalid_cmd_error:1;
        uint8_t parity_error:1;
        uint8_t :5;
    };

    struct ProgReg:public Reg8<>{
        scexpr RegAddress address = 0x002;

        uint8_t prog_otp_en:1;
        uint8_t otp_reflash:1;
        uint8_t start_otp_prog:1;
        uint8_t prog_verify:1;
        uint8_t :4;
    };


    hal::SpiDrv spi_drv_;

    real_t lap_position;
    size_t errcnt = 0;
    bool fast_mode = true;

    uint16_t get_position_data();

    IResult<> write_reg(const RegAddress addr, const uint8_t data);
    IResult<> read_reg(const RegAddress addr, uint8_t & data);

};

};