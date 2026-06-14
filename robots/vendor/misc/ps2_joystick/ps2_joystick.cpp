#include "ps2_joystick.hpp"



using namespace ymd;
using namespace ymd::drivers;

using Error = Ps2Joystick::Error;

template<typename T = void>
using IResult = Result<T, Error>;


enum class Command:uint8_t{
    RequestData = 0x42
};

IResult<> Ps2Joystick::init(){
    return Ok();
}


IResult<> Ps2Joystick::update(){

    if(const auto res = spi_drv_.write_single<uint8_t>((uint8_t)0x01);
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = spi_drv_.transceive_single<uint8_t>(
        reinterpret_cast<uint8_t &>(dev_id_),
        std::bit_cast<uint8_t>(Command::RequestData));

        res.is_err()) return Err(res.unwrap_err());

    uint8_t permit;

    if(const auto res = spi_drv_.transceive_single<uint8_t>(permit, uint8_t(0x00));
        res.is_err()) return Err(res.unwrap_err());

    const auto tx_packet = TxPacket::from_parts(0, 0);
    RxPacket rx_packet;

    if(const auto res = spi_drv_.transceive_burst<uint8_t>(
        rx_packet.as_bytes_mut(),
        tx_packet.as_bytes()
    );  res.is_err())
        return Err(res.unwrap_err());

    if(permit == 0x5a){
        rx_packet_ = rx_packet;
    }else{
        // PANIC{permit};
    }

    return Ok();
}

IResult<Ps2Joystick::RxPacket> Ps2Joystick::read_info() const noexcept {
    switch(dev_id_){
        case DevId::Digit:
            return Err(Error::CantParseAtDigitMode);
        case DevId::AnalogRed:
            return Ok(rx_packet_);
        default:
            break;
    }
    PANIC{dev_id_};
    return Err(Error::Unreachable);
}

