#include "mt6825.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Self = MT6825;
using Error = Self::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<Angular<uq32>> Self::read_lap_angle(){
    if(const auto res = read_packet();
        res.is_err()) return Err(res.unwrap_err());
    else{
        const auto & packet = res.unwrap();
        // DEBUG_PRINTLN(std::hex, packet.as_bytes());
        return packet.parse();
    }
}

[[nodiscard]] IResult<Self::Packet> Self::read_packet(){
    #if 1
    static constexpr std::array<uint16_t, 2> tx = {0x8300, 0x0000};
    std::array<uint16_t, 2> rx;
    if(const auto res = spi_drv_.transceive_burst<uint16_t>(rx, tx);
        res.is_err()) return Err(Error(res.unwrap_err()));
    return Ok(Packet::from_bytes(
        static_cast<uint8_t>(rx[0]),
        static_cast<uint8_t>(rx[1] >> 8),
        static_cast<uint8_t>(rx[1])
    ));
    #else

    //legacy
    static constexpr std::array<uint8_t, 4> tx = {0x83, 0x00, 0x00, 0x00};
    std::array<uint8_t, 4> rx;
    if(const auto res = spi_drv_.transceive_burst<uint8_t>(rx, tx);
        res.is_err()) return Err(Error(res.unwrap_err()));
    return Ok(Packet::from_bytes(rx[1], rx[2], rx[3]));
    #endif
}