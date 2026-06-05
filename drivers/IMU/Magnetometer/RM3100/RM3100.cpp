#include "rm3100.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = RM3100::Error;

template<typename T = void>
using IResult = Result<T, Error>;



static constexpr int32_t three_bytes_to_i32(const uint8_t bytes[3]) {
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

    int32_t x = three_bytes_to_i32(&buf[0]);
    int32_t y = three_bytes_to_i32(&buf[3]);
    int32_t z = three_bytes_to_i32(&buf[6]);


    return Ok(math::Vec3<int32_t>(x,y,z));
}