#include "paw3395.hpp"




using namespace ymd;
using namespace ymd::drivers::paw3395;

using Self = PAW3395;

using Error = Self::Error;
template<typename T = void>
using IResult = Result<T, Error>;

// https://github.com/ttwards/motor/blob/939f1db78dcaae6eb819dcb54b6146d94db7dffc/drivers/sensor/paw3395/paw3395.h#L122
// https://github.com/dotdotchan/bs2x_sdk/blob/main/application/samples/products/sle_mouse_with_dongle/mouse_sensor/mouse_sensor_paw3395.c

#define BURST_MOTION_READ               0x16
#define READ_LENGTH                     6
#define POWER_UP_DELAY_MS               40

#define WRITE_RESET_ADDR                0xba     // Write operation, MSB is 1.
#define RESET_VALUE                     0x5a
#define READ_ID_ADDR                    0x00
#define LEN_1                           1
#define LEN_2                           2
#define RAW_DATA_GRAB_ADDR              0x58
#define RAW_DATA_GRAB_STATUS_ADDR       0x59
#define RAW_DATA_LEN                    1225
#define PIN_MOTION                      S_MGPIO21
#define USB_8K_MOUSE_REPORT_DELAY       125
#define MOUSE_TO_BT_DATA_LEN            5
#define MOUSE_3395_READ_TIMES           60
#define MOUSE_3395_READ_REG             0x6c
#define MOUSE_3395_READ_TARGET_VAL      0x80
#define MOUSE_3395_READ_DELAY           1000

#define BT_MOUSE_REPORT_PARAM_NUM       4
#define SPI_NUM_5                       5
#define XY_DATA_SHIFT_LEN               8
#define X_LOW_8BIT                      2
#define X_HIGH_8BIT                     3
#define Y_LOW_8BIT                      4
#define Y_HIGH_8BIT                     5
#define SPI_MHZ                         4



IResult<> Self::corded_gaming(){
    return write_list(std::span(INIT_GAME_TABLE));
}

IResult<> Self::validate(){
	uint8_t reg_val;
	if(const auto res = read_reg(RegAddr::PRODUCT_ID, reg_val);
		res.is_err()) return res;
	if(reg_val != PAW3395_PRODUCT_ID)
		return Err(Error::InvalidProductId);
	return Ok();
}

IResult<> Self::set_dpi(uint16_t dpi_num){

	// 设置分辨率模式：X轴和Y轴分辨率均由RESOLUTION_X_LOW和RESOLUTION_X_HIGH定义
	if(const auto res = write_reg(RegAddr::MOTION, 0x00);
		res.is_err()) return res;

	// 两个8位寄存器设置X轴分辨率
	const auto [temp_low, temp_high] = std::bit_cast<std::array<uint8_t, 2>>(
		static_cast<uint16_t>((dpi_num / 50)));

	//分别写入x,y寄存器
	if(const auto res = write_reg(RegAddr::RESOLUTION_X_LOW, temp_low);
		res.is_err()) return res;
	if(const auto res = write_reg(RegAddr::RESOLUTION_X_HIGH, temp_high);
		res.is_err()) return res;

	if(const auto res = write_reg(RegAddr::RESOLUTION_Y_LOW, temp_low);
		res.is_err()) return res;
	if(const auto res = write_reg(RegAddr::RESOLUTION_Y_HIGH, temp_high);
		res.is_err()) return res;

	// 更新分辨率
	if(const auto res = write_reg(RegAddr::SET_RESOLUTION, 0x01);
		res.is_err()) return res;

	return Ok();
}

IResult<> Self::set_lift_off(bool height){
	// 1. 将值0x0C写入寄存器0x7F
	if(const auto res = write_reg(static_cast<RegAddr>(0x7F), 0x0C);
		res.is_err()) return res;
	// 2. 将值0x01写入寄存器0x4E
	if(const auto res = write_reg(static_cast<RegAddr>(0x4E), height ? 0x01 : 0x00); // 0x4E 未定义
		res.is_err()) return res;
	// 3. 将值0x00写入寄存器0x7F
	if(const auto res = write_reg(static_cast<RegAddr>(0x7F), 0x00);
		res.is_err()) return res;

	return Ok();
}

IResult<bool> Self::is_motioned(){
	uint8_t temp = 0;
	if(const auto res = read_reg(RegAddr::MOTION, temp);
		res.is_err()) return Err(res.unwrap_err());
	return Ok(temp != 0);
}

IResult<math::Vec2i> Self::query_xy(){
	const int16_t x = ({
		const auto res = read_i16(RegAddr::DELTA_X_L, RegAddr::DELTA_X_H);
		if(res.is_err()) return Err(res.unwrap_err());
		res.unwrap();
	});

	const int16_t y = ({
		const auto res = read_i16(RegAddr::DELTA_Y_L, RegAddr::DELTA_Y_H);
		if(res.is_err()) return Err(res.unwrap_err());
		res.unwrap();
	});

	return Ok(math::Vec2i{x, y});
}

IResult<math::Vec2i> Self::update(){
	const bool any_motion = ({
		const auto res = is_motioned();
		if(res.is_err()) return Err(res.unwrap_err());
		res.unwrap();
	});
	
	if(any_motion == false)
		return Err(Error::DataNotReady);
	return query_xy();
};

