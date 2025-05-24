#include "ICM20948.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = ICM20948::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> ICM20948::init()
{
	if(const auto res = validate();
		res.is_err()) return res;

	if(const auto res = device_reset();
		res.is_err()) return res;
	if(const auto res = wakeup();
		res.is_err()) return res;

	if(const auto res = clock_source(1);
		res.is_err()) return res;
	if(const auto res = odr_align_enable();
		res.is_err()) return res;
	
	if(const auto res = spi_slave_enable();
		res.is_err()) return res;
	
	if(const auto res = gyro_low_pass_filter(0);
		res.is_err()) return res;
	if(const auto res = accel_low_pass_filter(0);
		res.is_err()) return res;

	if(const auto res = gyro_sample_rate_divider(0);
		res.is_err()) return res;
	if(const auto res = accel_sample_rate_divider(0);
		res.is_err()) return res;

	// if(const auto res = gyro_calibration();
	// 		res.is_err()) return res;
	// if(const auto res = accel_calibration();
	// 		res.is_err()) return res;

	if(const auto res = gyro_full_scale_select(_2000dps);
		res.is_err()) return res;
	if(const auto res = accel_full_scale_select(_16g);
		res.is_err()) return res;

	return Ok();
}

void ak09916_init()
{
	// i2c_master_reset();
	// i2c_master_enable();
	// i2c_master_clk_frq(7);

	// while(!ak09916_who_am_i());

	// ak09916_soft_reset();
	// ak09916_operation_mode_setting(continuous_measurement_100hz);
	TODO();
}

// IResult<> ICM20948::gyro_read(axises* data)
// {
// 	uint8_t* temp = read_multiple_reg(ub_0, B0_GYRO_XOUT_H, 6);

// 	data->x = (int16_t)(temp[0] << 8 | temp[1]);
// 	data->y = (int16_t)(temp[2] << 8 | temp[3]);
// 	data->z = (int16_t)(temp[4] << 8 | temp[5]);
// }

// IResult<> ICM20948::accel_read(axises* data)
// {
// 	uint8_t* temp = read_multiple_reg(ub_0, B0_ACCEL_XOUT_H, 6);

// 	data->x = (int16_t)(temp[0] << 8 | temp[1]);
// 	data->y = (int16_t)(temp[2] << 8 | temp[3]);
// 	data->z = (int16_t)(temp[4] << 8 | temp[5]) + accel_scale_factor; 
// 	// Add scale factor because calibraiton function offset gravity acceleration.
// }

// bool ICM20948::ak09916_mag_read(axises* data)
// {
// 	uint8_t* temp;
// 	uint8_t drdy, hofl;	// data ready, overflow

// 	drdy = read_single_ak09916_reg(MAG_ST1) & 0x01;
// 	if(!drdy)	return false;

// 	temp = read_multiple_ak09916_reg(MAG_HXL, 6);

// 	hofl = read_single_ak09916_reg(MAG_ST2) & 0x08;
// 	if(hofl)	return false;

// 	data->x = (int16_t)(temp[1] << 8 | temp[0]);
// 	data->y = (int16_t)(temp[3] << 8 | temp[2]);
// 	data->z = (int16_t)(temp[5] << 8 | temp[4]);

// 	return true;
// }

// IResult<> ICM20948::gyro_read_dps(axises* data)
// {
// 	gyro_read(data);

// 	data->x = data->x / gyro_scale_factor;
// 	data->y = data->y / gyro_scale_factor;
// 	data->z = data->z / gyro_scale_factor;
// }

// IResult<> ICM20948::accel_read_g(axises* data)
// {
// 	accel_read(data);

// 	data->x = data->x / accel_scale_factor;
// 	data->y = data->y / accel_scale_factor;
// 	data->z = data->z / accel_scale_factor;
// }

// bool ICM20948::ak09916_mag_read_uT(axises* data)
// {
// 	axises temp;
// 	bool new_data = ak09916_mag_read(&temp);
// 	if(!new_data)	return false;

// 	data->x = int32_t(temp.x * real_t(0.15));
// 	data->y = int32_t(temp.y * real_t(0.15));
// 	data->z = int32_t(temp.z * real_t(0.15));

// 	return true;
// }	


/* Sub Functions */
IResult<> ICM20948::validate()
{
	const auto id = read_single_reg(ub_0, B0_WHO_AM_I);

	if(id != ICM20948_ID) return Err(Error::WrongWhoAmI);

	return Ok();
}

bool ICM20948::ak09916_who_am_i()
{
	uint8_t ak09916_id = read_single_ak09916_reg(MAG_WIA2);

	if(ak09916_id == AK09916_ID)
		return true;
	else
		return false;
}

