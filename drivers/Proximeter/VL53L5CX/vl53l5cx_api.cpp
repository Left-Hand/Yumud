#include "vl53l5cx.hpp"
#include "details/vl53l5cx_buffers.hpp"

using namespace ymd;
using namespace ymd::drivers;

using namespace ymd::drivers::vl53l5cx_details;

using Error = VL53L5CX::Error;

template<typename T = void>
using IResult = Result<T, Error>;


#if 0

IResult<> VL53L5CX::init()
{
	uint8_t tmp, status = VL53L5CX_STATUS_OK;
	static constexpr uint8_t pipe_ctrl[] = {VL53L5CX_NB_TARGET_PER_ZONE, 0x00, 0x01, 0x00};
	uint32_t single_range = 0x01;

	/* SW reboot sequence */
	if(const auto res = write_byte(0x7fff, 0x00); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x0009, 0x04); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x000F, 0x40); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x000A, 0x03); res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_byte(0x7FFF, &tmp); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x000C, 0x01); res.is_err()) return Err(res.unwrap_err());

	if(const auto res = write_byte(0x0101, 0x00); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x0102, 0x00); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x010A, 0x01); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x4002, 0x01); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x4002, 0x00); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x010A, 0x03); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x0103, 0x01); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x000C, 0x00); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x000F, 0x43); res.is_err()) return Err(res.unwrap_err());
    clock::delay(1ms);

	if(const auto res = write_byte(0x000F, 0x40); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x000A, 0x01); res.is_err()) return Err(res.unwrap_err());
    clock::delay(100ms);

	/* Wait for sensor booted (several ms required to get sensor ready ) */
	if(const auto res = write_byte(0x7fff, 0x00); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = poll_for_answer(1, 0, 0x06, 0xff, 1); res.is_err()) return Err(res.unwrap_err());

	if(const auto res = write_byte(0x000E, 0x01); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x02); res.is_err()) return Err(res.unwrap_err());

	/* Enable FW access */
	if(const auto res = write_byte(0x03, 0x0D); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x01); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = poll_for_answer(1, 0, 0x21, 0x10, 0x10); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x00); res.is_err()) return Err(res.unwrap_err());

	/* Enable host access to GO1 */
	if(const auto res = read_byte(0x7fff, &tmp); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x0C, 0x01); res.is_err()) return Err(res.unwrap_err());

	/* Power ON status */
	if(const auto res = write_byte(0x7fff, 0x00); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x101, 0x00); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x102, 0x00); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x010A, 0x01); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x4002, 0x01); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x4002, 0x00); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x010A, 0x03); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x103, 0x01); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x400F, 0x00); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x21A, 0x43); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x21A, 0x03); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x21A, 0x01); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x21A, 0x00); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x219, 0x00); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x21B, 0x00); res.is_err()) return Err(res.unwrap_err());

	/* Wake up MCU */
	if(const auto res = write_byte(0x7fff, 0x00); res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_byte(0x7fff, &tmp); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x0C, 0x00); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x01); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x20, 0x07); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x20, 0x06); res.is_err()) return Err(res.unwrap_err());

	/* Download FW into VL53L5 */
	if(const auto res = write_byte(0x7fff, 0x09); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_burst(0, &VL53L5CX_FIRMWARE[0],0x8000); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x0a); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_burst(0, &VL53L5CX_FIRMWARE[0x8000],0x8000); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x0b); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_burst(0, &VL53L5CX_FIRMWARE[0x10000],0x5000); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x01); res.is_err()) return Err(res.unwrap_err());

	/* Check if FW correctly downloaded */
	if(const auto res = write_byte(0x7fff, 0x02); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x03, 0x0D); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x01); res.is_err()) return Err(res.unwrap_err());
	this->poll_for_answer(1, 0, 0x21, 0x10, 0x10);
	if(const auto res = write_byte(0x7fff, 0x00); res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_byte(0x7fff, &tmp); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x0C, 0x01); res.is_err()) return Err(res.unwrap_err());

	/* Reset MCU and wait boot */
	if(const auto res = write_byte(0x7FFF, 0x00); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x114, 0x00); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x115, 0x00); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x116, 0x42); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x117, 0x00); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x0B, 0x00); res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_byte(0x7fff, &tmp); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x0C, 0x00); res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x0B, 0x01); res.is_err()) return Err(res.unwrap_err());
	status |= this->poll_for_mcu_boot();
	if(const auto res = write_byte(0x7fff, 0x02); res.is_err()) return Err(res.unwrap_err());

	/* Get offset NVM data and store them into the offset buffer */
	status |= write_burst( 0x2fd8,
		VL53L5CX_GET_NVM_CMD, sizeof(VL53L5CX_GET_NVM_CMD));
	if(const auto res = poll_for_answer(4, 0, res.is_err()) return Err(res.unwrap_err());
		VL53L5CX_UI_CMD_STATUS, 0xff, 2);
	status |= read_burst( VL53L5CX_UI_CMD_START,
		this->temp_buffer, VL53L5CX_NVM_DATA_SIZE);
	(void)memcpy(this->offset_data, this->temp_buffer,
		VL53L5CX_OFFSET_BUFFER_SIZE);
	status |= this->send_offset_data(VL53L5CX_RESOLUTION_4X4);

	/* Set default Xtalk shape. Send Xtalk to sensor */
	(void)memcpy(this->xtalk_data, VL53L5CX_DEFAULT_XTALK,
		VL53L5CX_XTALK_BUFFER_SIZE);
	status |= this->send_xtalk_data(VL53L5CX_RESOLUTION_4X4);

	/* Send default configuration to VL53L5CX firmware */
	status |= write_burst( 0x2c34,
		this->default_configuration,
		sizeof(VL53L5CX_DEFAULT_CONFIGURATION));
	if(const auto res = poll_for_answer(4, 1, res.is_err()) return Err(res.unwrap_err());
		VL53L5CX_UI_CMD_STATUS, 0xff, 0x03);

	status |= vl53l5cx_dci_write_data(&pipe_ctrl,
		VL53L5CX_DCI_PIPE_CONTROL, (uint16_t)sizeof(pipe_ctrl));
#if VL53L5CX_NB_TARGET_PER_ZONE != 1
	tmp = VL53L5CX_NB_TARGET_PER_ZONE;
	status |= vl53l5cx_dci_replace_data(this->temp_buffer,
		VL53L5CX_DCI_FW_NB_TARGET, 16,
	&tmp, 1, 0x0C);
#endif

	status |= vl53l5cx_dci_write_data(&single_range,
			VL53L5CX_DCI_SINGLE_RANGE,
			(uint16_t)sizeof(single_range));

    return Ok();
}


IResult<> VL53L5CX::poll_for_answer(
		uint8_t					size,
		uint8_t					pos,
		uint16_t				address,
		uint8_t					mask,
		uint8_t					expected_value)
{
	uint8_t status = VL53L5CX_STATUS_OK;
	uint8_t timeout = 0;

	do {
		if(const auto res = read_burst(address,this->temp_buffer, size); 
            res.is_err()) return Err(res.unwrap_err());

        clock::delay(10ms);

		if(timeout >= (uint8_t)200)	/* 2s timeout */
		{
			status |= (uint8_t)VL53L5CX_STATUS_TIMEOUT_ERROR;
		}else if((size >= (uint8_t)4) && (this->temp_buffer[2] >= (uint8_t)0x7f))
		{
			status |= VL53L5CX_MCU_ERROR;
		}
		else
		{
			timeout++;
		}
	}while ((this->temp_buffer[pos] & mask) != expected_value);

	return Ok();
}

/*
 * Inner function, not available outside this file. This function is used to
 * wait for the MCU to boot.
 */
IResult<> VL53L5CX::poll_for_mcu_boot()
{
   uint8_t go2_status0, go2_status1, status = VL53L5CX_STATUS_OK;
   uint16_t timeout = 0;

   do {
		status |= read_byte(0x06, &go2_status0);
		if((go2_status0 & (uint8_t)0x80) != (uint8_t)0){
			status |= RdByte(0x07, &go2_status1);
			status |= go2_status1;
			break;
		}
		// (void)WaitMs(1);
        clock::delay(1ms);
		timeout++;

		if((go2_status0 & (uint8_t)0x1) != (uint8_t)0){
			break;
		}

	}while (timeout < (uint16_t)500);

   return Ok();
}

/**
 * @brief Inner function, not available outside this file. This function is used
 * to set the offset data gathered from NVM.
 */