IResult<> Self::enable_ripple(const Enable en){

	uint8_t temp ;
	if(const auto res = read_reg(RegAddr::RIPPLE_CONTROL, temp);
		res.is_err()) return res;
	
	const auto temp2 = en == EN ? 
		(temp | 0x80) : temp & (~0x80);
	if(const auto res = write_reg(RegAddr::RIPPLE_CONTROL, temp2);
		res.is_err()) return res;

	return Ok();
}

IResult<> Self::powerup(){
	/* Write register and data */
	if(const auto res = write_reg(RegAddr::POWER_UP_RESET, 0x5A);
		res.is_err()) return res;

	clock::delay(5ms);

	if(const auto res = write_list(std::span(INIT_TABLE_1));
		res.is_err()) return res;

	clock::delay(1ms);

	bool is_susccessfully_inited = false;

	static constexpr size_t MAX_RETRY_TIMES = 60;

	for (size_t retry = 0; retry < MAX_RETRY_TIMES; retry++) {
		uint8_t temp;
		const auto res = read_reg(static_cast<RegAddr>(0x6C), temp);
		if(res.is_err())
			return Err(res.unwrap_err());

		is_susccessfully_inited = (temp == 0x80);

		if(is_susccessfully_inited){
			break;
		}
		
		clock::delay(1ms);
	}

	if(!is_susccessfully_inited){
		if(const auto res = write_reg(static_cast<RegAddr>(0x7F), 0x14);
			res.is_err()) return res;
		if(const auto res = write_reg(static_cast<RegAddr>(0x6C), 0x00);
			res.is_err()) return res;
		if(const auto res = write_reg(static_cast<RegAddr>(0x7F), 0x00);
			res.is_err()) return res;
	}

	// 138. 将值0x70写入寄存器0x22
	if(const auto res = write_reg(static_cast<RegAddr>(0x22), 0x70);
		res.is_err()) return res; // 0x22 未定义
	// 139. 将值0x01写入寄存器0x22
	if(const auto res = write_reg(static_cast<RegAddr>(0x22), 0x01);
		res.is_err()) return res; // 0x22 未定义
	// 140. 将值0x00写入寄存器0x22
	if(const auto res = write_reg(static_cast<RegAddr>(0x22), 0x00);
		res.is_err()) return res; // 0x22 未定义
	// 141. 将值0x00写入寄存器0x55
	if(const auto res = write_reg(static_cast<RegAddr>(0x55), 0x00);
		res.is_err()) return res; // 0x55 未定义
	// 142. 将值0x07写入寄存器0x7F
	if(const auto res = write_reg(static_cast<RegAddr>(0x7F), 0x07);
		res.is_err()) return res;
	// 143. 将值0x40写入寄存器0x40
	if(const auto res = write_reg(static_cast<RegAddr>(0x40), 0x40);
		res.is_err()) return res; // 0x40 未定义
	// 144. 将值0x00写入寄存器0x7F
	if(const auto res = write_reg(static_cast<RegAddr>(0x7F), 0x00);
		res.is_err()) return res;
	return Ok();
}

IResult<> Self::init(const Config & cfg) {

	if(const auto res = validate();
		res.is_err()) return res;

	if(const auto res = powerup();
		res.is_err()) return res;

	if(const auto res = corded_gaming();
		res.is_err()) return res;

	if(const auto res = set_dpi(cfg.dpi_num);
		res.is_err()) return res;

	if(const auto res = set_lift_off(true);
		res.is_err()) return res;

	if(const auto res = enable_ripple(EN);
		res.is_err()) return res;

	return Ok();
}


IResult<> Self::write_reg(const RegAddr addr, const uint8_t data){
	const std::array<uint8_t, 2> temp = {
		uint8_t(std::bit_cast<uint8_t>(addr) | 0x80),
		data
	};

	if(const auto res = spi_drv_.write_burst<uint8_t>(std::span(temp));
		res.is_err()) return Err(res.unwrap_err());
	return Ok();
}

IResult<> Self::read_reg(const RegAddr addr, uint8_t & data){
	const std::array<uint8_t, 2> pbuf_tx = {
		uint8_t(std::bit_cast<uint8_t>(addr) | 0x80),
		0x00
	};

	std::array<uint8_t, 2> pbuf_rx;

	if(const auto res = spi_drv_.transceive_burst<uint8_t, 2>(
		std::span(pbuf_rx), 
		std::span(pbuf_tx)
	); res.is_err()) return Err(res.unwrap_err());
	data = pbuf_rx[1];

	return Ok();
}

IResult<int16_t> Self::read_i16(const RegAddr addr1, const RegAddr addr2){
	uint8_t low_byte;
	uint8_t high_byte;


	if(const auto res = read_reg(addr1, low_byte);
		res.is_err()) return Err(res.unwrap_err());

	if(const auto res = read_reg(addr2, high_byte);
		res.is_err()) return Err(res.unwrap_err());

	return Ok(int16_t((high_byte << 8) | low_byte));
}

IResult<> Self::write_list(std::span<const std::pair<uint8_t, uint8_t>> list){
	for(const auto & [addr, data] : list){
		if(const auto res = write_reg(std::bit_cast<RegAddr>(addr), data); 
			res.is_err()) return res;
	}
	return Ok();
}