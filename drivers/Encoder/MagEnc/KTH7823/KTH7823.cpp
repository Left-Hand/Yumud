#include "KTH7823.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = KTH7823::Error;

template<typename T = void>
using IResult =  Result<T, Error>;

IResult<> KTH7823_Phy::transceive_u16(uint16_t & rx, const uint16_t tx){
    uint16_t dummy = 0;
    if(const auto res = spi_drv_.transceive_single<uint16_t>(dummy, tx);
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = spi_drv_.transceive_single<uint16_t>(rx, dummy);
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> KTH7823::update(){
    const auto res = phy_.direct_read(); 
    if(res.is_err()) return Err(res.unwrap_err());

    const auto data = res.unwrap();
    lap_position_ = u16_to_uni(data);

    return Ok();
}