IResult<> VL53L5CX::send_offset_data(
		uint8_t						resolution)
{
	uint8_t status = VL53L5CX_STATUS_OK;
	uint32_t signal_grid[64];
	int16_t range_grid[64];
	uint8_t dss_4x4[] = {0x0F, 0x04, 0x04, 0x00, 0x08, 0x10, 0x10, 0x07};
	uint8_t footer[] = {0x00, 0x00, 0x00, 0x0F, 0x03, 0x01, 0x01, 0xE4};
	int8_t i, j;
	uint16_t k;

	(void)memcpy(this->temp_buffer,
               this->offset_data, VL53L5CX_OFFSET_BUFFER_SIZE);

	/* Data extrapolation is required for 4X4 offset */
	if(resolution == (uint8_t)VL53L5CX_RESOLUTION_4X4){
		(void)memcpy(&(this->temp_buffer[0x10]), dss_4x4, sizeof(dss_4x4));
		SwapBuffer(this->temp_buffer, VL53L5CX_OFFSET_BUFFER_SIZE);
		(void)memcpy(signal_grid,&(this->temp_buffer[0x3C]),
			sizeof(signal_grid));
		(void)memcpy(range_grid,&(this->temp_buffer[0x140]),
			sizeof(range_grid));

		for (j = 0; j < (int8_t)4; j++)
		{
			for (i = 0; i < (int8_t)4 ; i++)
			{
				signal_grid[i+(4*j)] =
				(signal_grid[(2*i)+(16*j)+ (int8_t)0]
				+ signal_grid[(2*i)+(16*j)+(int8_t)1]
				+ signal_grid[(2*i)+(16*j)+(int8_t)8]
				+ signal_grid[(2*i)+(16*j)+(int8_t)9])
                                  /(uint32_t)4;
				range_grid[i+(4*j)] =
				(range_grid[(2*i)+(16*j)]
				+ range_grid[(2*i)+(16*j)+1]
				+ range_grid[(2*i)+(16*j)+8]
				+ range_grid[(2*i)+(16*j)+9])
                                  /(int16_t)4;
			}
		}
	    (void)memset(&range_grid[0x10], 0, (uint16_t)96);
	    (void)memset(&signal_grid[0x10], 0, (uint16_t)192);
            (void)memcpy(&(this->temp_buffer[0x3C]),
		signal_grid, sizeof(signal_grid));
            (void)memcpy(&(this->temp_buffer[0x140]),
		range_grid, sizeof(range_grid));
            SwapBuffer(this->temp_buffer, VL53L5CX_OFFSET_BUFFER_SIZE);
	}

	for(k = 0; k < (VL53L5CX_OFFSET_BUFFER_SIZE - (uint16_t)4); k++)
	{
		this->temp_buffer[k] = this->temp_buffer[k + (uint16_t)8];
	}

	(void)memcpy(&(this->temp_buffer[0x1E0]), footer, 8);
	status |= write_burst(0x2e18, this->temp_buffer,
		VL53L5CX_OFFSET_BUFFER_SIZE);
	status |=this->poll_for_answer(4, 1,
		VL53L5CX_UI_CMD_STATUS, 0xff, 0x03);

    return Ok();
}

/**
 * @brief Inner function, not available outside this file. This function is used
 * to set the Xtalk data from generic configuration, or user's calibration.
 */

IResult<> send_xtalk_data(uint8_t resolution){
	uint8_t status = VL53L5CX_STATUS_OK;
	uint8_t res4x4[] = {0x0F, 0x04, 0x04, 0x17, 0x08, 0x10, 0x10, 0x07};
	uint8_t dss_4x4[] = {0x00, 0x78, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08};
	uint8_t profile_4x4[] = {0xA0, 0xFC, 0x01, 0x00};
	uint32_t signal_grid[64];
	int8_t i, j;

	(void)memcpy(this->temp_buffer, &(this->xtalk_data[0]),
		VL53L5CX_XTALK_BUFFER_SIZE);

	/* Data extrapolation is required for 4X4 Xtalk */
	if(resolution == (uint8_t)VL53L5CX_RESOLUTION_4X4)
	{
		(void)memcpy(&(this->temp_buffer[0x8]),
			res4x4, sizeof(res4x4));
		(void)memcpy(&(this->temp_buffer[0x020]),
			dss_4x4, sizeof(dss_4x4));

		SwapBuffer(this->temp_buffer, VL53L5CX_XTALK_BUFFER_SIZE);
		(void)memcpy(signal_grid, &(this->temp_buffer[0x34]),
			sizeof(signal_grid));

		for (j = 0; j < (int8_t)4; j++)
		{
			for (i = 0; i < (int8_t)4 ; i++)
			{
				signal_grid[i+(4*j)] =
				(signal_grid[(2*i)+(16*j)+0]
				+ signal_grid[(2*i)+(16*j)+1]
				+ signal_grid[(2*i)+(16*j)+8]
				+ signal_grid[(2*i)+(16*j)+9])/(uint32_t)4;
			}
		}
	    (void)memset(&signal_grid[0x10], 0, (uint32_t)192);
	    (void)memcpy(&(this->temp_buffer[0x34]),
                  signal_grid, sizeof(signal_grid));
	    SwapBuffer(this->temp_buffer, VL53L5CX_XTALK_BUFFER_SIZE);
	    (void)memcpy(&(this->temp_buffer[0x134]),
	    profile_4x4, sizeof(profile_4x4));
	    (void)memset(&(this->temp_buffer[0x078]),0 ,
                         (uint32_t)4*sizeof(uint8_t));
	}

	status |= write_burst(0x2cf8,
			this->temp_buffer, VL53L5CX_XTALK_BUFFER_SIZE);
	status |=this->poll_for_answer(4, 1,
			VL53L5CX_UI_CMD_STATUS, 0xff, 0x03);

    return Ok();
}

IResult<> VL53L5CX::is_alive(
		uint8_t				*p_is_alive)
{
	uint8_t status = VL53L5CX_STATUS_OK;
	uint8_t device_id, revision_id;

	status |= WrByte(0x7fff, 0x00);
	status |= RdByte(0, &device_id);
	status |= RdByte(1, &revision_id);
	status |= WrByte(0x7fff, 0x02);

	if((device_id == (uint8_t)0xF0) && (revision_id == (uint8_t)0x02))
	{
		*p_is_alive = 1;
	}
	else
	{
		*p_is_alive = 0;
	}

    return Ok();
}


IResult<> VL53L5CX::set_i2c_address(
		uint16_t		        i2c_address)
{
	uint8_t status = VL53L5CX_STATUS_OK;

	status |= WrByte(0x7fff, 0x00);
	status |= WrByte(0x4, (uint8_t)(i2c_address >> 1));
	this->platform.address = i2c_address;
	status |= WrByte(0x7fff, 0x02);

    return Ok();
}

IResult<> VL53L5CX::get_power_mode(
		uint8_t				*p_power_mode)
{
	uint8_t tmp, status = VL53L5CX_STATUS_OK;

	status |= WrByte(0x7FFF, 0x00);
	status |= RdByte(0x009, &tmp);

	switch(tmp)
	{
		case 0x4:
			*p_power_mode = VL53L5CX_POWER_MODE_WAKEUP;
			break;
		case 0x2:
			*p_power_mode = VL53L5CX_POWER_MODE_SLEEP;

			break;
		default:
			*p_power_mode = 0;
			status = VL53L5CX_STATUS_ERROR;
			break;
	}

	status |= WrByte(0x7FFF, 0x02);

    return Ok();
}

IResult<> VL53L5CX::set_power_mode(
		uint8_t			        power_mode)
{
	uint8_t current_power_mode, status = VL53L5CX_STATUS_OK;

	status |= vl53l5cx_get_power_mode(&current_power_mode);
	if(power_mode != current_power_mode)
	{
	switch(power_mode)
	{
		case VL53L5CX_POWER_MODE_WAKEUP:
			status |= WrByte(0x7FFF, 0x00);
			status |= WrByte(0x09, 0x04);
			if(const auto res = poll_for_answer( res.is_err()) return Err(res.unwrap_err());
						this, 1, 0, 0x06, 0x01, 1);
			break;

		case VL53L5CX_POWER_MODE_SLEEP:
			status |= WrByte(0x7FFF, 0x00);
			status |= WrByte(0x09, 0x02);
			if(const auto res = poll_for_answer( res.is_err()) return Err(res.unwrap_err());
						this, 1, 0, 0x06, 0x01, 0);
			break;

		default:
			status = VL53L5CX_STATUS_ERROR;
			break;
		}
		status |= WrByte(0x7FFF, 0x02);
	}

    return Ok();
}

