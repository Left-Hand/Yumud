#include "AS5047.hpp"
#include <bit>


using namespace ymd;
using namespace ymd::drivers;

using Error = AS5047::Error;

template<typename T = void>
using IResult = Result<T, Error>;






// https://blog.csdn.net/Mark_md/article/details/119645201
// SPI读角度流程：
// 1、AS5047P和MCU上电。
// 2、上电后至少延时等待tpon=10ms。才可以发送有效数据。
// 3、循环读 ANGLECOM寄存器(0x3FFF)，得到 uint16 的数据。PARD位校验无误后，将其转换为转子的实际角度。


IResult<> AS5047::write_reg(const Packet write_addr_pkt, const uint16_t reg_val){

    if(const auto res = spi_drv_.write_single<uint16_t>(write_addr_pkt.bits, CONT);
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = spi_drv_.write_single<uint16_t>(make_write_data_packet(reg_val).bits);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}


IResult<> AS5047::read_reg(Packet addr_pkt, uint16_t & reg_val){
    addr_pkt = writepkt_to_readpkt(addr_pkt);

    if(const auto res = spi_drv_.write_single<uint16_t>(addr_pkt.bits, CONT);
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = spi_drv_.write_single<uint16_t>(make_write_data_packet(reg_val).bits);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}