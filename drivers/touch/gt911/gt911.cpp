#include "gt911.hpp"

// https://blog.csdn.net/qlexcel/article/details/99696108

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;

using Self = GT9XX;

using Error = Self::Error;

template<typename T = void>
using IResult = Result<T, Error>;

static constexpr auto VALID_PRODUCT_ID = 
    std::bit_cast<uint32_t>(std::to_array<char>({'9', '1', '1', '\0'}));

static constexpr uint8_t GT9147_CONFIG_TABLE[]={ 
	0X60, 0XE0, 0X01, 0X20, 0X03, 0X05, 0X35, 0X00, 0X02, 0X08,
	0X1E, 0X08, 0X50, 0X3C, 0X0F, 0X05, 0X00, 0X00, 0XFF, 0X67,
	0X50, 0X00, 0X00, 0X18, 0X1A, 0X1E, 0X14, 0X89, 0X28, 0X0A,
	0X30, 0X2E, 0XBB, 0X0A, 0X03, 0X00, 0X00, 0X02, 0X33, 0X1D,
	0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X32, 0X00, 0X00,
	0X2A, 0X1C, 0X5A, 0X94, 0XC5, 0X02, 0X07, 0X00, 0X00, 0X00,
	0XB5, 0X1F, 0X00, 0X90, 0X28, 0X00, 0X77, 0X32, 0X00, 0X62,
	0X3F, 0X00, 0X52, 0X50, 0X00, 0X52, 0X00, 0X00, 0X00, 0X00,
	0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
	0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X0F,
	0X0F, 0X03, 0X06, 0X10, 0X42, 0XF8, 0X0F, 0X14, 0X00, 0X00,
	0X00, 0X00, 0X1A, 0X18, 0X16, 0X14, 0X12, 0X10, 0X0E, 0X0C,
	0X0A, 0X08, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
	0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
	0X00, 0X00, 0X29, 0X28, 0X24, 0X22, 0X20, 0X1F, 0X1E, 0X1D,
	0X0E, 0X0C, 0X0A, 0X08, 0X06, 0X05, 0X04, 0X02, 0X00, 0XFF,
	0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
	0X00, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
	0XFF, 0XFF, 0XFF, 0XFF,
}; 

IResult<> Self::validate(){
    if(const auto res = write_reg(GT9XX_COMMAND_REG_ADDR, 0);
        res.is_err()) return Err(res.unwrap_err());

    std::array<uint8_t, 4> buf;
    if(const auto res = read_reg(GT9XX_PRODUCT_ID_REG_ADDR, buf);
        res.is_err()) return Err(res.unwrap_err());

    const auto product_id = std::bit_cast<uint32_t>(buf);

    if(product_id != VALID_PRODUCT_ID) 
        return Err(Error::UnexpectedProductId);

    return Ok();
}

IResult<> Self::init(){
    if(const auto res = validate(); 
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}


IResult<Option<Self::TouchPoint>> Self::get_touch_point(const Nth nth) {

    const auto num_touch_points = ({
        const auto res = get_num_touch_points();
        if (res.is_err()) 
            return Err(res.unwrap_err());
        res.unwrap();
    });
    
    std::array<uint8_t, TOUCHPOINT_ENTRY_LEN> buf;

    if (const auto res = read_reg(map_nth_to_addr(nth), buf);
        res.is_err()) return Err(res.unwrap_err());
    
    // clear status register
    if (const auto res = clear_status();
        res.is_err()) return Err(res.unwrap_err());

    const auto may_point = [&] -> Option<TouchPoint>{
        if (num_touch_points == 0) 
            return None;
        const auto decoded_point = TouchPoint::from_bytes(std::span(buf));
        return Some(decoded_point);
    }();

    return Ok(may_point);
}

IResult<> Self::clear_status(){
    return write_reg(GT9XX_TOUCHPOINT_STATUS_REG_ADDR, 0);
}

IResult<Self::TouchPoints> Self::get_touch_points() {
    
    const size_t num_touch_points = ({
        const auto res = get_num_touch_points();
        if (res.is_err())
            return Err(res.unwrap_err());
        res.unwrap();
    });
    
    TouchPoints points{};
    
    if (num_touch_points > 0) {
        if (num_touch_points > MAX_NUM_TOUCHPOINTS) {
            return Err(Error::UnexpectedData);
        }
        
        std::array<uint8_t, TOUCHPOINT_ENTRY_LEN * MAX_NUM_TOUCHPOINTS> buf;
        const auto read_size = TOUCHPOINT_ENTRY_LEN * num_touch_points;
        
        if (const auto res = read_reg(GT9XX_TOUCHPOINT_1_REG_ADDR, 
                std::span<uint8_t>(buf.data(), read_size));
            res.is_err()) return Err(res.unwrap_err());

        for (size_t n = 0; n < num_touch_points; n++) {
            const auto point = decode_specified_point(std::span(buf), Nth(n));
            points.push_back(point);
        }
    }
    
    // clear status register
    if (const auto res = clear_status();
        res.is_err()) return Err(res.unwrap_err());
        
    return Ok(points);
}

IResult<size_t> Self::get_num_touch_points() {
    // read_reg coords
    std::array<uint8_t, 1> buf;
    if (const auto res = read_reg(GT9XX_TOUCHPOINT_STATUS_REG_ADDR, buf);
        res.is_err()) return Err(res.unwrap_err());
        
    const uint8_t status = buf[0];
    const bool is_ready = static_cast<bool>(status & 0x80);
    const size_t num_touch_points = static_cast<size_t>(status & 0x0F);

    if (num_touch_points > MAX_NUM_TOUCHPOINTS)
        return Err(Error::UnexpectedData);

    if (not is_ready)
        return Err(Error::NotReady);

    return Ok(num_touch_points);
}