IResult<> ICM20948::device_reset()
{
	return write_single_reg(ub_0, B0_PWR_MGMT_1, 0x80 | 0x41);
}

void ICM20948::ak09916_soft_reset()
{
	write_single_ak09916_reg(MAG_CNTL3, 0x01).unwrap();
	clock::delay(100ms);
}

IResult<> ICM20948::wakeup()
{
	uint8_t new_val = read_single_reg(ub_0, B0_PWR_MGMT_1);
	new_val &= 0xBF;

	return write_single_reg(ub_0, B0_PWR_MGMT_1, new_val);
}

IResult<> ICM20948::sleep()
{
	uint8_t new_val = read_single_reg(ub_0, B0_PWR_MGMT_1);
	new_val |= 0x40;

	return write_single_reg(ub_0, B0_PWR_MGMT_1, new_val);
}

IResult<> ICM20948::spi_slave_enable(){
	uint8_t new_val = read_single_reg(ub_0, B0_USER_CTRL);
	new_val |= 0x10;

	return write_single_reg(ub_0, B0_USER_CTRL, new_val);
}

IResult<> ICM20948::i2c_master_reset(){
	uint8_t new_val = read_single_reg(ub_0, B0_USER_CTRL);
	new_val |= 0x02;

	return write_single_reg(ub_0, B0_USER_CTRL, new_val);
}

IResult<> ICM20948::i2c_master_enable()
{
	uint8_t new_val = read_single_reg(ub_0, B0_USER_CTRL);
	new_val |= 0x20;

	return write_single_reg(ub_0, B0_USER_CTRL, new_val);
}

IResult<> ICM20948::i2c_master_clk_frq(uint8_t config)
{
	uint8_t new_val = read_single_reg(ub_3, B3_I2C_MST_CTRL);
	new_val |= config;

	return write_single_reg(ub_3, B3_I2C_MST_CTRL, new_val);	
}

IResult<> ICM20948::clock_source(uint8_t source)
{
	uint8_t new_val = read_single_reg(ub_0, B0_PWR_MGMT_1);
	new_val |= source;

	return write_single_reg(ub_0, B0_PWR_MGMT_1, new_val);
}

IResult<> ICM20948::odr_align_enable()
{
	return write_single_reg(ub_2, B2_ODR_ALIGN_EN, 0x01);
}

IResult<> ICM20948::gyro_low_pass_filter(uint8_t config)
{
	uint8_t new_val = read_single_reg(ub_2, B2_GYRO_CONFIG_1);
	new_val |= config << 3;

	return write_single_reg(ub_2, B2_GYRO_CONFIG_1, new_val);
}

IResult<> ICM20948::accel_low_pass_filter(uint8_t config)
{
	uint8_t new_val = read_single_reg(ub_2, B2_ACCEL_CONFIG);
	new_val |= config << 3;

	return write_single_reg(ub_2, B2_GYRO_CONFIG_1, new_val);
}

IResult<> ICM20948::gyro_sample_rate_divider(uint8_t divider)
{
	return write_single_reg(ub_2, B2_GYRO_SMPLRT_DIV, divider);
}

IResult<> ICM20948::accel_sample_rate_divider(uint16_t divider)
{
	uint8_t divider_1 = (uint8_t)(divider >> 8);
	uint8_t divider_2 = (uint8_t)(0x0F & divider);

	if(const auto res = write_single_reg(ub_2, B2_ACCEL_SMPLRT_DIV_1, divider_1);
		res.is_err()) return res;
	return write_single_reg(ub_2, B2_ACCEL_SMPLRT_DIV_2, divider_2);
}

IResult<> ICM20948::ak09916_operation_mode_setting(operation_mode mode)
{
	return write_single_ak09916_reg(MAG_CNTL2, mode);
}

static constexpr size_t CALI_TIMES = 64;
static constexpr real_t INV_CALI_TIMES = real_t(1.0 / CALI_TIMES);

// IResult<> ICM20948::gyro_calibration(){
// 	axises temp;
// 	int32_t gyro_bias[3] = {0};
// 	uint8_t gyro_offset[6] = {0};

// 	for(int i = 0; i < CALI_TIMES; i++)
// 	{
// 		gyro_read(&temp);
// 		gyro_bias[0] += temp.x;
// 		gyro_bias[1] += temp.y;
// 		gyro_bias[2] += temp.z;
// 	}

// 	gyro_bias[0] = int32_t(gyro_bias[0] * INV_CALI_TIMES);
// 	gyro_bias[1] = int32_t(gyro_bias[1] * INV_CALI_TIMES);
// 	gyro_bias[2] = int32_t(gyro_bias[2] * INV_CALI_TIMES);

