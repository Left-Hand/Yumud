#include "PAW3395.hpp"


// https://github.com/ttwards/motor/blob/939f1db78dcaae6eb819dcb54b6146d94db7dffc/drivers/sensor/paw3395/paw3395.h#L122

using namespace ymd;
using namespace ymd::drivers;

using Error = PAW3395::Error;
template<typename T = void>
using IResult = Result<T, Error>;

IResult<> PAW3395::corded_gaming(){
    return write_list(std::span(INIT_GAME_TABLE));
}

IResult<> PAW3395::validate(){
	TODO();
	return Ok();
}

IResult<> PAW3395::set_dpi(uint16_t DPI_Num){

	// 设置分辨率模式：X轴和Y轴分辨率均由RESOLUTION_X_LOW和RESOLUTION_X_HIGH定义
	if(const auto res = write_reg(PAW3395_REG_MOTION, 0x00);
		res.is_err()) return res;

	// 两个8位寄存器设置X轴分辨率
	const uint8_t temp_low = uint8_t(((DPI_Num / 50) << 8) >> 8);
	if(const auto res = write_reg(PAW3395_REG_RESOLUTION_X_LOW, temp_low);
		res.is_err()) return res;
	if(const auto res = write_reg(PAW3395_REG_RESOLUTION_Y_LOW, temp_low);
		res.is_err()) return res;
	const uint8_t temp_high = (uint8_t)((DPI_Num / 50) >> 8);
	if(const auto res = write_reg(PAW3395_REG_RESOLUTION_X_HIGH, temp_high);
		res.is_err()) return res;
	if(const auto res = write_reg(PAW3395_REG_RESOLUTION_Y_HIGH, temp_high);
		res.is_err()) return res;

	// 更新分辨率
	if(const auto res = write_reg(PAW3395_REG_SET_RESOLUTION, 0x01);
		res.is_err()) return res;

	return Ok();
}

IResult<> PAW3395::set_lift_off(bool height){
	// 1. 将值0x0C写入寄存器0x7F
	if(const auto res = write_reg(0x7F, 0x0C);
		res.is_err()) return res;
	// 2. 将值0x01写入寄存器0x4E
	if(const auto res = write_reg(0x4E, height ? 0x01 : 0x00); // 0x4E 未定义
		res.is_err()) return res;
	// 3. 将值0x00写入寄存器0x7F
	if(const auto res = write_reg(0x7F, 0x00);
		res.is_err()) return res;

	return Ok();
}

IResult<Vec2i> PAW3395::sample_fetch(){

	const bool any_motion = ({
		uint8_t temp = 0;
		const auto res = read_reg(PAW3395_REG_MOTION, temp);
		if (res.is_err()) return Err(res.unwrap_err());
		temp == 0;
	});
	
	if(any_motion == false) return
		Ok(Vec2i(0,0));

	const int16_t x = ({
		const auto res = read_i16(PAW3395_REG_DELTA_X_L, PAW3395_REG_DELTA_X_H);
		if(res.is_err()) return Err(res.unwrap_err());
		res.unwrap();
	});

	const int16_t y = ({
		const auto res = read_i16(PAW3395_REG_DELTA_Y_L, PAW3395_REG_DELTA_Y_H);
		if(res.is_err()) return Err(res.unwrap_err());
		res.unwrap();
	});

	return Ok(Vec2i{x,y});
}

// IResult<> paw3395_channel_get(
// 	enum sensor_channel chan,
// 	struct sensor_value *val)
// {
// 	struct paw3395_data *data = dev->data;
// 	const struct paw3395_config *config = dev->config;

// 	if (chan == SENSOR_CHAN_POS_DX) {
// 		int64_t micro = (float)data->sum_x_last * (25400000.0f / (float)config->dpi);

// 		val->val1 = (int32_t)(micro / 1000000LL);
// 		val->val2 = (int32_t)(micro % 1000000LL);

// 		data->sum_x_last = 0;
// 		return 0;
// 	} else if (chan == SENSOR_CHAN_POS_DY) {
// 		int64_t micro = (float)data->sum_y_last * (25400000.0f / (float)config->dpi);

// 		val->val1 = (int32_t)(micro / 1000000LL);
// 		val->val2 = (int32_t)(micro % 1000000LL);

// 		data->sum_y_last = 0;
// 		return 0;
// 	}