IResult<> VL53L5CX::start_ranging()
{
	uint8_t resolution, status = VL53L5CX_STATUS_OK;
	uint16_t tmp;
	uint32_t i;
	uint32_t header_config[2] = {0, 0};

	union Block_header *bh_ptr;
	uint8_t cmd[] = {0x00, 0x03, 0x00, 0x00};

	status |= vl53l5cx_get_resolution(&resolution);
	this->data_read_size = 0;
	this->streamcount = 255;

	/* Enable mandatory output (meta and common data) */
	uint32_t output_bh_enable[] = {
		0x00000007U,
		0x00000000U,
		0x00000000U,
		0xC0000000U};

	/* Send addresses of possible output */
	uint32_t output[] ={VL53L5CX_START_BH,
		VL53L5CX_METADATA_BH,
		VL53L5CX_COMMONDATA_BH,
		VL53L5CX_AMBIENT_RATE_BH,
		VL53L5CX_SPAD_COUNT_BH,
		VL53L5CX_NB_TARGET_DETECTED_BH,
		VL53L5CX_SIGNAL_RATE_BH,
		VL53L5CX_RANGE_SIGMA_MM_BH,
		VL53L5CX_DISTANCE_BH,
		VL53L5CX_REFLECTANCE_BH,
		VL53L5CX_TARGET_STATUS_BH,
		VL53L5CX_MOTION_DETECT_BH};

	/* Enable selected outputs in the 'platform.h' file */
#ifndef VL53L5CX_DISABLE_AMBIENT_PER_SPAD
	output_bh_enable[0] += (uint32_t)8;
#endif
#ifndef VL53L5CX_DISABLE_NB_SPADS_ENABLED
	output_bh_enable[0] += (uint32_t)16;
#endif
#ifndef VL53L5CX_DISABLE_NB_TARGET_DETECTED
	output_bh_enable[0] += (uint32_t)32;
#endif
#ifndef VL53L5CX_DISABLE_SIGNAL_PER_SPAD
	output_bh_enable[0] += (uint32_t)64;
#endif
#ifndef VL53L5CX_DISABLE_RANGE_SIGMA_MM
	output_bh_enable[0] += (uint32_t)128;
#endif
#ifndef VL53L5CX_DISABLE_DISTANCE_MM
	output_bh_enable[0] += (uint32_t)256;
#endif
#ifndef VL53L5CX_DISABLE_REFLECTANCE_PERCENT
	output_bh_enable[0] += (uint32_t)512;
#endif
#ifndef VL53L5CX_DISABLE_TARGET_STATUS
	output_bh_enable[0] += (uint32_t)1024;
#endif
#ifndef VL53L5CX_DISABLE_MOTION_INDICATOR
	output_bh_enable[0] += (uint32_t)2048;
#endif

	/* Update data size */
	for (i = 0; i < (uint32_t)(sizeof(output)/sizeof(uint32_t)); i++)
	{
		if ((output[i] == (uint8_t)0) 
                    || ((output_bh_enable[i/(uint32_t)32]
                         &((uint32_t)1 << (i%(uint32_t)32))) == (uint32_t)0))
		{
			continue;
		}

		bh_ptr = (union Block_header *)&(output[i]);
		if (((uint8_t)bh_ptr->type >= (uint8_t)0x1) 
                    && ((uint8_t)bh_ptr->type < (uint8_t)0x0d))
		{
			if ((bh_ptr->idx >= (uint16_t)0x54d0) 
                            && (bh_ptr->idx < (uint16_t)(0x54d0 + 960)))
			{
				bh_ptr->size = resolution;
			}
			else
			{
				bh_ptr->size = (uint8_t)(resolution 
                                  * (uint8_t)VL53L5CX_NB_TARGET_PER_ZONE);
			}
			this->data_read_size += bh_ptr->type * bh_ptr->size;
		}
		else
		{
			this->data_read_size += bh_ptr->size;
		}
		this->data_read_size += (uint32_t)4;
	}
	this->data_read_size += (uint32_t)20;

	status |= vl53l5cx_dci_write_data(this,
			&(output), VL53L5CX_DCI_OUTPUT_LIST,
			(uint16_t)sizeof(output));

	header_config[0] = this->data_read_size;
	header_config[1] = i + (uint32_t)1;

	status |= vl53l5cx_dci_write_data(this,
			&(header_config), VL53L5CX_DCI_OUTPUT_CONFIG,
			(uint16_t)sizeof(header_config));

	status |= vl53l5cx_dci_write_data(this,
			&(output_bh_enable), VL53L5CX_DCI_OUTPUT_ENABLES,
			(uint16_t)sizeof(output_bh_enable));

	/* Start xshut bypass (interrupt mode) */
	status |= WrByte(0x7fff, 0x00);
	status |= WrByte(0x09, 0x05);
	status |= WrByte(0x7fff, 0x02);

	/* Start ranging session */
	status |= write_burst(VL53L5CX_UI_CMD_END -
			(uint16_t)(4 - 1), cmd, sizeof(cmd));
	if(const auto res = poll_for_answer(4, 1, res.is_err()) return Err(res.unwrap_err());
			VL53L5CX_UI_CMD_STATUS, 0xff, 0x03);

	/* Read ui range data content and compare if data size is the correct one */
	status += vl53l5cx_dci_read_data(this,
			this->temp_buffer, 0x5440, 12);
	(void)memcpy(&tmp, &(this->temp_buffer[0x8]), sizeof(tmp));
	if(tmp != this->data_read_size)
	{
		status |= VL53L5CX_STATUS_ERROR;
	}

    return Ok();
}

IResult<> VL53L5CX::stop_ranging()
{
	uint8_t tmp = 0, status = VL53L5CX_STATUS_OK;
	uint16_t timeout = 0;
	uint32_t auto_stop_flag = 0;

	status |= read_burst(&(this->platform),
                          0x2FFC, &auto_stop_flag, 4);
	if(auto_stop_flag != (uint32_t)0x4FF)
	{
		status |= WrByte(0x7fff, 0x00);

		/* Provoke MCU stop */
		status |= WrByte(0x15, 0x16);
		status |= WrByte(0x14, 0x01);

		/* Poll for G02 status 0 MCU stop */
		while(((tmp & (uint8_t)0x80) >> 7) == (uint8_t)0x00)
		{
			status |= RdByte(0x6, &tmp);
			status |= WaitMs(10);
			timeout++;	/* Timeout reached after 5 seconds */

			if(timeout > (uint16_t)500)
			{
				status |= tmp;
				break;
			}
		}
	}

	/* Check GO2 status 1 if status is still OK */
	status += RdByte(0x6, &tmp);
	if((tmp & (uint8_t)0x80) != (uint8_t)0){
		status += RdByte(0x7, &tmp);
		if((tmp != (uint8_t)0x84) && (tmp != (uint8_t)0x85)){
		   status |= tmp;
		}
	}

	/* Undo MCU stop */
	status |= WrByte(0x7fff, 0x00);
	status |= WrByte(0x14, 0x00);
	status |= WrByte(0x15, 0x00);

	/* Stop xshut bypass */
	status |= WrByte(0x09, 0x04);
	status |= WrByte(0x7fff, 0x02);

    return Ok();
}

IResult<> VL53L5CX::check_data_ready(
		uint8_t				*p_isReady)
{
	uint8_t status = VL53L5CX_STATUS_OK;

	status |= read_burst(0x0, this->temp_buffer, 4);

	if((this->temp_buffer[0] != this->streamcount)
			&& (this->temp_buffer[0] != (uint8_t)255)
			&& (this->temp_buffer[1] == (uint8_t)0x5)
			&& ((this->temp_buffer[2] & (uint8_t)0x5) == (uint8_t)0x5)
			&& ((this->temp_buffer[3] & (uint8_t)0x10) ==(uint8_t)0x10)
			)
	{
		*p_isReady = (uint8_t)1;
        this->streamcount = this->temp_buffer[0];
	}else{
        if ((this->temp_buffer[3] & (uint8_t)0x80) != (uint8_t)0){
        	status |= this->temp_buffer[2];	/* Return GO2 error status */
        }

		*p_isReady = 0;
	}

    return Ok();
}

