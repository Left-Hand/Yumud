#include "gt911.hpp"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;

using Error = GT911::Error;

template<typename T = void>
using IResult = Result<T, Error>;


IResult<> GT911::validate(){
    if(const auto res = write(GT911_COMMAND_REG, 0);
        res.is_err()) return Err(res.unwrap_err());

    std::array<uint8_t, 4> buf;
    if(const auto res = read(GT911_PRODUCT_ID_REG, buf);
        res.is_err()) return Err(res.unwrap_err());

    const auto product_id = std::bit_cast<uint32_t>(buf);
    static constexpr auto VALID_PRODUCT_ID = 
        std::bit_cast<uint32_t>(std::to_array<char>({'9', '1', '1', '\0'}));

    if(product_id != VALID_PRODUCT_ID) return Err(Error::UnexpectedProductId);

    return Ok();
}

IResult<> GT911::init(){
    if(const auto res = validate(); 
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}


IResult<Option<GT911::TouchPoint>> GT911::get_touch_point(const Nth nth) {

    const auto num_touch_points = ({
        const auto res = get_num_touch_points();
        if (res.is_err()) {
            return Err(res.unwrap_err());
        }
        res.unwrap();
    });
    
    std::array<uint8_t, TOUCHPOINT_ENTRY_LEN> buf;

    if (const auto res = read(map_nth_to_addr(nth), buf);
        res.is_err()) return Err(res.unwrap_err());

    Option<TouchPoint> may_point = None;
    if (num_touch_points > 0) {
        const auto decoded_point = decode_point(std::span(buf));
        may_point = Some(decoded_point);
    }
    
    // clear status register
    if (const auto res = write(GT911_TOUCHPOINT_STATUS_REG, 0);
        res.is_err()) return Err(res.unwrap_err());
        
    return Ok(may_point);
}

// IResult<TouchPoint> GT911::get_touch_point_unchecked(const Nth nth);

/// Gets multiple stack allocated touch points (0-5 points)
/// Returns points.len()==0 for release, points.len()>0 for press or move and Err(Error::NotReady) for no data
IResult<GT911::TouchPoints> GT911::get_touch_points() {
    
    const size_t num_touch_points = ({
        const auto res = get_num_touch_points();
        if (res.is_err()) {
            return Err(res.unwrap_err());
        }
        res.unwrap();
    });
    
    TouchPoints points{};
    
    if (num_touch_points > 0) {
        if (num_touch_points > MAX_NUM_TOUCHPOINTS) {
            return Err(Error::UnexpectedData);
        }
        
        std::array<uint8_t, TOUCHPOINT_ENTRY_LEN * MAX_NUM_TOUCHPOINTS> buf;
        const auto read_size = TOUCHPOINT_ENTRY_LEN * num_touch_points;
        
        if (const auto res = read(GT911_TOUCHPOINT_1_REG, 
                std::span<uint8_t>(buf.data(), read_size));
            res.is_err()) return Err(res.unwrap_err());

        for (size_t n = 0; n < num_touch_points; n++) {
            const auto point = map_buf_to_point(std::span(buf), Nth(n));
            points.push_back(point);
        }
    }
    
    // clear status register
    if (const auto res = write(GT911_TOUCHPOINT_STATUS_REG, 0);
        res.is_err()) return Err(res.unwrap_err());
        
    return Ok(points);
}

IResult<size_t> GT911::get_num_touch_points() {
    // read coords
    std::array<uint8_t, 1> buf;
    if (const auto res = read(GT911_TOUCHPOINT_STATUS_REG, buf);
        res.is_err()) return Err(res.unwrap_err());
        
    const auto status = buf[0];
    const auto ready = (status & 0x80) > 0;
    const auto num_touch_points = static_cast<size_t>(status & 0x0F);
    
    if (ready) {
        return Ok(num_touch_points);
    } else {
        return Err(Error::NotReady);
    }
}