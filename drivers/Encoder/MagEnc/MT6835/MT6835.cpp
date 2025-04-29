#include "MT6835.hpp"


using namespace ymd::drivers;
using namespace ymd;

using Error = MT6835::Error;

template<typename T = void>
using IResult = Result<T, Error>;


void MT6835::init() {

}

uint16_t MT6835::getPositionData(){
    return 0;
}

IResult<> MT6835::update() {
    return Ok();
}


struct WRFormat{
    uint16_t addr:12;
    uint16_t type:4;

    operator uint16_t & (){return *reinterpret_cast<uint16_t *>(this);}
    operator uint16_t () const {return *reinterpret_cast<const uint16_t *>(this);}
};



hal::HalResult MT6835::write_reg(const RegAddress addr, const uint8_t data){

    WRFormat format = {
        .addr = addr,
        .type = 0b0110
    };

    return spi_drv_.write_single<uint16_t>(format, CONT)
    | spi_drv_.write_single<uint8_t>(data);
}


hal::HalResult MT6835::read_reg(const RegAddress addr, uint8_t & data){

    WRFormat format = {
        .addr = addr,
        .type = 0b0011
    };

    return spi_drv_.write_single<uint16_t>(format, CONT)
    | spi_drv_.read_single<uint8_t>(data);
}