IResult<> VL53L5CX::get_ranging_data(
		VL53L5CX_ResultsData		*p_results)
{
	uint8_t status = VL53L5CX_STATUS_OK;
	union Block_header *bh_ptr;
	size_t msize;
	status |= read_burst(0x0,
			this->temp_buffer, this->data_read_size);
	this->streamcount = this->temp_buffer[0];
	SwapBuffer(this->temp_buffer, (uint16_t)this->data_read_size);

	/* Start conversion at position 16 to avoid headers */
	for (size_t i = (uint32_t)16; i < (uint32_t)this->data_read_size; i+=(uint32_t)4){
		bh_ptr = (union Block_header *)&(this->temp_buffer[i]);
		if ((bh_ptr->type > (uint32_t)0x1) 
                    && (bh_ptr->type < (uint32_t)0xd)){
			msize = bh_ptr->type * bh_ptr->size;
		}else{
			msize = bh_ptr->size;
		}

		switch(bh_ptr->idx){
			case VL53L5CX_METADATA_IDX:
				p_results->silicon_temp_degc =
						(int8_t)this->temp_buffer[i + (uint32_t)12];
				break;

#ifndef VL53L5CX_DISABLE_AMBIENT_PER_SPAD
			case VL53L5CX_AMBIENT_RATE_IDX:
				(void)memcpy(p_results->ambient_per_spad,
				&(this->temp_buffer[i + (uint32_t)4]), msize);
				break;
#endif
#ifndef VL53L5CX_DISABLE_NB_SPADS_ENABLED
			case VL53L5CX_SPAD_COUNT_IDX:
				(void)memcpy(p_results->nb_spads_enabled,
				&(this->temp_buffer[i + (uint32_t)4]), msize);
				break;
#endif
#ifndef VL53L5CX_DISABLE_NB_TARGET_DETECTED
			case VL53L5CX_NB_TARGET_DETECTED_IDX:
				(void)memcpy(p_results->nb_target_detected,
				&(this->temp_buffer[i + (uint32_t)4]), msize);
				break;
#endif
#ifndef VL53L5CX_DISABLE_SIGNAL_PER_SPAD
			case VL53L5CX_SIGNAL_RATE_IDX:
				(void)memcpy(p_results->signal_per_spad,
				&(this->temp_buffer[i + (uint32_t)4]), msize);
				break;
#endif
#ifndef VL53L5CX_DISABLE_RANGE_SIGMA_MM
			case VL53L5CX_RANGE_SIGMA_MM_IDX:
				(void)memcpy(p_results->range_sigma_mm,
				&(this->temp_buffer[i + (uint32_t)4]), msize);
				break;
#endif
#ifndef VL53L5CX_DISABLE_DISTANCE_MM
			case VL53L5CX_DISTANCE_IDX:
				(void)memcpy(p_results->distance_mm,
				&(this->temp_buffer[i + (uint32_t)4]), msize);
				break;
#endif
#ifndef VL53L5CX_DISABLE_REFLECTANCE_PERCENT
			case VL53L5CX_REFLECTANCE_EST_PC_IDX:
				(void)memcpy(p_results->reflectance,
				&(this->temp_buffer[i + (uint32_t)4]), msize);
				break;
#endif
#ifndef VL53L5CX_DISABLE_TARGET_STATUS
			case VL53L5CX_TARGET_STATUS_IDX:
				(void)memcpy(p_results->target_status,
				&(this->temp_buffer[i + (uint32_t)4]), msize);
				break;
#endif
#ifndef VL53L5CX_DISABLE_MOTION_INDICATOR
			case VL53L5CX_MOTION_DETEC_IDX:
				(void)memcpy(&p_results->motion_indicator,
				&(this->temp_buffer[i + (uint32_t)4]), msize);
				break;
#endif
			default:
				break;
		}
		i += msize;
	}

#ifndef VL53L5CX_USE_RAW_FORMAT

	/* Convert data into their real format */
#ifndef VL53L5CX_DISABLE_AMBIENT_PER_SPAD
	for(i = 0; i < (uint32_t)VL53L5CX_RESOLUTION_8X8; i++)
	{
		p_results->ambient_per_spad[i] /= (uint32_t)2048;
	}
#endif

	for(i = 0; i < (uint32_t)(VL53L5CX_RESOLUTION_8X8
			*VL53L5CX_NB_TARGET_PER_ZONE); i++)
	{
#ifndef VL53L5CX_DISABLE_DISTANCE_MM
		p_results->distance_mm[i] /= 4;
		if(p_results->distance_mm[i] < 0)
		{
			p_results->distance_mm[i] = 0;
		}
#endif
#ifndef VL53L5CX_DISABLE_REFLECTANCE_PERCENT
		p_results->reflectance[i] /= (uint8_t)2;
#endif
#ifndef VL53L5CX_DISABLE_RANGE_SIGMA_MM
		p_results->range_sigma_mm[i] /= (uint16_t)128;
#endif
#ifndef VL53L5CX_DISABLE_SIGNAL_PER_SPAD
		p_results->signal_per_spad[i] /= (uint32_t)2048;
#endif
	}

	/* Set target status to 255 if no target is detected for this zone */
#ifndef VL53L5CX_DISABLE_NB_TARGET_DETECTED
	for(i = 0; i < (uint32_t)VL53L5CX_RESOLUTION_8X8; i++)
	{
		if(p_results->nb_target_detected[i] == (uint8_t)0){
			for(j = 0; j < (uint32_t)
				VL53L5CX_NB_TARGET_PER_ZONE; j++)
			{
#ifndef VL53L5CX_DISABLE_TARGET_STATUS
				p_results->target_status
				[((uint32_t)VL53L5CX_NB_TARGET_PER_ZONE
					*(uint32_t)i) + j]=(uint8_t)255;
#endif
			}
		}
	}
#endif

#ifndef VL53L5CX_DISABLE_MOTION_INDICATOR
	for(i = 0; i < (uint32_t)32; i++)
	{
		p_results->motion_indicator.motion[i] /= (uint32_t)65535;
	}
#endif

#endif
    return Ok();
}

IResult<> VL53L5CX::get_resolution(
		uint8_t				*p_resolution)
{
	uint8_t status = VL53L5CX_STATUS_OK;

	status |= vl53l5cx_dci_read_data(this->temp_buffer,
			VL53L5CX_DCI_ZONE_CONFIG, 8);
	*p_resolution = this->temp_buffer[0x00]*this->temp_buffer[0x01];

    return Ok();
}



IResult<> VL53L5CX::set_resolution(
		VL53L5CX_Configuration 		 *this,
		uint8_t				resolution)
{
	uint8_t status = VL53L5CX_STATUS_OK;

	switch(resolution){
		case VL53L5CX_RESOLUTION_4X4:
			status |= vl53l5cx_dci_read_data(this,
					this->temp_buffer,
					VL53L5CX_DCI_DSS_CONFIG, 16);
			this->temp_buffer[0x04] = 64;
			this->temp_buffer[0x06] = 64;
			this->temp_buffer[0x09] = 4;
			status |= vl53l5cx_dci_write_data(this,
					this->temp_buffer,
					VL53L5CX_DCI_DSS_CONFIG, 16);

			status |= vl53l5cx_dci_read_data(this,
					this->temp_buffer,
					VL53L5CX_DCI_ZONE_CONFIG, 8);
			this->temp_buffer[0x00] = 4;
			this->temp_buffer[0x01] = 4;
			this->temp_buffer[0x04] = 8;
			this->temp_buffer[0x05] = 8;
			status |= vl53l5cx_dci_write_data(this,
					this->temp_buffer,
					VL53L5CX_DCI_ZONE_CONFIG, 8);
			break;

		case VL53L5CX_RESOLUTION_8X8:
			status |= vl53l5cx_dci_read_data(this,
					this->temp_buffer,
					VL53L5CX_DCI_DSS_CONFIG, 16);
			this->temp_buffer[0x04] = 16;
			this->temp_buffer[0x06] = 16;
			this->temp_buffer[0x09] = 1;
			status |= vl53l5cx_dci_write_data(this,
					this->temp_buffer,
					VL53L5CX_DCI_DSS_CONFIG, 16);

			status |= vl53l5cx_dci_read_data(this,
					this->temp_buffer,
					VL53L5CX_DCI_ZONE_CONFIG, 8);
			this->temp_buffer[0x00] = 8;
			this->temp_buffer[0x01] = 8;
			this->temp_buffer[0x04] = 4;
			this->temp_buffer[0x05] = 4;
			status |= vl53l5cx_dci_write_data(this,
					this->temp_buffer,
					VL53L5CX_DCI_ZONE_CONFIG, 8);

			break;

		default:
			status = VL53L5CX_STATUS_INVALID_PARAM;
			break;
		}

	status |= this->send_offset_data(resolution);
	status |= this->send_xtalk_data(resolution);

    return Ok();
}