// 	// Construct the gyro biases for push to the hardware gyro bias registers,
// 	// which are reset to zero upon device startup.
// 	// Divide by 4 to get 32.9 LSB per deg/s to conform to expected bias input format.
// 	// Biases are additive, so change sign on calculated average gyro biases
// 	gyro_offset[0] = (-gyro_bias[0] / 4  >> 8) & 0xFF; 
// 	gyro_offset[1] = (-gyro_bias[0] / 4)       & 0xFF; 
// 	gyro_offset[2] = (-gyro_bias[1] / 4  >> 8) & 0xFF;
// 	gyro_offset[3] = (-gyro_bias[1] / 4)       & 0xFF;
// 	gyro_offset[4] = (-gyro_bias[2] / 4  >> 8) & 0xFF;
// 	gyro_offset[5] = (-gyro_bias[2] / 4)       & 0xFF;
	
// 	write_multiple_reg(ub_2, B2_XG_OFFS_USRH, gyro_offset, 6);
// }

// IResult<> ICM20948::accel_calibration(){
// 	axises temp;
// 	uint8_t* temp2;
// 	uint8_t* temp3;
// 	uint8_t* temp4;
	
// 	int32_t accel_bias[3] = {0};
// 	int32_t accel_bias_reg[3] = {0};
// 	uint8_t accel_offset[6] = {0};

// 	for(int i = 0; i < CALI_TIMES; i++)
// 	{
// 		accel_read(&temp);
// 		accel_bias[0] += temp.x;
// 		accel_bias[1] += temp.y;
// 		accel_bias[2] += temp.z;
// 	}

// 	accel_bias[0] = int32_t(accel_bias[0] * INV_CALI_TIMES);
// 	accel_bias[1] = int32_t(accel_bias[1] * INV_CALI_TIMES);
// 	accel_bias[2] = int32_t(accel_bias[2] * INV_CALI_TIMES);

// 	uint8_t mask_bit[3] = {0, 0, 0};

// 	temp2 = read_multiple_reg(ub_1, B1_XA_OFFS_H, 2);
// 	accel_bias_reg[0] = (int32_t)(temp2[0] << 8 | temp2[1]);
// 	mask_bit[0] = temp2[1] & 0x01;

// 	temp3 = read_multiple_reg(ub_1, B1_YA_OFFS_H, 2);
// 	accel_bias_reg[1] = (int32_t)(temp3[0] << 8 | temp3[1]);
// 	mask_bit[1] = temp3[1] & 0x01;

// 	temp4 = read_multiple_reg(ub_1, B1_ZA_OFFS_H, 2);
// 	accel_bias_reg[2] = (int32_t)(temp4[0] << 8 | temp4[1]);
// 	mask_bit[2] = temp4[1] & 0x01;

// 	accel_bias_reg[0] -= (accel_bias[0] / 8);
// 	accel_bias_reg[1] -= (accel_bias[1] / 8);
// 	accel_bias_reg[2] -= (accel_bias[2] / 8);

//     accel_offset[0] = (accel_bias_reg[0] >> 8) & 0xFF;
//     accel_offset[1] = (accel_bias_reg[0])      & 0xFE;
//     accel_offset[1] = accel_offset[1] | mask_bit[0];

//     accel_offset[2] = (accel_bias_reg[1] >> 8) & 0xFF;
//     accel_offset[3] = (accel_bias_reg[1])      & 0xFE;
//     accel_offset[3] = accel_offset[3] | mask_bit[1];

// 	accel_offset[4] = (accel_bias_reg[2] >> 8) & 0xFF;
// 	accel_offset[5] = (accel_bias_reg[2])      & 0xFE;
// 	accel_offset[5] = accel_offset[5] | mask_bit[2];
	
// 	write_multiple_reg(ub_1, B1_XA_OFFS_H, &accel_offset[0], 2);
// 	write_multiple_reg(ub_1, B1_YA_OFFS_H, &accel_offset[2], 2);
// 	write_multiple_reg(ub_1, B1_ZA_OFFS_H, &accel_offset[4], 2);
// }

IResult<> ICM20948::gyro_full_scale_select(gyro_full_scale full_scale)
{
	uint8_t new_val = read_single_reg(ub_2, B2_GYRO_CONFIG_1);
	
	switch(full_scale)
	{
		case _250dps :
			new_val |= 0x00;
			gyro_scale_factor = 131.0_r;
			break;
		case _500dps :
			new_val |= 0x02;
			gyro_scale_factor = 65.5_r;
			break;
		case _1000dps :
			new_val |= 0x04;
			gyro_scale_factor = 32.8_r;
			break;
		case _2000dps :
			new_val |= 0x06;
			gyro_scale_factor = 16.4_r;
			break;
	}

	return write_single_reg(ub_2, B2_GYRO_CONFIG_1, new_val);
}

