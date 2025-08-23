//这个驱动还未完成
//这个驱动还未测试

//VL53L5X 是意法半导体的一款具有宽视野的飞行时间 (ToF) 8x8多区测距传感器

// VL53L5CX是意法半导体FlightSense产品系列中最先进的飞行时间 (ToF) 多区测距传感器。
// 该产品采用微型可回流焊封装，集成了SPAD阵列、物理红外滤光片和衍射光学元件 (DOE)，
// 确保在各种环境照明条件下，搭配不同类型的盖片材料，均能够实现卓越的测距性能。

// 通过在垂直腔面发射激光器 (VCSEL) 上方使用DOE，将一个方形FoV投影到场景上。反射光随后由接收器透镜聚焦到SPAD阵列上。

// 不同于传统的IR传感器，VL53L5CX利用意法半导体最新一代ToF技术，能够测量目标的绝对距离，
// 而不受目标颜色或反射率的影响。该产品可提供高达400 cm的精确测距，并能以极快的速度 
// (60 Hz) 运行，是目前市场上速度最快的多区微型ToF传感器之一。

// 多区测距最多可覆盖8x8个区域，对角线视野可达65°（可通过软件调整）

// 得益于意法半导体的专利型直方图算法，VL53L5CX能够检测并区分FoV内的多个目标。
// 该算法还能有效抵御60 cm以上的盖片串扰。

#pragma once

#include "details/vl53l5cx_prelude.hpp"
#include "details/vl53l5cx_buffers.hpp"

namespace ymd::drivers{
using namespace vl53l5cx_details;

class VL53L5CX final{ 
public:
    enum class ErrorKind:uint8_t { 
        TimeOut = 1,
        McuError = 66,
        InvalidParam = 127,
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, ErrorKind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    IResult<> init();

    static constexpr ErrorKind map_status_to_error(uint8_t status){
        return std::bit_cast<ErrorKind>(status);
    }
private:
	/* Results streamcount, value auto-incremented at each range */
	uint8_t		        streamcount;
	/* Size of data read though I2C */
	uint32_t	        data_read_size;
	/* Address of default configuration buffer */
	const uint8_t		        *default_configuration = VL53L5CX_DEFAULT_CONFIGURATION;
	/* Address of default Xtalk buffer */
	const uint8_t		        *default_xtalk = VL53L5CX_DEFAULT_XTALK;
	/* Offset buffer */
	uint8_t		        offset_data[VL53L5CX_OFFSET_BUFFER_SIZE];
	/* Xtalk buffer */
	uint8_t		        xtalk_data[VL53L5CX_XTALK_BUFFER_SIZE];
	/* Temporary buffer used for internal driver processing */
    uint8_t	        temp_buffer[VL53L5CX_TEMPORARY_BUFFER_SIZE];

    IResult<> read_byte(const uint16_t addr, uint8_t *data){
        return Ok();
    }

    IResult<> write_byte(const uint16_t addr, uint8_t data){
        return Ok();
    }

    IResult<> read_burst(const uint16_t addr, uint8_t *data, uint16_t size){
        return Ok();
    }

    IResult<> write_burst(const uint16_t addr, const uint8_t *data, uint16_t size){
        return Ok();
    }

    IResult<> poll_for_answer(
		uint8_t					size,
		uint8_t					pos,
		uint16_t				address,
		uint8_t					mask,
		uint8_t					expected_value);

    IResult<> poll_for_mcu_boot();
};
}