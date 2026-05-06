#include "rm3100.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = RM3100::Error;

template<typename T = void>
using IResult = Result<T, Error>;



static constexpr int32_t three_bytes_to_i32(const std::span<const uint8_t, 3> bytes) {
    uint32_t prefix = (bytes[0] & 0x80) != 0 ? (static_cast<uint32_t>(0xff) << 24) : 0;
    return static_cast<int32_t>(
        prefix | 
        (static_cast<uint32_t>(bytes[0]) << 16) | 
        (static_cast<uint32_t>(bytes[1]) << 8) | 
        static_cast<uint32_t>(bytes[2])
    );
}

IResult<math::Vec3<int32_t>> RM3100::get_mag_i32(){
    uint8_t buf[9];

    if(const auto res = transport_.read_bulk(RegAddr::MX, std::span(buf));
        res.is_err()) return Err(res.unwrap_err());

    auto [x2,x1,x0,y2,y1,y0,z2,z1,z0] = buf;
    int32_t x, y, z;
    //special bit manipulation since there is not a 24 bit signed int data type
    if (x2 & 0x80){
        x = 0xFF;
    }
    if (y2 & 0x80){
        y = 0xFF;
    }
    if (z2 & 0x80){
        z = 0xFF;
    }

    //format results into single 32 bit signed value
    x = (x << 24) | (int32_t)(x2) << 16 | (uint16_t)(x1) << 8 | x0;
    y = (y << 24) | (int32_t)(y2) << 16 | (uint16_t)(y1) << 8 | y0;
    z = (z << 24) | (int32_t)(z2) << 16 | (uint16_t)(z1) << 8 | z0;

    return Ok(math::Vec3<int32_t>(x,y,z));
}