IResult<> VL53L5CX::get_ranging_frequency_hz(
		uint8_t				*p_frequency_hz)
{
	uint8_t status = VL53L5CX_STATUS_OK;

	status |= vl53l5cx_dci_read_data(this->temp_buffer,
			VL53L5CX_DCI_FREQ_HZ, 4);
	*p_frequency_hz = this->temp_buffer[0x01];

    return Ok();
}

IResult<> VL53L5CX::set_ranging_frequency_hz(
		uint8_t				frequency_hz)
{
	uint8_t status = VL53L5CX_STATUS_OK;

	status |= vl53l5cx_dci_replace_data(this->temp_buffer,
					VL53L5CX_DCI_FREQ_HZ, 4,
					&frequency_hz, 1, 0x01);

    return Ok();
}

IResult<> VL53L5CX::get_integration_time_ms(
		uint32_t			*p_time_ms)
{
	uint8_t status = VL53L5CX_STATUS_OK;

	status |= vl53l5cx_dci_read_data(this->temp_buffer,
			VL53L5CX_DCI_INT_TIME, 20);

	(void)memcpy(p_time_ms, &(this->temp_buffer[0x0]), 4);
	*p_time_ms /= (uint32_t)1000;

    return Ok();
}

IResult<> VL53L5CX::set_integration_time_ms(
		uint32_t			integration_time_ms)
{
	uint8_t status = VL53L5CX_STATUS_OK;
        uint32_t integration = integration_time_ms;

	/* Integration time must be between 2ms and 1000ms */
	if((integration < (uint32_t)2)
           || (integration > (uint32_t)1000))
	{
		status |= VL53L5CX_STATUS_INVALID_PARAM;
	}else
	{
		integration *= (uint32_t)1000;

		status |= vl53l5cx_dci_replace_data(this->temp_buffer,
				VL53L5CX_DCI_INT_TIME, 20,
				&integration, 4, 0x00);
	}

    return Ok();
}

IResult<> VL53L5CX::get_sharpener_percent(
		uint8_t				*p_sharpener_percent)
{
	uint8_t status = VL53L5CX_STATUS_OK;

	status |= vl53l5cx_dci_read_data(this,this->temp_buffer,
			VL53L5CX_DCI_SHARPENER, 16);

	*p_sharpener_percent = (this->temp_buffer[0xD]
                                *(uint8_t)100)/(uint8_t)255;

    return Ok();
}

IResult<> VL53L5CX::set_sharpener_percent(
		uint8_t				sharpener_percent)
{
	uint8_t status = VL53L5CX_STATUS_OK;
        uint8_t sharpener;

	if(sharpener_percent >= (uint8_t)100)
	{
		status |= VL53L5CX_STATUS_INVALID_PARAM;
	}
	else
	{
		sharpener = (sharpener_percent*(uint8_t)255)/(uint8_t)100;
		status |= vl53l5cx_dci_replace_data(this->temp_buffer,
				VL53L5CX_DCI_SHARPENER, 16,
                                &sharpener, 1, 0xD);
	}

    return Ok();
}

IResult<> VL53L5CX::get_target_order(
		uint8_t				*p_target_order)
{
	uint8_t status = VL53L5CX_STATUS_OK;

	status |= vl53l5cx_dci_read_data(this->temp_buffer,
			VL53L5CX_DCI_TARGET_ORDER, 4);
	*p_target_order = (uint8_t)this->temp_buffer[0x0];

    return Ok();
}

IResult<> VL53L5CX::set_target_order(
		uint8_t				target_order)
{
	uint8_t status = VL53L5CX_STATUS_OK;

	if((target_order == (uint8_t)VL53L5CX_TARGET_ORDER_CLOSEST)
		|| (target_order == (uint8_t)VL53L5CX_TARGET_ORDER_STRONGEST))
	{
		status |= vl53l5cx_dci_replace_data(this->temp_buffer,
				VL53L5CX_DCI_TARGET_ORDER, 4,
                                &target_order, 1, 0x0);
	}else
	{
		status |= VL53L5CX_STATUS_INVALID_PARAM;
	}

    return Ok();
}

IResult<> VL53L5CX::get_ranging_mode(
		uint8_t				*p_ranging_mode)
{
	uint8_t status = VL53L5CX_STATUS_OK;

	status |= vl53l5cx_dci_read_data(this->temp_buffer,
			VL53L5CX_DCI_RANGING_MODE, 8);

	if(this->temp_buffer[0x01] == (uint8_t)0x1)
	{
		*p_ranging_mode = VL53L5CX_RANGING_MODE_CONTINUOUS;
	}
	else
	{
		*p_ranging_mode = VL53L5CX_RANGING_MODE_AUTONOMOUS;
	}

    return Ok();
}

IResult<> VL53L5CX::set_ranging_mode(
		uint8_t				ranging_mode)
{
	uint8_t status = VL53L5CX_STATUS_OK;
	uint32_t single_range = 0x00;

	status |= vl53l5cx_dci_read_data(this->temp_buffer,
			VL53L5CX_DCI_RANGING_MODE, 8);

	switch(ranging_mode)
	{
		case VL53L5CX_RANGING_MODE_CONTINUOUS:
			this->temp_buffer[0x01] = 0x1;
			this->temp_buffer[0x03] = 0x3;
			single_range = 0x00;
			break;

		case VL53L5CX_RANGING_MODE_AUTONOMOUS:
			this->temp_buffer[0x01] = 0x3;
			this->temp_buffer[0x03] = 0x2;
			single_range = 0x01;
			break;

		default:
			status = VL53L5CX_STATUS_INVALID_PARAM;
			break;
	}

	status |= vl53l5cx_dci_write_data(this->temp_buffer,
			VL53L5CX_DCI_RANGING_MODE, (uint16_t)8);

	status |= vl53l5cx_dci_write_data(&single_range,
			VL53L5CX_DCI_SINGLE_RANGE, 
                        (uint16_t)sizeof(single_range));

    return Ok();
}

IResult<> VL53L5CX::dci_read_data(
		uint8_t				*data,
		uint32_t			index,
		uint16_t			data_size)
{
	int16_t i;
	uint8_t status = VL53L5CX_STATUS_OK;
        uint32_t rd_size = (uint32_t) data_size + (uint32_t)12;
	uint8_t cmd[] = {0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x0f,
			0x00, 0x02, 0x00, 0x08};

	/* Check if tmp buffer is large enough */
	if((data_size + (uint16_t)12)>(uint16_t)VL53L5CX_TEMPORARY_BUFFER_SIZE)
	{
		status |= VL53L5CX_STATUS_ERROR;
	}
	else
	{
		cmd[0] = (uint8_t)(index >> 8);	
		cmd[1] = (uint8_t)(index & (uint32_t)0xff);			
		cmd[2] = (uint8_t)((data_size & (uint16_t)0xff0) >> 4);
		cmd[3] = (uint8_t)((data_size & (uint16_t)0xf) << 4);

	/* Request data reading from FW */
		status |= write_burst(&(this->platform),
			(VL53L5CX_UI_CMD_END-(uint16_t)11),cmd, sizeof(cmd));
		if(const auto res = poll_for_answer(4, 1, res.is_err()) return Err(res.unwrap_err());
			VL53L5CX_UI_CMD_STATUS,
			0xff, 0x03);

	/* Read new data sent (4 bytes header + data_size + 8 bytes footer) */
		status |= read_burst(VL53L5CX_UI_CMD_START,
			this->temp_buffer, rd_size);
		SwapBuffer(this->temp_buffer, data_size + (uint16_t)12);

	/* Copy data from FW into input structure (-4 bytes to remove header) */
		for(i = 0 ; i < (int16_t)data_size;i++){
			data[i] = this->temp_buffer[i + 4];
		}
	}

    return Ok();
}