IResult<> ICM20948::accel_full_scale_select(accel_full_scale full_scale)
{
	uint8_t new_val = read_single_reg(ub_2, B2_ACCEL_CONFIG);
	
	switch(full_scale)
	{
		case _2g :
			new_val |= 0x00;
			accel_scale_factor = 16384;
			break;
		case _4g :
			new_val |= 0x02;
			accel_scale_factor = 8192;
			break;
		case _8g :
			new_val |= 0x04;
			accel_scale_factor = 4096;
			break;
		case _16g :
			new_val |= 0x06;
			accel_scale_factor = 2048;
			break;
	}

	return write_single_reg(ub_2, B2_ACCEL_CONFIG, new_val);
}


IResult<> ICM20948::select_user_bank(userbank ub){
    // uint8_t write_reg[2];
    // write_reg[0] = WRITE_CMD | REG_BANK_SEL;
    // write_reg[1] = ub;

    // cs_low();
    // HAL_SPI_Transmit(SPI, write_reg, 2, 10);
    // cs_high();
    TODO();
}

uint8_t ICM20948::read_single_reg(userbank ub, uint8_t reg)
{
    // uint8_t read_reg = READ_CMD | reg;
    // uint8_t reg_val;
    // select_user_bank(ub);

    // cs_low();
    // HAL_SPI_Transmit(SPI, &read_reg, 1, 1000);
    // HAL_SPI_Receive(SPI, &reg_val, 1, 1000);
    // cs_high();

    // return reg_val;
    TODO();
	return 0;
}

IResult<> ICM20948::write_single_reg(userbank ub, uint8_t reg, uint8_t val)
{
    // uint8_t write_reg[2];
    // write_reg[0] = WRITE_CMD | reg;
    // write_reg[1] = val;

    // select_user_bank(ub);

    // cs_low();
    // HAL_SPI_Transmit(SPI, write_reg, 2, 1000);
    // cs_high();

    TODO();
	return Ok();
}

uint8_t* ICM20948::read_multiple_reg(userbank ub, uint8_t reg, uint8_t len)
{
    // uint8_t read_reg = READ_CMD | reg;
    // uint8_t reg_val[6];
    // select_user_bank(ub);

    // cs_low();
    // HAL_SPI_Transmit(SPI, &read_reg, 1, 1000);
    // HAL_SPI_Receive(SPI, reg_val, len, 1000);
    // cs_high();

    // return reg_val;

    TODO();
    return nullptr;
}

IResult<> ICM20948::write_multiple_reg(userbank ub, uint8_t reg, uint8_t* val, uint8_t len)
{
    // uint8_t write_reg = WRITE_CMD | reg;
    // select_user_bank(ub);

    // cs_low();
    // HAL_SPI_Transmit(SPI, &write_reg, 1, 1000);
    // HAL_SPI_Transmit(SPI, val, len, 1000);
    // cs_high();

    TODO();
}


uint8_t ICM20948::read_single_ak09916_reg(uint8_t reg)
{
    // write_single_reg(ub_3, B3_I2C_SLV0_ADDR, READ_CMD | MAG_SLAVE_ADDR);
    // write_single_reg(ub_3, B3_I2C_SLV0_REG, reg);
    // write_single_reg(ub_3, B3_I2C_SLV0_CTRL, 0x81);

    // clock::delay(1ms);
    // return read_single_reg(ub_0, B0_EXT_SLV_SENS_DATA_00);

    TODO();
    return 0;
}

IResult<> ICM20948::write_single_ak09916_reg(uint8_t reg, uint8_t val)
{
    // write_single_reg(ub_3, B3_I2C_SLV0_ADDR, WRITE_CMD | MAG_SLAVE_ADDR);
    // write_single_reg(ub_3, B3_I2C_SLV0_REG, reg);
    // write_single_reg(ub_3, B3_I2C_SLV0_DO, val);
    // write_single_reg(ub_3, B3_I2C_SLV0_CTRL, 0x81);
    TODO();
}

uint8_t* ICM20948::read_multiple_ak09916_reg(uint8_t reg, uint8_t len)
{	
    // write_single_reg(ub_3, B3_I2C_SLV0_ADDR, READ_CMD | MAG_SLAVE_ADDR);
    // write_single_reg(ub_3, B3_I2C_SLV0_REG, reg);
    // write_single_reg(ub_3, B3_I2C_SLV0_CTRL, 0x80 | len);

    // clock::delay(1ms);
    // return read_multiple_reg(ub_0, B0_EXT_SLV_SENS_DATA_00, len);

    TODO();
    return nullptr;
}