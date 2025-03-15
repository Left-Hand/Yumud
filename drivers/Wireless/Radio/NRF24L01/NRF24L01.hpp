//这个驱动还在推进状态


#include "drivers/device_defs.h"
#include "sys/utils/result.hpp"


namespace ymd::drivers{

class NRF24L01{
public:
    using Error = BusError;

    enum class Package{
        NRF24L01,
    };
protected:
    using SpiDrvProxy = std::optional<hal::SpiDrv>;
    SpiDrvProxy p_spi_drv_ = std::nullopt;

    using RegAddress = uint8_t;    



    [[nodiscard]] Result<void, BusError> writeReg(const uint8_t addr, const uint8_t data);

    template<typename T>
    [[nodiscard]] Result<void, BusError> writeReg(const T & reg){return writeReg(reg.address, reg);}

    [[nodiscard]] Result<void, BusError> readReg(const uint8_t addr, uint8_t & data);

    template<typename T>
    [[nodiscard]] Result<void, BusError> readReg(T & reg){return readReg(reg.address, reg);}

    [[nodiscard]] Result<void, BusError> readBurst(const uint8_t reg_addr, int16_t * datas, const size_t len);
    

    public:
    NRF24L01(const hal::SpiDrv spi_drv):p_spi_drv_(spi_drv){;}
    NRF24L01(const NRF24L01 & other) = delete;
    NRF24L01(NRF24L01 && other) = delete;

    Result<void, Error> verify();

    Result<void, Error> init();
    
    Result<void, Error> update();


};

};

namespace ymd::custom{
    template<typename T>
    struct result_converter<T, drivers::NRF24L01::Error, BusError> {
        static Result<T, drivers::NRF24L01::Error> convert(const BusError berr){
            using Error = drivers::NRF24L01::Error;
            using BusError = BusError;
            
            if(berr.ok()) return Ok();

            Error err = [](const BusError berr_){
                switch(berr_.type){
                    // case BusError::NO_ACK : return Error::I2C_NOT_ACK;

                    // case BusError::I2C_NOT_READY: return NRF24L01::Error::I2C_NOT_READY;
                    default: return Error::UNSPECIFIED;
                }
            }(berr);

            return Err(err); 
        }
    };
}