IResult<> VL53L5CX::dci_write_data(
		uint8_t				*data,
		uint32_t			index,
		uint16_t			data_size)
{
	uint8_t status = VL53L5CX_STATUS_OK;
	int16_t i;

	uint8_t headers[] = {0x00, 0x00, 0x00, 0x00};
	uint8_t footer[] = {0x00, 0x00, 0x00, 0x0f, 0x05, 0x01,
			(uint8_t)((data_size + (uint16_t)8) >> 8), 
			(uint8_t)((data_size + (uint16_t)8) & (uint8_t)0xFF)};

	uint16_t address = (uint16_t)VL53L5CX_UI_CMD_END - 
		(data_size + (uint16_t)12) + (uint16_t)1;

	/* Check if cmd buffer is large enough */
	if((data_size + (uint16_t)12) 
           > (uint16_t)VL53L5CX_TEMPORARY_BUFFER_SIZE)
	{
		status |= VL53L5CX_STATUS_ERROR;
	}
	else
	{
		headers[0] = (uint8_t)(index >> 8);
		headers[1] = (uint8_t)(index & (uint32_t)0xff);
		headers[2] = (uint8_t)(((data_size & (uint16_t)0xff0) >> 4));
		headers[3] = (uint8_t)((data_size & (uint16_t)0xf) << 4);

	/* Copy data from structure to FW format (+4 bytes to add header) */
		SwapBuffer(data, data_size);
		for(i = (int16_t)data_size - (int16_t)1 ; i >= 0; i--)
		{
			this->temp_buffer[i + 4] = data[i];
		}

	/* Add headers and footer */
		(void)memcpy(&this->temp_buffer[0], headers, sizeof(headers));
		(void)memcpy(&this->temp_buffer[data_size + (uint16_t)4],
			footer, sizeof(footer));

	/* Send data to FW */
		status |= write_burst(&(this->platform),address,
			this->temp_buffer,
			(uint32_t)((uint32_t)data_size + (uint32_t)12));
		if(const auto res = poll_for_answer(4, 1, res.is_err()) return Err(res.unwrap_err());
			VL53L5CX_UI_CMD_STATUS, 0xff, 0x03);

		SwapBuffer(data, data_size);
	}

    return Ok();
}

IResult<> VL53L5CX::dci_replace_data(
		uint8_t				*data,
		uint32_t			index,
		uint16_t			data_size,
		uint8_t				*new_data,
		uint16_t			new_data_size,
		uint16_t			new_data_pos)
{
	uint8_t status = VL53L5CX_STATUS_OK;

	status |= vl53l5cx_dci_read_data(data, index, data_size);
	(void)memcpy(&(data[new_data_pos]), new_data, new_data_size);
	status |= vl53l5cx_dci_write_data(data, index, data_size);

    return Ok();
}

IResult<> VL53L5CX::get_detection_thresholds_enable(
		uint8_t				*p_enabled)
{
	uint8_t status = VL53L5CX_STATUS_OK;

	status |= vl53l5cx_dci_read_data(this->temp_buffer,
			VL53L5CX_DCI_DET_THRESH_GLOBAL_CONFIG, 8);
	*p_enabled = this->temp_buffer[0x1];

    return Ok();
}

IResult<> VL53L5CX::set_detection_thresholds_enable(
		uint8_t				enabled)
{
	uint8_t tmp, status = VL53L5CX_STATUS_OK;
	uint8_t grp_global_config[] = {0x01, 0x00, 0x01, 0x00};

	if(enabled == (uint8_t)1)
	{
		grp_global_config[0x01] = 0x01;
		tmp = 0x04;
	}
	else
	{
		grp_global_config[0x01] = 0x00;
		tmp = 0x0C;
	}

	/* Set global interrupt config */
	status |= vl53l5cx_dci_replace_data(this->temp_buffer,
			VL53L5CX_DCI_DET_THRESH_GLOBAL_CONFIG, 8,
			&grp_global_config, 4, 0x00);

	/* Update interrupt config */
	status |= vl53l5cx_dci_replace_data(this->temp_buffer,
			VL53L5CX_DCI_DET_THRESH_CONFIG, 20,
			&tmp, 1, 0x11);

    return Ok();
}

IResult<> VL53L5CX::get_detection_thresholds(VL53L5CX_DetectionThresholds	*p_thresholds)
{
	uint8_t i, status = VL53L5CX_STATUS_OK;

	/* Get thresholds configuration */
	status |= vl53l5cx_dci_read_data(p_thresholds,
			VL53L5CX_DCI_DET_THRESH_START, 
                        (uint16_t)VL53L5CX_NB_THRESHOLDS
			*(uint16_t)sizeof(VL53L5CX_DetectionThresholds));

	for(i = 0; i < (uint8_t)VL53L5CX_NB_THRESHOLDS; i++)
	{
		switch(p_thresholds[i].measurement)
		{
			case VL53L5CX_DISTANCE_MM:
				p_thresholds[i].param_low_thresh  /= 4;
				p_thresholds[i].param_high_thresh /= 4;
				break;
			case VL53L5CX_SIGNAL_PER_SPAD_KCPS:
				p_thresholds[i].param_low_thresh  /= 2048;
				p_thresholds[i].param_high_thresh /= 2048;
				break;
			case VL53L5CX_RANGE_SIGMA_MM:
				p_thresholds[i].param_low_thresh  /= 128;
				p_thresholds[i].param_high_thresh /= 128;
				break;
			case VL53L5CX_AMBIENT_PER_SPAD_KCPS:
				p_thresholds[i].param_low_thresh  /= 2048;
				p_thresholds[i].param_high_thresh /= 2048;
				break;
			case VL53L5CX_NB_SPADS_ENABLED:
				p_thresholds[i].param_low_thresh  /= 256;
				p_thresholds[i].param_high_thresh /= 256;
				break;
			case VL53L5CX_MOTION_INDICATOR:
				p_thresholds[i].param_low_thresh  /= 65535;
				p_thresholds[i].param_high_thresh /= 65535;
				break;
			default:
				break;
		}
	}

    return Ok();
}

IResult<> VL53L5CX::set_detection_thresholds(
		VL53L5CX_Configuration			*this,
		VL53L5CX_DetectionThresholds	*p_thresholds)
{
	uint8_t i, status = VL53L5CX_STATUS_OK;
	uint8_t grp_valid_target_cfg[] = {0x05, 0x05, 0x05, 0x05,
					0x05, 0x05, 0x05, 0x05};

	for(i = 0; i < (uint8_t) VL53L5CX_NB_THRESHOLDS; i++)
	{
		switch(p_thresholds[i].measurement)
		{
			case VL53L5CX_DISTANCE_MM:
				p_thresholds[i].param_low_thresh  *= 4;
				p_thresholds[i].param_high_thresh *= 4;
				break;
			case VL53L5CX_SIGNAL_PER_SPAD_KCPS:
				p_thresholds[i].param_low_thresh  *= 2048;
				p_thresholds[i].param_high_thresh *= 2048;
				break;
			case VL53L5CX_RANGE_SIGMA_MM:
				p_thresholds[i].param_low_thresh  *= 128;
				p_thresholds[i].param_high_thresh *= 128;
				break;
			case VL53L5CX_AMBIENT_PER_SPAD_KCPS:
				p_thresholds[i].param_low_thresh  *= 2048;
				p_thresholds[i].param_high_thresh *= 2048;
				break;
			case VL53L5CX_NB_SPADS_ENABLED:
				p_thresholds[i].param_low_thresh  *= 256;
				p_thresholds[i].param_high_thresh *= 256;
				break;
			case VL53L5CX_MOTION_INDICATOR:
				p_thresholds[i].param_low_thresh  *= 65535;
				p_thresholds[i].param_high_thresh *= 65535;
				break;
			default:
				break;
		}
	}

	/* Set valid target list */
	status |= vl53l5cx_dci_write_data(grp_valid_target_cfg,
			VL53L5CX_DCI_DET_THRESH_VALID_STATUS, 
			(uint16_t)sizeof(grp_valid_target_cfg));

	/* Set thresholds configuration */
	status |= vl53l5cx_dci_write_data(p_thresholds,
			VL53L5CX_DCI_DET_THRESH_START, 
			(uint16_t)(VL53L5CX_NB_THRESHOLDS
			*sizeof(VL53L5CX_DetectionThresholds)));

    return Ok();
}

