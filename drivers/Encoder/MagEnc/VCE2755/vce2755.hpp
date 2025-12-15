#pragma once

#include "vce2755_prelude.hpp"

namespace ymd::drivers{

class VCE2755 final:
    public VCE2755_Prelude,
    public MagEncoderIntf{
public:
    struct Config{
        ClockDirection direction;
    };

    explicit VCE2755(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit VCE2755(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
    explicit VCE2755(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv(spi, rank)){;}


    [[nodiscard]] IResult<> init(const Config & cfg);
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> set_zero_angle(const Angular<q31> angle);
    [[nodiscard]] IResult<Angular<q31>> read_lap_angle(){
        return Ok(Angular<q31>::from_turns(lap_turns_));
    }

private:
    hal::SpiDrv spi_drv_;
    VCE2755_Regset regs_ = {};
    q31 lap_turns_ = 0;

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        const auto address = T::ADDRESS;
        const uint8_t data = reg.to_bits();
        const auto tx = uint16_t(
            0x8000 | (std::bit_cast<uint8_t>(address) << 8) | data);
        if(const auto res = spi_drv_.write_single<uint16_t>(tx);
            res.is_err()) return Err(Error(res.unwrap_err()));
        reg.apply();
        return Ok();
    }


    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        uint16_t dummy;
        const auto addr = std::bit_cast<uint8_t>(T::ADDRESS);
        const auto tx = uint16_t(0x4000 | ((uint8_t)addr << 8));
        if(const auto res = spi_drv_.write_single<uint16_t>(tx); 
            res.is_err()) return Err(Error(res.unwrap_err()));
        if(const auto res = spi_drv_.read_single<uint16_t>(dummy);
            res.is_err()) return Err(Error(res.unwrap_err()));
        if((dummy & 0xff) != 0x00) 
            return Err(Error(Error::Kind::InvalidRxFormat));
        reg.as_bits_mut() = (dummy >> 8);
        return Ok();
    }

    [[nodiscard]] IResult<Packet> read_packet() {
        #if 1
        static constexpr std::array<uint8_t, 4> tx = {0x83, 0x00, 0x00, 0x00};
        std::array<uint8_t, 4> rx;
        if(const auto res = spi_drv_.transceive_burst<uint8_t>(rx, tx);
            res.is_err()) return Err(Error(res.unwrap_err()));
        // DEBUG_PRINTLN(rx);
        return Ok(Packet::from_bytes(rx[1], rx[2], rx[3]));
        #else
        //exprimental
        static constexpr std::array<uint16_t, 2> tx = {0x8300, 0x0000};
        std::array<uint16_t, 2> rx;
        if(const auto res = spi_drv_.transceive_burst<uint16_t>(rx, tx);
            res.is_err()) return Err(Error(res.unwrap_err()));
        return Ok(Packet::from_u24(static_cast<uint32_t>(rx[0] >> 8) | static_cast<uint32_t>(rx[1] << 8)));
        #endif
    }


};

}