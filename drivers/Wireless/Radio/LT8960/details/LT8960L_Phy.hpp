#pragma once

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "hal/gpio/gpio_intf.hpp"

namespace ymd::drivers{

class LT8960L_Error{
public:
    enum Kind:uint8_t{
        TransmitTimeout,
        PacketOverlength,
        ChipIdMismatch,
        ReceiveTimeout,
        InvalidState,
        Unspecified = 0xff
    };

    constexpr LT8960L_Error(const Kind kind):
        kind_(kind){;}

    constexpr bool operator ==(const LT8960L_Error & rhs) const {return kind_ == rhs.kind_;}
private:
    Kind kind_;
};




class LT8960L_Phy final{
public:
    static constexpr uint8_t DEFAULT_I2C_ADDR = 0x1A;
    using Error = LT8960L_Error;

public:
    LT8960L_Phy(hal::Gpio & scl_io, hal::Gpio & sda_io):
        i2c_(hal::I2cSw(scl_io, sda_io)){};

    [[nodiscard]] Result<void, Error> init();

    [[nodiscard]] Result<void, Error> write_reg(uint8_t address, uint16_t data);

    [[nodiscard]] Result<void, Error> read_reg(uint8_t address, uint16_t & data);

    [[nodiscard]] Result<size_t, Error> read_burst(uint8_t address, std::span<std::byte> pbuf);

    [[nodiscard]] Result<size_t, Error> write_burst(uint8_t address, std::span<const std::byte> pbuf);

    [[nodiscard]] Result<void, Error> start_hw_listen_pkt();

    [[nodiscard]] Result<bool, Error> check_and_skip_hw_listen_pkt();

    [[nodiscard]] Result<void, Error> wait_pkt_ready(const uint timeout);
private:
    hal::I2cSw i2c_;
    [[nodiscard]] Result<void, Error> _write_reg(uint8_t address, uint16_t data);

    [[nodiscard]] Result<void, Error> _read_reg(uint8_t address, uint16_t & data);
};
}