IResult<> VL53L5CX::motion_indicator_init(
		VL53L5CX_Motion_Configuration	*p_motion_config,
		uint8_t				resolution)
{
	uint8_t status = VL53L5CX_STATUS_OK;

	(void)memset(p_motion_config, 0, sizeof(VL53L5CX_Motion_Configuration));

	p_motion_config->ref_bin_offset = 13633;
	p_motion_config->detection_threshold = 2883584;
	p_motion_config->extra_noise_sigma = 0;
	p_motion_config->null_den_clip_value = 0;
	p_motion_config->mem_update_mode = 6;
	p_motion_config->mem_update_choice = 2;
	p_motion_config->sum_span = 4;
	p_motion_config->feature_length = 9;
	p_motion_config->nb_of_aggregates = 16;
	p_motion_config->nb_of_temporal_accumulations = 16;
	p_motion_config->min_nb_for_global_detection = 1;
	p_motion_config->global_indicator_format_1 = 8;
	p_motion_config->global_indicator_format_2 = 0;
	p_motion_config->spare_1 = 0;
	p_motion_config->spare_2 = 0;
	p_motion_config->spare_3 = 0;

	status |= vl53l5cx_motion_indicator_set_resolution(this,
			p_motion_config, resolution);

    return Ok();
}

IResult<> VL53L5CX::motion_indicator_set_distance_motion(
		VL53L5CX_Motion_Configuration	*p_motion_config,
		uint16_t			distance_min_mm,
		uint16_t			distance_max_mm)
{
	uint8_t status = VL53L5CX_STATUS_OK;
	float_t tmp;

	if(((distance_max_mm - distance_min_mm) > (uint16_t)1500)
			|| (distance_min_mm < (uint16_t)400)
                        || (distance_max_mm > (uint16_t)4000))
	{
		status |= VL53L5CX_STATUS_INVALID_PARAM;
	}
	else
	{           
		tmp = (float_t)((((float_t)distance_min_mm/(float_t)37.5348)
                               -(float_t)4.0)*(float_t)2048.5);
                p_motion_config->ref_bin_offset = (int32_t)tmp;
                
                tmp = (float_t)((((((float_t)distance_max_mm-
			(float_t)distance_min_mm)/(float_t)10.0)+(float_t)30.02784)
			/((float_t)15.01392))+(float_t)0.5);
		p_motion_config->feature_length = (uint8_t)tmp;

		status |= vl53l5cx_dci_write_data(
			(p_motion_config),
			VL53L5CX_DCI_MOTION_DETECTOR_CFG,
                        (uint16_t)sizeof(*p_motion_config));
	}

    return Ok();
}

IResult<> VL53L5CX::motion_indicator_set_resolution(
		VL53L5CX_Motion_Configuration	*p_motion_config,
		uint8_t				resolution)
{
	uint8_t i, status = VL53L5CX_STATUS_OK;

	switch(resolution)
	{
		case VL53L5CX_RESOLUTION_4X4:
			for(i = 0; i < (uint8_t)VL53L5CX_RESOLUTION_4X4; i++)
			{
				p_motion_config->map_id[i] = (int8_t)i;
			}
		(void)memset(p_motion_config->map_id + 16, -1, 48);
		break;

		case VL53L5CX_RESOLUTION_8X8:
			for(i = 0; i < (uint8_t)VL53L5CX_RESOLUTION_8X8; i++)
			{
                               p_motion_config->map_id[i] = (int8_t)((((int8_t)
                               i % 8)/2) + (4*((int8_t)i/16)));
			}
		break;

		default:
			status |= VL53L5CX_STATUS_ERROR;
		break;
	}

	if(status != (uint8_t)0)
	{
		status |= vl53l5cx_dci_write_data(
				(p_motion_config),
				VL53L5CX_DCI_MOTION_DETECTOR_CFG, 
                                (uint16_t)sizeof(*p_motion_config));
	}

    return Ok();
}



IResult<> VL53L5CX::poll_for_answer(
		VL53L5CX_Configuration   *this,
		uint16_t 				address,
		uint8_t 				expected_value)
{
	uint8_t status = VL53L5CX_STATUS_OK;
	uint8_t timeout = 0;

	do {
		status |= read_burst(
                                  address, this->temp_buffer, 4);
		status |= WaitMs(10);
		
                /* 2s timeout or FW error*/
		if((timeout >= (uint8_t)200) 
                   || (this->temp_buffer[2] >= (uint8_t) 0x7f))
		{
			status |= VL53L5CX_MCU_ERROR;		
			break;
		}
                else
                {
                  timeout++;
                }
	}while ((this->temp_buffer[0x1]) != expected_value);
        
    return Ok();
}

/*
 * Inner function, not available outside this file. This function is used to
 * program the output using the macro defined into the 'platform.h' file.
 */

IResult<> VL53L5CX::program_output_config(
		VL53L5CX_Configuration 		 *this)
{
	uint8_t resolution, status = VL53L5CX_STATUS_OK;
	uint32_t i;
	uint64_t header_config;
	union Block_header *bh_ptr;

	status |= vl53l5cx_get_resolution(&resolution);
	this->data_read_size = 0;

	/* Enable mandatory output (meta and common data) */
	uint32_t output_bh_enable[] = {
			0x0001FFFFU,
			0x00000000U,
			0x00000000U,
			0xC0000000U};

	/* Send addresses of possible output */
	uint32_t output[] ={
			0x0000000DU,
			0x54000040U,
			0x9FD800C0U,
			0x9FE40140U,
			0x9FF80040U,
			0x9FFC0404U,
			0xA0FC0100U,
			0xA10C0100U,
			0xA11C00C0U,
			0xA1280902U,
			0xA2480040U,
			0xA24C0081U,
			0xA2540081U,
			0xA25C0081U,
			0xA2640081U,
			0xA26C0084U,
			0xA28C0082U};

	/* Update data size */
	for (i = 0; i < (uint32_t)(sizeof(output)/sizeof(uint32_t)); i++)
	{
		if ((output[i] == (uint8_t)0) 
                    || ((output_bh_enable[i/(uint32_t)32]
                         &((uint32_t)1 << (i%(uint32_t)32))) == (uint32_t)0))
		{
			continue;
		}

		bh_ptr = (union Block_header *)&(output[i]);
		if (((uint8_t)bh_ptr->type >= (uint8_t)0x1) 
                    && ((uint8_t)bh_ptr->type < (uint8_t)0x0d))
		{
			if ((bh_ptr->idx >= (uint16_t)0x54d0) 
                            && (bh_ptr->idx < (uint16_t)(0x54d0 + 960)))
			{
				bh_ptr->size = resolution;
			}	
			else 
			{
				bh_ptr->size = (uint8_t)(resolution 
                                  * (uint8_t)VL53L5CX_NB_TARGET_PER_ZONE);
			}

                        
			this->data_read_size += bh_ptr->type * bh_ptr->size;
		}
		else
		{
			this->data_read_size += bh_ptr->size;
		}

		this->data_read_size += (uint32_t)4;
	}
	this->data_read_size += (uint32_t)20;

	status |= vl53l5cx_dci_write_data(this,
			&(output), 
                        VL53L5CX_DCI_OUTPUT_LIST, (uint16_t)sizeof(output));
        
        header_config = (uint64_t)i + (uint64_t)1;
	header_config = header_config << 32;
	header_config += (uint64_t)this->data_read_size;

	status |= vl53l5cx_dci_write_data(&(header_config),
			VL53L5CX_DCI_OUTPUT_CONFIG, 
                        (uint16_t)sizeof(header_config));
	status |= vl53l5cx_dci_write_data(&(output_bh_enable),
			VL53L5CX_DCI_OUTPUT_ENABLES, 
                        (uint16_t)sizeof(output_bh_enable));

    return Ok();
}