// 	return -ENOTSUP;
// }

IResult<> PAW3395::enable_ripple(const Enable en){

	uint8_t temp ;
	if(const auto res = read_reg(PAW3395_REG_RIPPLE_CONTROL, temp);
		res.is_err()) return res;
	
	const auto temp2 = en == EN ? 
		(temp | 0x80) : temp & (~0x80);
	if(const auto res = write_reg(PAW3395_REG_RIPPLE_CONTROL, temp2);
		res.is_err()) return res;

	return Ok();
}

IResult<> PAW3395::powerup(){
	/* Write register and data */
	if(const auto res = write_reg(PAW3395_REG_POWER_UP_RESET, 0x5A);
		res.is_err()) return res;

	clock::delay(5ms);

	if(const auto res = write_list(std::span(INIT_TABLE_1));
		res.is_err()) return res;

	clock::delay(1ms);

	bool is_susccessfully_inited = false;

	static constexpr size_t MAX_RETRY_TIMES = 60;

	for (size_t retry = 0; retry < MAX_RETRY_TIMES; retry++) {
		uint8_t temp;
		const auto res = read_reg(0x6C, temp);
		if(res.is_err()){
			return Err(res.unwrap_err());
		}else{
			is_susccessfully_inited = (temp == 0x80);
			break;
		}
		
		clock::delay(1ms);
	}

	if(const auto res = write_reg(0x7F, 0x14);
		res.is_err()) return res;
	if(const auto res = write_reg(0x6C, 0x00);
		res.is_err()) return res;
	if(const auto res = write_reg(0x7F, 0x00);
		res.is_err()) return res;

	if(!is_susccessfully_inited) return Err(Error::InitError);

	// 138. 将值0x70写入寄存器0x22
	if(const auto res = write_reg(0x22, 0x70);
		res.is_err()) return res; // 0x22 未定义
	// 139. 将值0x01写入寄存器0x22
	if(const auto res = write_reg(0x22, 0x01);
		res.is_err()) return res; // 0x22 未定义
	// 140. 将值0x00写入寄存器0x22
	if(const auto res = write_reg(0x22, 0x00);
		res.is_err()) return res; // 0x22 未定义
	// 141. 将值0x00写入寄存器0x55
	if(const auto res = write_reg(0x55, 0x00);
		res.is_err()) return res; // 0x55 未定义
	// 142. 将值0x07写入寄存器0x7F
	if(const auto res = write_reg(0x7F, 0x07);
		res.is_err()) return res;
	// 143. 将值0x40写入寄存器0x40
	if(const auto res = write_reg(0x40, 0x40);
		res.is_err()) return res; // 0x40 未定义
	// 144. 将值0x00写入寄存器0x7F
	if(const auto res = write_reg(0x7F, 0x00);
		res.is_err()) return res;
	return Ok();
}

IResult<> PAW3395::init() {
	// TODO();
	if(const auto res = validate();
		res.is_err()) return res;

	if(const auto res = powerup();
		res.is_err()) return res;
	return Ok();
}


IResult<> PAW3395::write_reg(const uint8_t addr, const uint8_t data){
	const std::array<uint8_t, 2> temp = {
		uint8_t(addr| 0x80),
		data
	};

	if(const auto res = spi_drv_.write_burst<uint8_t>(std::span(temp));
		res.is_err()) return Err(res.unwrap_err());
	return Ok();
}

IResult<> PAW3395::read_reg(const uint8_t addr, uint8_t & data){
	const std::array<uint8_t, 2> pbuf_tx = {
		uint8_t(addr| 0x80),
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

IResult<int16_t> PAW3395::read_i16(const uint8_t addr1, const uint8_t addr2){
	uint8_t low_byte;
	uint8_t high_byte;


	if(const auto res = read_reg(addr1, low_byte);
		res.is_err()) return Err(res.unwrap_err());

	if(const auto res = read_reg(addr2, high_byte);
		res.is_err()) return Err(res.unwrap_err());

	return Ok(int16_t((high_byte << 8) | low_byte));
}

IResult<> PAW3395::write_list(std::span<const std::pair<uint8_t, uint8_t>> list){
	for(const auto & [addr, data] : list){
		if(const auto res = write_reg(addr, data); 
			res.is_err()) return res;
	}
	return Ok();
}