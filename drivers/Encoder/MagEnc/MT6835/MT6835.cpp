#include "MT6835.hpp"


using namespace ymd::drivers;
using namespace ymd;

using Error = MT6835::Error;

template<typename T = void>
using IResult = Result<T, Error>;


IResult<> MT6835::init() {
    return Ok();
}

uint16_t MT6835::get_position_data(){
    return 0;
}

IResult<> MT6835::update() {
    return Ok();
}


struct Frame{
    enum class Type:uint16_t{
        Write = 0b0110,
        Read = 0b0011,
    };


    uint16_t addr:12;
    Type type:4;

    operator uint16_t & (){return *reinterpret_cast<uint16_t *>(this);}
    operator uint16_t () const {return *reinterpret_cast<const uint16_t *>(this);}
};

static_assert(sizeof(Frame) == 2);


IResult<> MT6835::write_reg(const RegAddr addr, const uint8_t data){

    Frame format = {
        .addr = addr,
        .type = Frame::Type::Write
    };

    if(const auto res = spi_drv_.write_single<uint16_t>(format, CONT);
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = spi_drv_.write_single<uint8_t>(data);
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}


IResult<> MT6835::read_reg(const RegAddr addr, uint8_t & data){

    Frame format = {
        .addr = addr,
        .type = Frame::Type::Read
    };

    if(const auto res = spi_drv_.write_single<uint16_t>(format, CONT);
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = spi_drv_.read_single<uint8_t>(data);
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}