IResult<> VL53L5CX::calibrate_xtalk(
		uint16_t			reflectance_percent,
		uint8_t				nb_samples,
		uint16_t			distance_mm)
{
	uint16_t timeout = 0;
	uint8_t cmd[] = {0x00, 0x03, 0x00, 0x00};
	uint8_t footer[] = {0x00, 0x00, 0x00, 0x0F, 0x00, 0x01, 0x03, 0x04};
	uint8_t continue_loop = 1, status = VL53L5CX_STATUS_OK;

	uint8_t resolution, frequency, target_order, sharp_prct, ranging_mode;
	uint32_t integration_time_ms, xtalk_margin;
        
	uint16_t reflectance = reflectance_percent;
	uint8_t	samples = nb_samples;
	uint16_t distance = distance_mm;

	/* Get initial configuration */
	status |= vl53l5cx_get_resolution(&resolution);
	status |= vl53l5cx_get_ranging_frequency_hz(&frequency);
	status |= vl53l5cx_get_integration_time_ms(&integration_time_ms);
	status |= vl53l5cx_get_sharpener_percent(&sharp_prct);
	status |= vl53l5cx_get_target_order(&target_order);
	status |= vl53l5cx_get_xtalk_margin(&xtalk_margin);
	status |= vl53l5cx_get_ranging_mode(&ranging_mode);

	/* Check input arguments validity */
	if(((reflectance < (uint16_t)1) || (reflectance > (uint16_t)99))
		|| ((distance < (uint16_t)600) || (distance > (uint16_t)3000))
		|| ((samples < (uint8_t)1) || (samples > (uint8_t)16)))
	{
		status |= VL53L5CX_STATUS_INVALID_PARAM;
	}
	else
	{
		status |= vl53l5cx_set_resolution(
				VL53L5CX_RESOLUTION_8X8);

		/* Send Xtalk calibration buffer */
                (void)memcpy(this->temp_buffer, VL53L5CX_CALIBRATE_XTALK, 
                       sizeof(VL53L5CX_CALIBRATE_XTALK));
		status |= write_burst(0x2c28,
				this->temp_buffer, 
                       (uint16_t)sizeof(VL53L5CX_CALIBRATE_XTALK));
		if(const auto res = poll_for_answer( res.is_err()) return Err(res.unwrap_err());
				VL53L5CX_UI_CMD_STATUS, 0x3);

		/* Format input argument */
		reflectance = reflectance * (uint16_t)16;
		distance = distance * (uint16_t)4;

		/* Update required fields */
		status |= vl53l5cx_dci_replace_data(this->temp_buffer,
				VL53L5CX_DCI_CAL_CFG, 8, 
                                &distance, 2, 0x00);

		status |= vl53l5cx_dci_replace_data(this->temp_buffer,
				VL53L5CX_DCI_CAL_CFG, 8,
                                &reflectance, 2, 0x02);

		status |= vl53l5cx_dci_replace_data(this->temp_buffer,
				VL53L5CX_DCI_CAL_CFG, 8, 
                                &samples, 1, 0x04);

		/* Program output for Xtalk calibration */
		status |= this->program_output_config(this);

		/* Start ranging session */
		status |= write_burst(&(this->platform),
				VL53L5CX_UI_CMD_END - (uint16_t)(4 - 1),
				cmd, sizeof(cmd));
		if(const auto res = poll_for_answer( res.is_err()) return Err(res.unwrap_err());
				VL53L5CX_UI_CMD_STATUS, 0x3);

		/* Wait for end of calibration */
		do {
			status |= read_burst(
                                          0x0, this->temp_buffer, 4);
			if(this->temp_buffer[0] != VL53L5CX_STATUS_ERROR)
			{
				/* Coverglass too good for Xtalk calibration */
				if((this->temp_buffer[2] >= (uint8_t)0x7f) &&
				(((uint16_t)(this->temp_buffer[3] & 
                                 (uint16_t)0x80) >> 7) == (uint16_t)1))
				{
					(void)memcpy(this->xtalk_data, 
                                               this->default_xtalk,
                                               VL53L5CX_XTALK_BUFFER_SIZE);
				}
				continue_loop = (uint8_t)0;
			}
			else if(timeout >= (uint16_t)400)
			{
				status |= VL53L5CX_STATUS_ERROR;
				continue_loop = (uint8_t)0;
			}
			else
			{
				timeout++;
				status |= WaitMs(50);
			}

		}while (continue_loop == (uint8_t)1);
	}

	/* Save Xtalk data into the Xtalk buffer */
        (void)memcpy(this->temp_buffer, VL53L5CX_GET_XTALK_CMD, 
               sizeof(VL53L5CX_GET_XTALK_CMD));
	status |= write_burst(0x2fb8,
			this->temp_buffer, 
                        (uint16_t)sizeof(VL53L5CX_GET_XTALK_CMD));
	if(const auto res = poll_for_answer(this,VL53L5CX_UI_CMD_STATUS, 0x03); res.is_err()) return Err(res.unwrap_err());
	status |= read_burst(VL53L5CX_UI_CMD_START,
			this->temp_buffer, 
                        VL53L5CX_XTALK_BUFFER_SIZE + (uint16_t)4);

	(void)memcpy(&(this->xtalk_data[0]), &(this->temp_buffer[8]),
			VL53L5CX_XTALK_BUFFER_SIZE - (uint16_t)8);
	(void)memcpy(&(this->xtalk_data[VL53L5CX_XTALK_BUFFER_SIZE 
                       - (uint16_t)8]), footer, sizeof(footer));

	/* Reset default buffer */
	if(const auto res = write_burst(0x2c34,
			this->default_configuration,
			VL53L5CX_CONFIGURATION_SIZE);
        res.is_err()) return Err(res.unwrap_err());
    
	if(const auto res = this->poll_for_answer(VL53L5CX_UI_CMD_STATUS, 0x03);
        res.is_err()) return Err(res.unwrap_err());

	/* Reset initial configuration */
	if(const auto res = set_resolution(resolution); 
        res.is_err()) return Err(res.unwrap_err());
	if(const auto res = set_ranging_frequency_hz(frequency); 
        res.is_err()) return Err(res.unwrap_err());
	if(const auto res = set_integration_time_ms(integration_time_ms); 
        res.is_err()) return Err(res.unwrap_err());
	if(const auto res = set_sharpener_percent(sharp_prct); 
        res.is_err()) return Err(res.unwrap_err());
	if(const auto res = set_target_order(target_order); 
        res.is_err()) return Err(res.unwrap_err());
	if(const auto res = set_xtalk_margin(xtalk_margin); 
        res.is_err()) return Err(res.unwrap_err());
	if(const auto res = set_ranging_mode(ranging_mode); 
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> VL53L5CX::get_caldata_xtalk(
		uint8_t				*p_xtalk_data)
{
	resolution;
	uint8_t footer[] = {0x00, 0x00, 0x00, 0x0F, 0x00, 0x01, 0x03, 0x04};

	status |= vl53l5cx_get_resolution(&resolution);
	status |= vl53l5cx_set_resolution(VL53L5CX_RESOLUTION_8X8);

        (void)memcpy(this->temp_buffer, VL53L5CX_GET_XTALK_CMD, 
               sizeof(VL53L5CX_GET_XTALK_CMD));
	status |= write_burst(0x2fb8,
			this->temp_buffer,  sizeof(VL53L5CX_GET_XTALK_CMD));
	if(const auto res = poll_for_answer(this,VL53L5CX_UI_CMD_STATUS, 0x03); res.is_err()) return Err(res.unwrap_err());
	status |= read_burst(VL53L5CX_UI_CMD_START,
			this->temp_buffer, 
                        VL53L5CX_XTALK_BUFFER_SIZE + (uint16_t)4);

	(void)memcpy(&(p_xtalk_data[0]), &(this->temp_buffer[8]),
			VL53L5CX_XTALK_BUFFER_SIZE-(uint16_t)8);
	(void)memcpy(&(p_xtalk_data[VL53L5CX_XTALK_BUFFER_SIZE - (uint16_t)8]),
			footer, sizeof(footer));

	status |= vl53l5cx_set_resolution(resolution);

    return Ok();
}

IResult<> VL53L5CX::set_caldata_xtalk(
		uint8_t				*p_xtalk_data)
{
	uint8_t resolution, status = VL53L5CX_STATUS_OK;

	status |= vl53l5cx_get_resolution(&resolution);
	(void)memcpy(this->xtalk_data, p_xtalk_data, VL53L5CX_XTALK_BUFFER_SIZE);
	status |= vl53l5cx_set_resolution(resolution);

    return Ok();
}

IResult<> VL53L5CX::get_xtalk_margin(
		uint32_t			*p_xtalk_margin)
{
	uint8_t status = VL53L5CX_STATUS_OK;

	status |= vl53l5cx_dci_read_data(this->temp_buffer,
			VL53L5CX_DCI_XTALK_CFG, 16);

	(void)memcpy(p_xtalk_margin, this->temp_buffer, 4);
	*p_xtalk_margin = *p_xtalk_margin/(uint32_t)2048;

    return Ok();
}

IResult<> VL53L5CX::set_xtalk_margin(
		uint32_t			xtalk_margin)
{
	uint8_t status = VL53L5CX_STATUS_OK;
        uint32_t margin_kcps = xtalk_margin;

	if(margin_kcps > (uint32_t)10000)
	{
		status |= VL53L5CX_STATUS_INVALID_PARAM;
	}
	else
	{
		margin_kcps = margin_kcps*(uint32_t)2048;
		status |= vl53l5cx_dci_replace_data(this->temp_buffer,
				VL53L5CX_DCI_XTALK_CFG, 16, 
                                &margin_kcps, 4, 0x00);
	}

    return Ok();
}

#endif