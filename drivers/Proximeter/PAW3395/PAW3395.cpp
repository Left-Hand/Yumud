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

// IResult<> paw3395_sample_fetch(const struct device *dev, enum sensor_channel chan)
// {
// 	struct paw3395_data *data = dev->data;
// 	const struct paw3395_config *config = dev->config;

// 	data->motion = SPI_READ_ADDR(&config->bus, PAW3395_REG_MOTION) != 0x00;

// 	if (data->motion) {
// 		/* Combine high and low bytes into 16-bit signed values */
// 		data->dx = -(int16_t)(SPI_READ_ADDR(&config->bus, PAW3395_REG_DELTA_X_L) +
// 				(SPI_READ_ADDR(&config->bus, PAW3395_REG_DELTA_X_H) << 8));
// 		data->dy = (int16_t)(SPI_READ_ADDR(&config->bus, PAW3395_REG_DELTA_Y_L) +
// 				(SPI_READ_ADDR(&config->bus, PAW3395_REG_DELTA_Y_H) << 8));

// 		data->sum_x += data->dx;
// 		data->sum_y += data->dy;

// 		data->sum_x_last += data->dx;
// 		data->sum_y_last += data->dy;
// 	} else {
// 		data->dx = 0;
// 		data->dy = 0;
// 	}
// 	SPI_CS_HIGH(&config->cs_gpio);
// 	return 0;
// }

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

// IResult<> paw3395_enable_ripple(const struct device *dev){
// 	int ret;

// 	uint8_t temp = SPI_READ_ADDR(&config->bus, PAW3395_REG_RIPPLE_CONTROL);
// 	// Set bit 7 to 1 to enable ripple
// 	temp |= 0x80;
// 	SPI_WRITE_ADDR(&config->bus, PAW3395_REG_RIPPLE_CONTROL, temp);

// 	return;
// }

IResult<> PAW3395::powerup(){
	/* Write register and data */
	if(const auto res = write_reg(PAW3395_REG_POWER_UP_RESET, 0x5A);
		res.is_ok()) return res;

	delay(5);

	if(const auto res = write_list(std::span(INIT_TABLE_1));
		res.is_err()) return res;

	delay(1);

	bool is_susccessfully_inited = false;
	static constexpr size_t MAX_RETRY_TIMES = 60;

	for (size_t retry = 0; retry < MAX_RETRY_TIMES; retry++) {
		const auto res = read_reg(0x6C).map([&](auto val) { return (val == 0x80);});
		if(res.is_ok()){
			is_susccessfully_inited = res.unwrap();
			break;
		}else{
			return Err(res.unwrap_err());
		}
		
		delay(1);
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

	for (int reg = 0x02; reg <= 0x06; reg++) {
		if(const auto res = read_reg(reg);
			res.is_err()) return Err(res.unwrap_err());
	}

	return Ok();
}

IResult<> PAW3395::init() {

}