#include "ST1615.hpp"


using namespace ymd;
using namespace ymd::drivers;

using Error = ST1615::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> ST1615::init(){
    if(const auto res = this->blocking_until_normal_status();
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

Result<ST1615::GestureInfo, Error> ST1615::get_gesture_info(){
    const uint8_t raw = ({
        const auto res = this->read_reg8(ADVANCED_TOUCH_INFO);
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });

    return Ok(GestureInfo {
        // gesture_type: GestureType::from_u8(raw),
        gesture_type: GestureType::DoubleTab,
        proximity: (raw & 0b0100'0000) != 0,
        water: (raw & 0b0010'0000) != 0,
    });
}

Result<Option<ST1615::Point>, Error> ST1615::get_point(uint8_t nth) {
    if (nth > 9) {
        return Err(Error::PointRankOutOfRange); // 最多支持 10 个点
    }

    uint8_t start_reg = 0x12 + 4 * nth;
    uint8_t buf[4] = {0};

    auto res = read_u8(start_reg, std::span(buf));
    if (res.is_err()) {
        return Err(res.unwrap_err());
    }

    if ((buf[0] >> 7) == 0) {
        return Ok(None); // 无效点
    } else {
        uint16_t x = ((static_cast<uint16_t>(buf[0] & 0b01110000)) << 4) | buf[1];
        uint16_t y = ((static_cast<uint16_t>(buf[0] & 0b00001111)) << 8) | buf[2];
        return Ok(Some(Point{x, y}));
    }
}


/// Sensing Counter Registers provide a frame-based scan counter for host to verify current scan rate.
Result<uint16_t, Error> ST1615::get_sensor_count() {
    uint8_t buf[2] = {0};
    const auto res = read_u8(SENSING_COUNTER_L, std::span(buf));
    if (res.is_err()) {
        return Err(res.unwrap_err());
    }
    uint16_t count = (static_cast<uint16_t>(buf[0]) << 8) | buf[1];
    return Ok(count);
}


Result<ST1615::Capabilities, Error> ST1615::get_capabilities() {
    const uint8_t max_contacts = ({
        const auto res = read_reg8(CONTACT_COUNT_MAX);
        if (res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });

    const uint8_t misc_info = ({
        const auto res = read_reg8(MISC_INFO);
        if (res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });

    std::array<uint8_t, 3> buf = {0};
    
    if (const auto res = read_u8(XY_RESOLUTION_H, std::span(buf));
        res.is_err()) {
        return Err(res.unwrap_err());
    }

    const uint16_t x_res = ((static_cast<uint16_t>(buf[0]) & 0b01110000) << 4) | buf[1];
    const uint16_t y_res = ((static_cast<uint16_t>(buf[0]) & 0b00001111) << 8) | buf[2];

    Capabilities caps = {
        max_contacts,
        x_res,
        y_res,
        (misc_info & 0b10000000) != 0
    };

    return Ok(caps);
}

IResult<> ST1615::blocking_until_normal_status(){
    uint8_t status;
    size_t times = 0;
    do {
        const auto res = this->read_reg8(STATUS);
        if(res.is_err()) return Err(res.unwrap_err());
        status = res.unwrap();
        times++;
        if(times > MAX_BLOCKING_TIMES)
            return Err(Error::RetryTimeout);
    } while ((status & 0xf0) != 0);
    
    return Ok();
}