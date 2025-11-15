#pragma once

#include <cstdint>

namespace ymd::drivers{
struct [[nodiscard]] DW1000_TimeStamp {
public:
    using Self = DW1000_TimeStamp;

    static constexpr size_t VALID_BITS = 40;
	// Time resolution in micro-seconds of time based registers/values.
	// Each bit in a count counts for a period of approx. 15.65ps
	static constexpr float TIME_RES     = 0.000015650040064103f;
	static constexpr float TIME_RES_INV = 63897.6f;
	
	// Speed of radio waves [m/s] * count resolution [~15.65ps] of DW1000
	static constexpr float DISTANCE_OF_RADIO     = 0.0046917639786159f;
	static constexpr float DISTANCE_OF_RADIO_INV = 213.139451293f;
	
	// count byte length - 40 bit -> 5 byte
	static constexpr uint8_t LENGTH_TIMESTAMP = 5;
	
	// timer/counter overflow (40 bits) -> 4overflow approx. every 17.2 seconds
	static constexpr int64_t TIME_OVERFLOW = 0x10000000000; //1099511627776LL
	static constexpr int64_t TIME_MAX      = 0xffffffffff;
	
    static constexpr Self from_us(const float timeUs){
        return Self((int64_t)(timeUs*TIME_RES_INV));
    }
	constexpr Self & warp(){
        if(count < 0) {
            count += TIME_OVERFLOW;
        }
        return *this;
    }

    constexpr bool is_valid() const {
        return (0 <= count && count <= TIME_MAX);
    }
    constexpr float to_float() const {
        return (count%TIME_OVERFLOW)*TIME_RES;
    }

    constexpr float to_meters() const {
        return (count%TIME_OVERFLOW)*DISTANCE_OF_RADIO;
    }
public:
	// count size from dw1000 is 40bit, maximum number 1099511627775
	// signed because you can calculate with Self; negative values are possible errors
	int64_t count;
};

}
