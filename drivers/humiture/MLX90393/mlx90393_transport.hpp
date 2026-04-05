#pragma once

#include "mlx90393_prelude.hpp"


namespace ymd::drivers{


class MLX90393_TransportIntf:public MLX90393_Prelude{
public:
    virtual IResult<> transceive(std::span<uint8_t> rx_pbuf, std::span<const uint8_t> tx_pbuf) = 0;
};



class MLX90393_I2cTransport final:public MLX90393_TransportIntf{

public:
    MLX90393_I2cTransport(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        MLX90393_I2cTransport(hal::I2cDrv(i2c, i2c_addr)){;}

    IResult<> transceive(std::span<uint8_t> rx_pbuf, std::span<const uint8_t> tx_pbuf);

private:
    MLX90393_I2cTransport(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    hal::I2cDrv i2c_drv_;
};



class MLX90393_SpiTransport final:public MLX90393_TransportIntf{
public:
    MLX90393_SpiTransport(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        MLX90393_SpiTransport(hal::SpiDrv(spi, rank)){;}

    IResult<> transceive(std::span<uint8_t> rx_pbuf, std::span<const uint8_t> tx_pbuf);

private:
    MLX90393_SpiTransport(hal::SpiDrv && spi_drv):spi_drv_(std::move(spi_drv)){;}

    hal::SpiDrv spi_drv_;
};

}