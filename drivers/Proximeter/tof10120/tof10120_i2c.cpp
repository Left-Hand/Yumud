#include "tof10120_i2c.hpp"

using namespace ymd::drivers;


#define TOF10120_DEBUG_EN 0

#if TOF10120_DEBUG_EN
#define TOF10120_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define TOF10120_PANIC(...) PANIC(__VA_ARGS__)
#define TOF10120_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define TOF10120_DEBUG(...)
#define TOF10120_PANIC(...)  PANIC_NSRC()
#define TOF10120_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif


using namespace ymd;
using namespace ymd::drivers;

using Error = TOF10120::Error;

template<typename T = void>
using IResult = Result<T, Error>;


IResult<uint16_t> TOF10120::get_realtime_distance(){
    uint16_t count;
    if(const auto res = read_reg(RegAddr::RealtimeDistance, count);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(count);
}


IResult<uint16_t> TOF10120::get_filtered_distance(){
    uint16_t count;
    if(const auto res = read_reg(RegAddr::FilteredDistance, count);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(count);
}


IResult<int16_t> TOF10120::get_distance_diff(){
    int16_t count;
    if(const auto res = read_reg(RegAddr::DistanceDiff, count);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(count);
}


IResult<TOF10120::RangeMode> TOF10120::get_range_mode(){
    uint8_t count;
    if(const auto res = read_reg(RegAddr::RangeMode, count);
        res.is_err()) return Err(res.unwrap_err());

    switch(std::bit_cast<RangeMode>(count)){
        case RangeMode::Filtered:
        case RangeMode::Unfiltered:
            return Ok(std::bit_cast<RangeMode>(count));
    }
    return Err(Error::InvalidRangeMode);
}


IResult<TOF10120::CommPort> TOF10120::get_comm_port(){
    uint8_t count;
    if(const auto res = read_reg(RegAddr::CommPort, count);
        res.is_err()) return Err(res.unwrap_err());
    

    switch(std::bit_cast<CommPort>(count)){
        case CommPort::Serial:
        case CommPort::I2c:
            return Ok(std::bit_cast<CommPort>(count));
    }
    return Err(Error::InvalidCommPort);
}

