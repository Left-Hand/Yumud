#include "mt6835.hpp"



using namespace ymd::drivers;
using namespace ymd;

using Self = MT6835;
using Error = Self::Error;

template<typename T = void>
using IResult = Result<T, Error>;


IResult<> Self::init() {
    return Ok();
}

uint16_t Self::get_position_data(){
    return 0;
}

IResult<> Self::update() {
    return Ok();
}


static constexpr uint16_t make_header(const Self::Command cmd, const Self::RegAddr reg_addr){
    return (static_cast<uint16_t>(cmd) << 12) | static_cast<uint16_t>(reg_addr);
}

IResult<> Self::write_reg(const RegAddr addr, const uint8_t data){

    const auto header = make_header(Command::Write, addr);

    if(const auto res = spi_drv_.write_single<uint16_t>(header, CONT);
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = spi_drv_.write_single<uint8_t>(data);
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}


IResult<> Self::read_reg(const RegAddr addr, uint8_t & data){

    const auto header = make_header(Command::Read, addr);

    if(const auto res = spi_drv_.write_single<uint16_t>(header, CONT);
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = spi_drv_.read_single<uint8_t>(data);
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}