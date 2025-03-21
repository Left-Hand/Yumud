#pragma once

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "hal/gpio/gpio_intf.hpp"

namespace ymd::drivers{

enum class LT8960L_Error:uint8_t{
    TransmitTimeout,
    PacketOverlength,
    ChipIdMismatch,
    InvalidState,
    Unspecified = 0xff
};

    

class LT8960L_Phy:public hal::ProtocolBusDrv<hal::I2c> {
public:
    static constexpr uint8_t DEFAULT_I2C_ADDR = 0x1A;
    using Error = LT8960L_Error;

public:
    LT8960L_Phy(hal::Gpio * scl, hal::Gpio * sda):
        hal::ProtocolBusDrv<hal::I2c>(bus_inst_, DEFAULT_I2C_ADDR), bus_inst_(hal::I2cSw(*scl, *sda)){};

    [[nodiscard]] Result<void, Error> init();

    [[nodiscard]] Result<void, Error> write_reg(uint8_t address, uint16_t data);

    [[nodiscard]] Result<void, Error> read_reg(uint8_t address, uint16_t & data);

    [[nodiscard]] Result<size_t, Error> read_burst(uint8_t address, std::span<std::byte> pbuf);

    [[nodiscard]] Result<size_t, Error> write_burst(uint8_t address, std::span<const std::byte> pbuf);

    [[nodiscard]] Result<void, Error> start_hw_listen_pkt();

    [[nodiscard]] Result<bool, Error> check_and_skip_hw_listen_pkt();

    [[nodiscard]] Result<void, Error> wait_pkt_ready(const uint timeout);
private:
    hal::I2cSw bus_inst_;
    [[nodiscard]] Result<void, Error> _write_reg(uint8_t address, uint16_t data);

    [[nodiscard]] Result<void, Error> _read_reg(uint8_t address, uint16_t & data);
};
}