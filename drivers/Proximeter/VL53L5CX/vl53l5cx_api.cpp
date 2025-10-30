#include "vl53l5cx.hpp"
#include "details/vl53l5cx_buffers.hpp"
#include "details/vl53l5cx_firmware.ipp"

using namespace ymd;
using namespace ymd::drivers;

using namespace ymd::drivers::vl53l5cx_details;

using Self = VL53L5CX;
using Error = Self::Error;

template<typename T = void>
using IResult = Result<T, Error>;

static constexpr uint8_t VL53L5CX_STATUS_ERROR = 255U;
static constexpr uint16_t VL53L5CX_DCI_CAL_CFG				= ((uint16_t)0x5470U);
static constexpr uint16_t VL53L5CX_DCI_XTALK_CFG				= ((uint16_t)0xAD94U);

// /* Address of default configuration buffer */
// const uint8_t		        *VL53L5CX_DEFAULT_CONFIGURATION = 
// 	vl53l5cx_details::VL53L5CX_DEFAULT_CONFIGURATION;
// /* Address of default Xtalk buffer */
// const uint8_t		        *VL53L5CX_DEFAULT_XTALK = 
// 	vl53l5cx_details::VL53L5CX_DEFAULT_XTALK;

template<typename T>
requires std::is_pointer_v<T>
static T ptr_cast(auto * obj){
	return reinterpret_cast<T>(obj);
}

template<typename E, typename T>
requires (std::is_integral_v<T>)
static constexpr IResult<E> try_into_enum(T obj){
	return Ok(std::bit_cast<E>(obj));
}

static const uint8_t VL53L5CX_GET_XTALK_CMD[] = {
	0x54, 0x00, 0x00, 0x40,
	0x9F, 0xD8, 0x00, 0xC0,
	0x9F, 0xE4, 0x01, 0x40,
	0x9F, 0xF8, 0x00, 0x40,
	0x9F, 0xFC, 0x04, 0x04,
	0xA0, 0xFC, 0x01, 0x00,
	0xA1, 0x0C, 0x01, 0x00,
	0xA1, 0x1C, 0x00, 0xC0,
	0xA1, 0x28, 0x09, 0x02,
	0xA2, 0x48, 0x00, 0x40,
	0xA2, 0x4C, 0x00, 0x81,
	0xA2, 0x54, 0x00, 0x81,
	0xA2, 0x5C, 0x00, 0x81,
	0xA2, 0x64, 0x00, 0x81,
	0xA2, 0x6C, 0x00, 0x84,
	0xA2, 0x8C, 0x00, 0x82,
	0x00, 0x00, 0x00, 0x0F,
	0x07, 0x02, 0x00, 0x44
};

/**
 * @brief Command used to get run Xtalk calibration
 */

static const uint8_t VL53L5CX_CALIBRATE_XTALK[] = {
	0x54, 0x50, 0x00, 0x80,
	0x00, 0x04, 0x08, 0x08,
	0x00, 0x00, 0x04, 0x04,
	0xAD, 0x30, 0x00, 0x80,
	0x03, 0x01, 0x06, 0x03,
	0x00, 0x00, 0x01, 0x00,
	0xAD, 0x38, 0x01, 0x00,
	0x01, 0xE0, 0x01, 0x40,
	0x00, 0x10, 0x00, 0x10,
	0x01, 0x00, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x01,
	0x54, 0x58, 0x00, 0x40,
	0x04, 0x1A, 0x02, 0x00,
	0x54, 0x5C, 0x01, 0x40,
	0x00, 0x01, 0x00, 0x51,
	0x00, 0x00, 0x0F, 0xA0,
	0x0F, 0xA0, 0x03, 0xE8,
	0x02, 0x80, 0x1F, 0x40,
	0x00, 0x00, 0x05, 0x00,
	0x54, 0x70, 0x00, 0x80,
	0x03, 0x20, 0x03, 0x20,
	0x00, 0x00, 0x00, 0x08,
	0x54, 0x78, 0x01, 0x00,
	0x01, 0x1B, 0x00, 0x21,
	0x00, 0x33, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x01,
	0x04, 0x01, 0x08, VL53L5CX_Prelude::VL53L5CX_FW_NBTAR_XTALK,
	0x54, 0x88, 0x01, 0x40,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x08, 0x00,
	0xAD, 0x48, 0x01, 0x00,
	0x01, 0xF4, 0x00, 0x00,
	0x03, 0x06, 0x00, 0x10,
	0x08, 0x08, 0x08, 0x08,
	0x00, 0x00, 0x00, 0x08,
	0xAD, 0x60, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x80,
	0x00, 0x00, 0x00, 0x00,
	0x20, 0x1F, 0x01, 0xF4,
	0x00, 0x00, 0x1D, 0x0A,
	0xAD, 0x70, 0x00, 0x80,
	0x08, 0x00, 0x1F, 0x40,
	0x00, 0x00, 0x00, 0x01,
	0xAD, 0x78, 0x00, 0x80,
	0x00, 0xA0, 0x03, 0x20,
	0x00, 0x01, 0x01, 0x90,
	0xAD, 0x80, 0x00, 0x40,
	0x00, 0x00, 0x28, 0x00,
	0xAD, 0x84, 0x00, 0x80,
	0x00, 0x00, 0x32, 0x00,
	0x03, 0x20, 0x00, 0x00,
	0xAD, 0x8C, 0x00, 0x80,
	0x02, 0x58, 0xFF, 0x38,
	0x00, 0x00, 0x00, 0x0C,
	0xAD, 0x94, 0x01, 0x00,
	0x00, 0x01, 0x90, 0x00,
	0xFF, 0xFF, 0xFC, 0x00,
	0x00, 0x00, 0x04, 0x00,
	0x00, 0x00, 0x01, 0x00,
	0xAD, 0xA4, 0x00, 0xC0,
	0x04, 0x80, 0x06, 0x1A,
	0x00, 0x80, 0x05, 0x80,
	0x00, 0x00, 0x01, 0x06,
	0xAD, 0xB0, 0x00, 0xC0,
	0x04, 0x80, 0x06, 0x1A,
	0x19, 0x00, 0x05, 0x80,
	0x00, 0x00, 0x01, 0x90,
	0xAD, 0xBC, 0x04, 0x40,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x12, 0x00, 0x25,
	0x00, 0x00, 0x00, 0x06,
	0x00, 0x00, 0x00, 0x05,
	0x00, 0x00, 0x00, 0x05,
	0x00, 0x00, 0x00, 0x06,
	0x00, 0x00, 0x00, 0x04,
	0x00, 0x00, 0x00, 0x0F,
	0x00, 0x00, 0x00, 0x5A,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x09,
	0x0B, 0x0C, 0x0B, 0x0B,
	0x03, 0x03, 0x11, 0x05,
	0x01, 0x01, 0x01, 0x01,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x0D, 0x00, 0x00,
	0xAE, 0x00, 0x01, 0x04,
	0x00, 0x00, 0x00, 0x04,
	0x00, 0x00, 0x00, 0x08,
	0x00, 0x00, 0x00, 0x0A,
	0x00, 0x00, 0x00, 0x0C,
	0x00, 0x00, 0x00, 0x0D,
	0x00, 0x00, 0x00, 0x0E,
	0x00, 0x00, 0x00, 0x08,
	0x00, 0x00, 0x00, 0x08,
	0x00, 0x00, 0x00, 0x10,
	0x00, 0x00, 0x00, 0x10,
	0x00, 0x00, 0x00, 0x20,
	0x00, 0x00, 0x00, 0x20,
	0x00, 0x00, 0x00, 0x06,
	0x00, 0x00, 0x05, 0x0A,
	0x02, 0x00, 0x0C, 0x08,
	0x00, 0x00, 0x00, 0x00,
	0xAE, 0x40, 0x00, 0x40,
	0x00, 0x00, 0x00, 0xFF,
	0xAE, 0x44, 0x00, 0x40,
	0x00, 0x10, 0x04, 0x01,
	0xAE, 0x48, 0x00, 0x40,
	0x00, 0x00, 0x10, 0x00,
	0xAE, 0x4C, 0x00, 0x40,
	0x00, 0x00, 0x00, 0x01,
	0xAE, 0x50, 0x01, 0x40,
	0x00, 0x00, 0x00, 0x14,
	0x04, 0x00, 0x28, 0x00,
	0x03, 0x20, 0x6C, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0xAE, 0x64, 0x00, 0x40,
	0x00, 0x00, 0x00, 0x01,
	0xAE, 0xD8, 0x01, 0x00,
	0x00, 0xC8, 0x05, 0xDC,
	0x00, 0x00, 0x0C, 0xCD,
	0x01, 0x04, 0x00, 0x00,
	0x00, 0x01, 0x26, 0x01,
	0xB5, 0x50, 0x02, 0x82,
	0xA3, 0xE8, 0xA3, 0xB8,
	0xA4, 0x38, 0xA4, 0x28,
	0xA6, 0x48, 0xA4, 0x48,
	0xA7, 0x88, 0xA7, 0x48,
	0xAC, 0x10, 0xA7, 0x90,
	0x99, 0xBC, 0x99, 0xB4,
	0x9A, 0xFC, 0x9A, 0xBC,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0xB5, 0xA0, 0x02, 0x82,
	0x00, 0x88, 0x03, 0x00,
	0x00, 0x82, 0x00, 0x82,
	0x04, 0x04, 0x04, 0x08,
	0x00, 0x80, 0x04, 0x01,
	0x09, 0x02, 0x09, 0x08,
	0x04, 0x04, 0x00, 0x80,
	0x04, 0x01, 0x04, 0x01,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0xB5, 0xF0, 0x00, 0x40,
	0x00, 0x04, 0x00, 0x00,
	0xB3, 0x9C, 0x01, 0x00,
	0x40, 0x00, 0x05, 0x1E,
	0x02, 0x1B, 0x08, 0x7C,
	0x80, 0x01, 0x12, 0x01,
	0x00, 0x00, 0x08, 0x00,
	0xB6, 0xC0, 0x00, 0xC0,
	0x00, 0x00, 0x60, 0x00,
	0x00, 0x00, 0x20, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0xAE, 0xA8, 0x00, 0x40,
	0x00, 0x00, 0x04, 0x05,
	0xAE, 0xAC, 0x00, 0x80,
	0x01, 0x00, 0x01, 0x00,
	0x00, 0x02, 0x00, 0x00,
	0xAE, 0xB4, 0x00, 0x40,
	0x00, 0x00, 0x00, 0x00,
	0xAE, 0xB8, 0x00, 0x81,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0xAE, 0xC0, 0x00, 0x81,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0xAE, 0xC8, 0x00, 0x81,
	0x08, 0x01, 0x01, 0x08,
	0x00, 0x00, 0x00, 0x08,
	0xAE, 0xD0, 0x00, 0x81,
	0x01, 0x08, 0x08, 0x08,
	0x00, 0x00, 0x00, 0x01,
	0xB5, 0xF4, 0x00, 0x80,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0xB5, 0xFC, 0x00, 0x80,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0xB6, 0x04, 0x00, 0x40,
	0x00, 0x00, 0x00, 0x00,
	0xB6, 0x08, 0x00, 0x44,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0xB6, 0x18, 0x00, 0x44,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0xB6, 0x28, 0x00, 0x44,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0xB6, 0x38, 0x00, 0x44,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0xB6, 0x48, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0xB6, 0x58, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0xB6, 0x68, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x54, 0x70, 0x00, 0x80,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x02,
	0x00, 0x00, 0x00, 0x0F,
	0x00, 0x01, 0x03, 0xD4
};



void SwapBuffer(uint8_t *buffer, uint16_t size)
{
    uint32_t tmp;
    /* Example of possible implementation using <string.h> */
    for(uint32_t i = 0; i < size; i = i + 4) {
        tmp = (
                buffer[i]<<24)
            |(buffer[i+1]<<16)
            |(buffer[i+2]<<8)
            |(buffer[i+3]);

        memcpy(&(buffer[i]), &tmp, 4);
    }
}

#if 1

IResult<> Self::init(){
	uint8_t tmp;
	static constexpr uint8_t pipe_ctrl[] = {VL53L5CX_NB_TARGET_PER_ZONE, 0x00, 0x01, 0x00};
	uint32_t single_range = 0x01;

	if(const auto res = validate();
		res.is_err()) return Err(res.unwrap_err());
	/* SW reboot sequence */
	if(const auto res = write_byte(0x7fff, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x0009, 0x04); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x000F, 0x40); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x000A, 0x03); 
		res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_byte(0x7FFF, &tmp); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x000C, 0x01); 
		res.is_err()) return Err(res.unwrap_err());

	if(const auto res = write_byte(0x0101, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x0102, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x010A, 0x01); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x4002, 0x01); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x4002, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x010A, 0x03); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x0103, 0x01); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x000C, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x000F, 0x43); 
		res.is_err()) return Err(res.unwrap_err());
    clock::delay(1ms);

	if(const auto res = write_byte(0x000F, 0x40); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x000A, 0x01); 
		res.is_err()) return Err(res.unwrap_err());
    clock::delay(100ms);

	/* Wait for sensor booted (several ms required to get sensor ready ) */
	if(const auto res = write_byte(0x7fff, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = poll_for_answer(1, 0, 0x06, 0xff, 1); 
		res.is_err()) return Err(res.unwrap_err());

	if(const auto res = write_byte(0x000E, 0x01); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x02); 
		res.is_err()) return Err(res.unwrap_err());

	/* Enable FW access */
	if(const auto res = write_byte(0x03, 0x0D); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x01); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = poll_for_answer(1, 0, 0x21, 0x10, 0x10); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x00); 
		res.is_err()) return Err(res.unwrap_err());

	/* Enable host access to GO1 */
	if(const auto res = read_byte(0x7fff, &tmp); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x0C, 0x01); 
		res.is_err()) return Err(res.unwrap_err());

	/* Power ON status */
	if(const auto res = write_byte(0x7fff, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x101, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x102, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x010A, 0x01); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x4002, 0x01); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x4002, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x010A, 0x03); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x103, 0x01); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x400F, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x21A, 0x43); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x21A, 0x03); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x21A, 0x01); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x21A, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x219, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x21B, 0x00); 
		res.is_err()) return Err(res.unwrap_err());

	/* Wake up MCU */
	if(const auto res = write_byte(0x7fff, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_byte(0x7fff, &tmp); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x0C, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x01); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x20, 0x07); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x20, 0x06); 
		res.is_err()) return Err(res.unwrap_err());

	/* Download FW into VL53L5 */
	if(const auto res = write_byte(0x7fff, 0x09); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_burst(0, &VL53L5CX_FIRMWARE[0],0x8000); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x0a); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_burst(0, &VL53L5CX_FIRMWARE[0x8000],0x8000); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x0b); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_burst(0, &VL53L5CX_FIRMWARE[0x10000],0x5000); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x01); 
		res.is_err()) return Err(res.unwrap_err());

	/* Check if FW correctly downloaded */
	if(const auto res = write_byte(0x7fff, 0x02); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x03, 0x0D); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x01); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = poll_for_answer(1, 0, 0x21, 0x10, 0x10);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_byte(0x7fff, &tmp); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x0C, 0x01); 
		res.is_err()) return Err(res.unwrap_err());

	/* Reset MCU and wait boot */
	if(const auto res = write_byte(0x7FFF, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x114, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x115, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x116, 0x42); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x117, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x0B, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_byte(0x7fff, &tmp); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x0C, 0x00); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x0B, 0x01); 
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = poll_for_mcu_boot();
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x02); 
		res.is_err()) return Err(res.unwrap_err());

	/* Get offset NVM data and store them into the offset buffer */
	if(const auto res = write_burst( 0x2fd8,
		VL53L5CX_GET_NVM_CMD, sizeof(VL53L5CX_GET_NVM_CMD));
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = poll_for_answer(4, 0, 
		VL53L5CX_UI_CMD_STATUS, 0xff, 2);
		res.is_err()) return Err(res.unwrap_err());

	if(const auto res = read_burst( VL53L5CX_UI_CMD_START,
		temp_buffer, VL53L5CX_NVM_DATA_SIZE);
		res.is_err()) return Err(res.unwrap_err());

	(void)memcpy(offset_data, temp_buffer,
		VL53L5CX_OFFSET_BUFFER_SIZE);
	if(const auto res = send_offset_data(Resolution::_4x4);
		res.is_err()) return Err(res.unwrap_err());

	/* Set default Xtalk shape. Send Xtalk to sensor */
	(void)memcpy(xtalk_data, VL53L5CX_DEFAULT_XTALK,
		VL53L5CX_XTALK_BUFFER_SIZE);
	if(const auto res = send_xtalk_data(Resolution::_4x4);
		res.is_err()) return Err(res.unwrap_err());

	/* Send default configuration to Self firmware */
	if(const auto res = write_burst( 0x2c34,
		VL53L5CX_DEFAULT_CONFIGURATION,
		sizeof(VL53L5CX_DEFAULT_CONFIGURATION));
		res.is_err()) return Err(res.unwrap_err());

	if(const auto res = poll_for_answer(4, 1, 
		VL53L5CX_UI_CMD_STATUS, 0xff, 0x03);
		res.is_err()) return Err(res.unwrap_err());

	if(const auto res = dci_write_data(pipe_ctrl,
		VL53L5CX_DCI_PIPE_CONTROL, (uint16_t)sizeof(pipe_ctrl));
		res.is_err()) return Err(res.unwrap_err());

	if constexpr(VL53L5CX_NB_TARGET_PER_ZONE != 1){
		tmp = VL53L5CX_NB_TARGET_PER_ZONE;
		if(const auto res = dci_replace_data(temp_buffer,
			VL53L5CX_DCI_FW_NB_TARGET, 16,
			&tmp, 1, 0x0C);
			res.is_err()) return Err(res.unwrap_err());
	}
	if(const auto res = dci_write_data(
		ptr_cast<const uint8_t *>(&single_range),
		VL53L5CX_DCI_SINGLE_RANGE,
		(uint16_t)sizeof(single_range));
		res.is_err()) return Err(res.unwrap_err());

    return Ok();
}



/*
 * Inner function, not available outside this file. This function is used to
 * wait for the MCU to boot.
 */


IResult<> Self::poll_for_mcu_boot()
{
   uint8_t go2_status0, go2_status1;
   uint16_t timeout = 0;

   do {
		if(const auto res = read_byte(0x06, &go2_status0);
			res.is_err()) return Err(res.unwrap_err());
		if((go2_status0 & (uint8_t)0x80) != (uint8_t)0){
			if(const auto res = read_byte(0x07, &go2_status1);
				res.is_err()) return Err(res.unwrap_err());
			return Err(map_status_to_error(go2_status1));
		}
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

IResult<> Self::send_offset_data(Resolution resolution)
{
	uint32_t signal_grid[64];
	int16_t range_grid[64];
	static constexpr uint8_t dss_4x4[] = {0x0F, 0x04, 0x04, 0x00, 0x08, 0x10, 0x10, 0x07};
	static constexpr uint8_t footer[] = {0x00, 0x00, 0x00, 0x0F, 0x03, 0x01, 0x01, 0xE4};
	uint16_t k;

	(void)memcpy(temp_buffer,
		offset_data, VL53L5CX_OFFSET_BUFFER_SIZE);

	/* Data extrapolation is required for 4X4 offset */
	if(resolution == Resolution::_4x4){
		(void)memcpy(&(temp_buffer[0x10]), dss_4x4, sizeof(dss_4x4));
		SwapBuffer(temp_buffer, VL53L5CX_OFFSET_BUFFER_SIZE);
		(void)memcpy(signal_grid,&(temp_buffer[0x3C]),
			sizeof(signal_grid));
		(void)memcpy(range_grid,&(temp_buffer[0x140]),
			sizeof(range_grid));

		for (size_t j = 0; j < 4; j++)
		{
			for (size_t i = 0; i < 4 ; i++)
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
            (void)memcpy(&(temp_buffer[0x3C]),
		signal_grid, sizeof(signal_grid));
            (void)memcpy(&(temp_buffer[0x140]),
		range_grid, sizeof(range_grid));
            SwapBuffer(temp_buffer, VL53L5CX_OFFSET_BUFFER_SIZE);
	}

	for(k = 0; k < (VL53L5CX_OFFSET_BUFFER_SIZE - (uint16_t)4); k++)
	{
		temp_buffer[k] = temp_buffer[k + (uint16_t)8];
	}

	(void)memcpy(&(temp_buffer[0x1E0]), footer, 8);
	if(const auto res = write_burst(0x2e18, temp_buffer,
		VL53L5CX_OFFSET_BUFFER_SIZE);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = poll_for_answer(4, 1,
		VL53L5CX_UI_CMD_STATUS, 0xff, 0x03);
		res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

/**
 * @brief Inner function, not available outside this file. This function is used
 * to set the Xtalk data from generic configuration, or user's calibration.
 */

IResult<> Self::send_xtalk_data(Resolution resolution){
	static constexpr uint8_t res4x4[] = {0x0F, 0x04, 0x04, 0x17, 0x08, 0x10, 0x10, 0x07};
	static constexpr uint8_t dss_4x4[] = {0x00, 0x78, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08};
	static constexpr uint8_t profile_4x4[] = {0xA0, 0xFC, 0x01, 0x00};
	uint32_t signal_grid[64];

	(void)memcpy(temp_buffer, &(xtalk_data[0]),
		VL53L5CX_XTALK_BUFFER_SIZE);

	/* Data extrapolation is required for 4X4 Xtalk */
	if(resolution == Resolution::_4x4)
	{
		(void)memcpy(&(temp_buffer[0x8]),
			res4x4, sizeof(res4x4));
		(void)memcpy(&(temp_buffer[0x020]),
			dss_4x4, sizeof(dss_4x4));

		SwapBuffer(temp_buffer, VL53L5CX_XTALK_BUFFER_SIZE);
		(void)memcpy(signal_grid, &(temp_buffer[0x34]),
			sizeof(signal_grid));

		for (size_t j = 0; j < 4; j++)
		{
			for (size_t i = 0; i < 4 ; i++)
			{
				signal_grid[i+(4*j)] =
				(signal_grid[(2*i)+(16*j)+0]
				+ signal_grid[(2*i)+(16*j)+1]
				+ signal_grid[(2*i)+(16*j)+8]
				+ signal_grid[(2*i)+(16*j)+9])/(uint32_t)4;
			}
		}
		(void)memset(&signal_grid[0x10], 0, (uint32_t)192);
		(void)memcpy(&(temp_buffer[0x34]),
			signal_grid, sizeof(signal_grid));
		SwapBuffer(temp_buffer, VL53L5CX_XTALK_BUFFER_SIZE);
		(void)memcpy(&(temp_buffer[0x134]),
		profile_4x4, sizeof(profile_4x4));
		(void)memset(&(temp_buffer[0x078]),0 ,
			(uint32_t)4*sizeof(uint8_t));
	}

	if(const auto res = write_burst(0x2cf8,
		temp_buffer, VL53L5CX_XTALK_BUFFER_SIZE);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = poll_for_answer(4, 1,
		VL53L5CX_UI_CMD_STATUS, 0xff, 0x03);
		res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> Self::validate(){
	uint8_t device_id, revision_id;

	static constexpr uint8_t CORRECT_DEVICE_ID = 0xf0;
	static constexpr uint8_t CORRECT_REVISION_ID = 0x02;

	if(const auto res = write_byte(0x7fff, 0x00);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = read_byte(0, &device_id);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = read_byte(1, &revision_id);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x02);
		res.is_err()) return Err(res.unwrap_err());

	if(device_id != CORRECT_DEVICE_ID)
		return Err(Error::InvalidDeviceId);

	if(revision_id != CORRECT_REVISION_ID)
		return Err(Error::InvalidRevisionId);
    return Ok();
}


IResult<> Self::set_i2c_address(hal::I2cSlaveAddr<7> i2c_addr){
	if(const auto res = write_byte(0x7fff, 0x00);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x4, i2c_addr.as_u7());
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x02);
		res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<Self::PowerMode> Self::get_power_mode(){
	uint8_t tmp;

	if(const auto res = write_byte(0x7FFF, 0x00);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = read_byte(0x009, &tmp);
		res.is_err()) return Err(res.unwrap_err());

	PowerMode power_mode;

	switch(tmp){
	case 0x4:
		power_mode = PowerMode::WakeUp;
		break;
	case 0x2:
		power_mode = PowerMode::Sleep;

		break;
	default:
		return Err(Error::Status);
		break;
	}

	if(const auto res = write_byte(0x7FFF, 0x02);
		res.is_err()) return Err(res.unwrap_err());

    return Ok(power_mode);
}

IResult<> Self::set_power_mode(PowerMode power_mode)
{
	const PowerMode current_power_mode = ({
		const auto res = get_power_mode();
		if(res.is_err()) return Err(res.unwrap_err());
		res.unwrap();
	});

	if(power_mode != current_power_mode){
		switch(power_mode){
			case PowerMode::WakeUp:
				if(const auto res = write_byte(0x7FFF, 0x00);
					res.is_err()) return Err(res.unwrap_err());
				if(const auto res = write_byte(0x09, 0x04);
					res.is_err()) return Err(res.unwrap_err());
				if(const auto res = poll_for_answer(1, 0, 0x06, 0x01, 1);
					res.is_err()) return Err(res.unwrap_err());
				break;

			case PowerMode::Sleep:
				if(const auto res = write_byte(0x7FFF, 0x00);
					res.is_err()) return Err(res.unwrap_err());
				if(const auto res = write_byte(0x09, 0x02);
					res.is_err()) return Err(res.unwrap_err());
				if(const auto res = poll_for_answer(1, 0, 0x06, 0x01, 0); 
					res.is_err()) return Err(res.unwrap_err());
				break;

			default:
				return Err(Error::Status);
				break;
		}
		if(const auto res = write_byte(0x7FFF, 0x02);
			res.is_err()) return Err(res.unwrap_err());
	}

    return Ok();
}

IResult<> Self::start_ranging()
{
	uint16_t tmp;
	uint32_t header_config[2] = {0, 0};

	uint8_t cmd[] = {0x00, 0x03, 0x00, 0x00};

	const auto resolution = ({
		const auto res = get_resolution();
		if(res.is_err()) return Err(res.unwrap_err());
		res.unwrap();
	});
	

	stream_count_ = 255;

	/* Enable mandatory output (meta and common data) */
	static constexpr uint32_t output_bh_enable[] = {
		0x00000007U
		#ifndef VL53L5CX_DISABLE_AMBIENT_PER_SPAD
			| (uint32_t)8
		#endif
		#ifndef VL53L5CX_DISABLE_NB_SPADS_ENABLED
			| (uint32_t)16
		#endif
		#ifndef VL53L5CX_DISABLE_NB_TARGET_DETECTED
			| (uint32_t)32
		#endif
		#ifndef VL53L5CX_DISABLE_SIGNAL_PER_SPAD
			| (uint32_t)64
		#endif
		#ifndef VL53L5CX_DISABLE_RANGE_SIGMA_MM
			| (uint32_t)128
		#endif
		#ifndef VL53L5CX_DISABLE_DISTANCE_MM
			| (uint32_t)256
		#endif
		#ifndef VL53L5CX_DISABLE_REFLECTANCE_PERCENT
			| (uint32_t)512
		#endif
		#ifndef VL53L5CX_DISABLE_TARGET_STATUS
			| (uint32_t)1024
		#endif
		#ifndef VL53L5CX_DISABLE_MOTION_INDICATOR
			| (uint32_t)2048
		#endif
		
		,
		0x00000000U,
		0x00000000U,
		0xC0000000U
	};

	/* Send addresses of possible output */
	static constexpr uint32_t output[] ={
		VL53L5CX_START_BH,
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

	data_read_size_ = 0;
	/* Update data size */
	for (uint32_t i = 0; i < (uint32_t)(sizeof(output)/sizeof(uint32_t)); i++)
	{
		if ((output[i] == (uint8_t)0) 
			|| ((output_bh_enable[i/(uint32_t)32]
			&((uint32_t)1 << (i%(uint32_t)32))) == (uint32_t)0))
		{
			continue;
		}

		auto bh = std::bit_cast<Block_header>(output[i]);
		if (((uint8_t)bh.type >= (uint8_t)0x1) 
                    && ((uint8_t)bh.type < (uint8_t)0x0d))
		{
			if ((bh.idx >= (uint16_t)0x54d0) 
                            && (bh.idx < (uint16_t)(0x54d0 + 960)))
			{
				bh.size = static_cast<uint8_t>(resolution);
			}
			else
			{
				bh.size = (uint8_t)(static_cast<uint8_t>(resolution) * (uint8_t)VL53L5CX_NB_TARGET_PER_ZONE);
			}
			data_read_size_ += bh.type * bh.size;
		}
		else
		{
			data_read_size_ += bh.size;
		}
		data_read_size_ += (uint32_t)4;
	}
	data_read_size_ += (uint32_t)20;

	if(const auto res = dci_write_data(
		ptr_cast<const uint8_t *>(&(output)), VL53L5CX_DCI_OUTPUT_LIST,
		(uint16_t)sizeof(output));
		res.is_err()) return Err(res.unwrap_err());

	header_config[0] = data_read_size_;
	header_config[1] = (uint32_t)(sizeof(output)/sizeof(uint32_t)) + (uint32_t)1;

	if(const auto res = dci_write_data(
		ptr_cast<const uint8_t *>(&(header_config)), VL53L5CX_DCI_OUTPUT_CONFIG,
		(uint16_t)sizeof(header_config));
		res.is_err()) return Err(res.unwrap_err());

	if(const auto res = dci_write_data(
		ptr_cast<const uint8_t *>(&(output_bh_enable)), VL53L5CX_DCI_OUTPUT_ENABLES,
		(uint16_t)sizeof(output_bh_enable));
		res.is_err()) return Err(res.unwrap_err());

	/* Start xshut bypass (interrupt mode) */
	if(const auto res = write_byte(0x7fff, 0x00);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x09, 0x05);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x02);
		res.is_err()) return Err(res.unwrap_err());

	/* Start ranging session */
	if(const auto res = write_burst(
		VL53L5CX_UI_CMD_END - (uint16_t)(4 - 1), 
		cmd, sizeof(cmd));
		res.is_err()) return Err(res.unwrap_err());

	if(const auto res = poll_for_answer(4, 1, 
		VL53L5CX_UI_CMD_STATUS, 0xff, 0x03);
		res.is_err()) return Err(res.unwrap_err());

	/* Read ui range data content and compare if data size is the correct one */
	if(const auto res = dci_read_data(
		temp_buffer, 0x5440, 12);
		res.is_err()) return Err(res.unwrap_err());

	(void)memcpy(&tmp, &(temp_buffer[0x8]), sizeof(tmp));

	if(tmp != data_read_size_){
		return Err(Error::Status);
	}

    return Ok();
}


IResult<> Self::stop_ranging()
{
	uint8_t tmp = 0;
	uint16_t timeout = 0;
	uint32_t auto_stop_flag = 0;

	if(const auto res = read_burst(
		0x2FFC, ptr_cast<uint8_t *>(&auto_stop_flag), 4);
		res.is_err()) return Err(res.unwrap_err());

	if(auto_stop_flag != (uint32_t)0x4FF){
		if(const auto res = write_byte(0x7fff, 0x00);
			res.is_err()) return Err(res.unwrap_err());

		/* Provoke MCU stop */
		if(const auto res = write_byte(0x15, 0x16);
			res.is_err()) return Err(res.unwrap_err());
		if(const auto res = write_byte(0x14, 0x01);
			res.is_err()) return Err(res.unwrap_err());

		/* Poll for G02 status 0 MCU stop */
		while(((tmp & (uint8_t)0x80) >> 7) == (uint8_t)0x00)
		{
			if(const auto res = read_byte(0x6, &tmp);
				res.is_err()) return Err(res.unwrap_err());
			clock::delay(10ms);
			timeout++;	/* Timeout reached after 5 seconds */

			if(timeout > (uint16_t)500)
			{
				return Err(map_status_to_error(tmp));
				break;
			}
		}
	}

	/* Check GO2 status 1 if status is still OK */
	if(const auto res = read_byte(0x6, &tmp);
		res.is_err()) return Err(res.unwrap_err());

	if((tmp & (uint8_t)0x80) != (uint8_t)0){
		if(const auto res = read_byte(0x7, &tmp);
			res.is_err()) return Err(res.unwrap_err());
		if((tmp != (uint8_t)0x84) && (tmp != (uint8_t)0x85)){
			return Err(map_status_to_error(tmp));
		}
	}

	/* Undo MCU stop */
	if(const auto res = write_byte(0x7fff, 0x00);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x14, 0x00);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x15, 0x00);
		res.is_err()) return Err(res.unwrap_err());

	/* Stop xshut bypass */
	if(const auto res = write_byte(0x09, 0x04);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte(0x7fff, 0x02);
		res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<bool> Self::is_data_ready(){
	if(const auto res = read_burst(0x0, temp_buffer, 4);
		res.is_err()) return Err(res.unwrap_err());

	if((temp_buffer[0] != stream_count_)
			&& (temp_buffer[0] != (uint8_t)255)
			&& (temp_buffer[1] == (uint8_t)0x5)
			&& ((temp_buffer[2] & (uint8_t)0x5) == (uint8_t)0x5)
			&& ((temp_buffer[3] & (uint8_t)0x10) ==(uint8_t)0x10)
			)
	{
        stream_count_ = temp_buffer[0];
		return Ok(true);
	}else{
        if ((temp_buffer[3] & (uint8_t)0x80) != (uint8_t)0){
        	return Err(map_status_to_error(temp_buffer[2]));	/* Return GO2 error status */
        }

		return Ok(false);
	}
}

IResult<> Self::get_ranging_data(
		VL53L5CX_Frame		*p_results)
{
	size_t msize;
	if(const auto res = read_burst(0x0,
		temp_buffer, data_read_size_);
		res.is_err()) return Err(res.unwrap_err());

	stream_count_ = temp_buffer[0];
	SwapBuffer(temp_buffer, (uint16_t)data_read_size_);

	/* Start conversion at position 16 to avoid headers */
	for (size_t i = (uint32_t)16; i < (uint32_t)data_read_size_; i+=(uint32_t)4){
		Block_header & bh = *reinterpret_cast<Block_header *>(temp_buffer + i);
		if ((bh.type > (uint32_t)0x1) 
                    && (bh.type < (uint32_t)0xd)){
			msize = bh.type * bh.size;
		}else{
			msize = bh.size;
		}

		switch(bh.idx){
			case VL53L5CX_METADATA_IDX:
				p_results->silicon_temp_degc =
						(int8_t)temp_buffer[i + (uint32_t)12];
				break;

#ifndef VL53L5CX_DISABLE_AMBIENT_PER_SPAD
			case VL53L5CX_AMBIENT_RATE_IDX:
				(void)memcpy(p_results->ambient_per_spad,
				&(temp_buffer[i + (uint32_t)4]), msize);
				break;
#endif
#ifndef VL53L5CX_DISABLE_NB_SPADS_ENABLED
			case VL53L5CX_SPAD_COUNT_IDX:
				(void)memcpy(p_results->nb_spads_enabled,
				&(temp_buffer[i + (uint32_t)4]), msize);
				break;
#endif
#ifndef VL53L5CX_DISABLE_NB_TARGET_DETECTED
			case VL53L5CX_NB_TARGET_DETECTED_IDX:
				(void)memcpy(p_results->nb_target_detected,
				&(temp_buffer[i + (uint32_t)4]), msize);
				break;
#endif
#ifndef VL53L5CX_DISABLE_SIGNAL_PER_SPAD
			case VL53L5CX_SIGNAL_RATE_IDX:
				(void)memcpy(p_results->signal_per_spad,
				&(temp_buffer[i + (uint32_t)4]), msize);
				break;
#endif
#ifndef VL53L5CX_DISABLE_RANGE_SIGMA_MM
			case VL53L5CX_RANGE_SIGMA_MM_IDX:
				(void)memcpy(p_results->range_sigma_mm,
				&(temp_buffer[i + (uint32_t)4]), msize);
				break;
#endif
#ifndef VL53L5CX_DISABLE_DISTANCE_MM
			case VL53L5CX_DISTANCE_IDX:
				(void)memcpy(p_results->distance_mm,
				&(temp_buffer[i + (uint32_t)4]), msize);
				break;
#endif
#ifndef VL53L5CX_DISABLE_REFLECTANCE_PERCENT
			case VL53L5CX_REFLECTANCE_EST_PC_IDX:
				(void)memcpy(p_results->reflectance,
				&(temp_buffer[i + (uint32_t)4]), msize);
				break;
#endif
#ifndef VL53L5CX_DISABLE_TARGET_STATUS
			case VL53L5CX_TARGET_STATUS_IDX:
				(void)memcpy(p_results->target_status,
				&(temp_buffer[i + (uint32_t)4]), msize);
				break;
#endif
#ifndef VL53L5CX_DISABLE_MOTION_INDICATOR
			case VL53L5CX_MOTION_DETEC_IDX:
				(void)memcpy(&p_results->motion_indicator,
				&(temp_buffer[i + (uint32_t)4]), msize);
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
	for(uint32_t i = 0; i < (uint32_t)VL53L5CX_RESOLUTION_8X8; i++)
	{
		p_results->ambient_per_spad[i] /= (uint32_t)2048;
	}
#endif

	for(uint32_t i = 0; i < (uint32_t)(VL53L5CX_RESOLUTION_8X8
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
	for(uint32_t i = 0; i < (uint32_t)VL53L5CX_RESOLUTION_8X8; i++)
	{
		if(p_results->nb_target_detected[i] == (uint8_t)0){
			for(uint32_t j = 0; j < (uint32_t)
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
	for(uint32_t i = 0; i < (uint32_t)32; i++)
	{
		p_results->motion_indicator.motion[i] /= (uint32_t)65535;
	}
#endif

#endif
    return Ok();
}

IResult<Self::Resolution> Self::get_resolution()
{

	if(const auto res = dci_read_data(temp_buffer,
		VL53L5CX_DCI_ZONE_CONFIG, 8);
		res.is_err()) return Err(res.unwrap_err());
	return Ok(std::bit_cast<Resolution>(uint8_t(temp_buffer[0x00]*temp_buffer[0x01])));
}



IResult<> Self::set_resolution(Resolution resolution)
{

	switch(resolution){
		case Resolution::_4x4:
			if(const auto res = dci_read_data(
				temp_buffer,
				VL53L5CX_DCI_DSS_CONFIG, 16);
				res.is_err()) return Err(res.unwrap_err());
			temp_buffer[0x04] = 64;
			temp_buffer[0x06] = 64;
			temp_buffer[0x09] = 4;
			if(const auto res = dci_write_data(
				temp_buffer,
				VL53L5CX_DCI_DSS_CONFIG, 16);
				res.is_err()) return Err(res.unwrap_err());

			if(const auto res = dci_read_data(
				temp_buffer,
				VL53L5CX_DCI_ZONE_CONFIG, 8);
				res.is_err()) return Err(res.unwrap_err());

			temp_buffer[0x00] = 4;
			temp_buffer[0x01] = 4;
			temp_buffer[0x04] = 8;
			temp_buffer[0x05] = 8;
			if(const auto res = dci_write_data(
				temp_buffer,
				VL53L5CX_DCI_ZONE_CONFIG, 8);
				res.is_err()) return Err(res.unwrap_err());
			break;

		case Resolution::_8x8:
			if(const auto res = dci_read_data(
				temp_buffer,
				VL53L5CX_DCI_DSS_CONFIG, 16);
				res.is_err()) return Err(res.unwrap_err());
			temp_buffer[0x04] = 16;
			temp_buffer[0x06] = 16;
			temp_buffer[0x09] = 1;
			if(const auto res = dci_write_data(
				temp_buffer,
				VL53L5CX_DCI_DSS_CONFIG, 16);
				res.is_err()) return Err(res.unwrap_err());

			if(const auto res = dci_read_data(
				temp_buffer,
				VL53L5CX_DCI_ZONE_CONFIG, 8);
				res.is_err()) return Err(res.unwrap_err());

			temp_buffer[0x00] = 8;
			temp_buffer[0x01] = 8;
			temp_buffer[0x04] = 4;
			temp_buffer[0x05] = 4;
			if(const auto res = dci_write_data(
				temp_buffer,
				VL53L5CX_DCI_ZONE_CONFIG, 8);
				res.is_err()) return Err(res.unwrap_err());

			break;

		default:
			return Err(Error::InvalidParam);
			break;
		}

	if(const auto res = send_offset_data(resolution);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = send_xtalk_data(resolution);
		res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> Self::get_ranging_frequency_hz(
		uint8_t				*p_frequency_hz)
{

	if(const auto res = dci_read_data(temp_buffer,
		VL53L5CX_DCI_FREQ_HZ, 4);
		res.is_err()) return Err(res.unwrap_err());

	*p_frequency_hz = temp_buffer[0x01];

    return Ok();
}

IResult<> Self::set_ranging_frequency_hz(
		uint8_t				frequency_hz)
{

	if(const auto res = dci_replace_data(temp_buffer,
		VL53L5CX_DCI_FREQ_HZ, 4,
		&frequency_hz, 1, 0x01);
		res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> Self::get_integration_time_ms(
		uint32_t			*p_time_ms)
{

	if(const auto res = dci_read_data(temp_buffer,
		VL53L5CX_DCI_INT_TIME, 20);
		res.is_err()) return Err(res.unwrap_err());

	(void)memcpy(p_time_ms, &(temp_buffer[0x0]), 4);
	*p_time_ms /= (uint32_t)1000;

    return Ok();
}

IResult<> Self::set_integration_time_ms(
		uint32_t			integration_time_ms)
{
        uint32_t integration = integration_time_ms;

	/* Integration time must be between 2ms and 1000ms */
	if((integration < (uint32_t)2)
           || (integration > (uint32_t)1000))
	{
		return Err(Error::InvalidParam);
	}else
	{
		integration *= (uint32_t)1000;

		if(const auto res = dci_replace_data(temp_buffer,
			VL53L5CX_DCI_INT_TIME, 20,
			ptr_cast<uint8_t *>(&integration), 4, 0x00);
			res.is_err()) return Err(res.unwrap_err());
	}

    return Ok();
}

IResult<> Self::get_sharpener_percent(
		uint8_t				*p_sharpener_percent)
{

	if(const auto res = dci_read_data(
		temp_buffer,
		VL53L5CX_DCI_SHARPENER, 16
	);
		res.is_err()) return Err(res.unwrap_err());

	*p_sharpener_percent = (temp_buffer[0xD]
                                *(uint8_t)100)/(uint8_t)255;

    return Ok();
}

IResult<> Self::set_sharpener_percent(
		uint8_t				sharpener_percent)
{
        uint8_t sharpener;

	if(sharpener_percent >= (uint8_t)100)
	{
		return Err(Error::InvalidParam);
	}
	else
	{
		sharpener = (sharpener_percent*(uint8_t)255)/(uint8_t)100;
		if(const auto res = dci_replace_data(temp_buffer,
			VL53L5CX_DCI_SHARPENER, 16,
			&sharpener, 1, 0xD
		);	res.is_err()) return Err(res.unwrap_err());
	}

    return Ok();
}

IResult<Self::TargetOrder> Self::get_target_order()
{

	if(const auto res = dci_read_data(temp_buffer,
			VL53L5CX_DCI_TARGET_ORDER, 4);
		res.is_err()) return Err(res.unwrap_err());

    return try_into_enum<Self::TargetOrder>(temp_buffer[0x0]);
}

IResult<> Self::set_target_order(Self::TargetOrder target_order)
{

	if((std::bit_cast<uint8_t>(target_order) == (uint8_t)VL53L5CX_TARGET_ORDER_CLOSEST)
		|| (std::bit_cast<uint8_t>(target_order) == (uint8_t)VL53L5CX_TARGET_ORDER_STRONGEST))
	{}else{
		return Err(Error::InvalidParam);
	}

	if(const auto res = dci_replace_data(temp_buffer,
			VL53L5CX_DCI_TARGET_ORDER, 4,
			ptr_cast<const uint8_t *>(&target_order), 1, 0x0);
		res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<Self::RangingMode> Self::get_ranging_mode(){
	if(const auto res = dci_read_data(temp_buffer,
			VL53L5CX_DCI_RANGING_MODE, 8);
		res.is_err()) return Err(res.unwrap_err());

	if(temp_buffer[0x01] == (uint8_t)0x1){
		return Ok(RangingMode::Continuous);
	}else{
		return Ok(RangingMode::Autonomous);
	}
}

IResult<> Self::set_ranging_mode(RangingMode ranging_mode){
	uint32_t single_range = 0x00;

	if(const auto res = dci_read_data(temp_buffer,
			VL53L5CX_DCI_RANGING_MODE, 8);
		res.is_err()) return Err(res.unwrap_err());
	switch(ranging_mode)
	{
		case RangingMode::Continuous:
			temp_buffer[0x01] = 0x1;
			temp_buffer[0x03] = 0x3;
			single_range = 0x00;
			break;

		case RangingMode::Autonomous :
			temp_buffer[0x01] = 0x3;
			temp_buffer[0x03] = 0x2;
			single_range = 0x01;
			break;

		default:
			return Err(Error::InvalidParam);
			break;
	}

	if(const auto res = dci_write_data(
		temp_buffer,
		VL53L5CX_DCI_RANGING_MODE, (uint16_t)8
	);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = dci_write_data(
		ptr_cast<const uint8_t *>(&single_range),
		VL53L5CX_DCI_SINGLE_RANGE, (uint16_t)sizeof(single_range)
	);
		res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> Self::dci_read_data(
		uint8_t				*data,
		uint32_t			index,
		uint16_t			data_size)
{
	const uint32_t rd_size = (uint32_t) data_size + (uint32_t)12;
	/* Check if tmp buffer is large enough */
	if((data_size + (uint16_t)12)>(uint16_t)VL53L5CX_TEMPORARY_BUFFER_SIZE){
		return Err(Error::Status);
	}

	const uint8_t cmd[] = {

		(uint8_t)(index >> 8),	
		(uint8_t)(index & (uint32_t)0xff),			
		(uint8_t)((data_size & (uint16_t)0xff0) >> 4),
		(uint8_t)((data_size & (uint16_t)0xf) << 4),
		0x00, 0x00, 0x00, 0x0f,
		0x00, 0x02, 0x00, 0x08
	};



/* Request data reading from FW */
	if(const auto res = write_burst(
		(VL53L5CX_UI_CMD_END-(uint16_t)11),cmd, sizeof(cmd));
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = poll_for_answer(4, 1, 
		VL53L5CX_UI_CMD_STATUS,
		0xff, 0x03);
		res.is_err()) return Err(res.unwrap_err());

/* Read new data sent (4 bytes header + data_size + 8 bytes footer) */
	if(const auto res = read_burst(VL53L5CX_UI_CMD_START,
		temp_buffer, rd_size);
		res.is_err()) return Err(res.unwrap_err());
	SwapBuffer(temp_buffer, data_size + (uint16_t)12);

/* Copy data from FW into input structure (-4 bytes to remove header) */
	for(uint32_t i = 0 ; i < data_size;i++){
		data[i] = temp_buffer[i + 4];
	}

    return Ok();
}

IResult<> Self::dci_write_data(
		const uint8_t				*data,
		uint32_t			index,
		uint16_t			data_size)
{

	const uint8_t headers[] = {
		(uint8_t)(index >> 8),
		(uint8_t)(index & (uint32_t)0xff),
		(uint8_t)(((data_size & (uint16_t)0xff0) >> 4)),
		(uint8_t)((data_size & (uint16_t)0xf) << 4)
	};

	const uint8_t footer[] = {0x00, 0x00, 0x00, 0x0f, 0x05, 0x01,
		(uint8_t)((data_size + (uint16_t)8) >> 8), 
		(uint8_t)((data_size + (uint16_t)8) & (uint8_t)0xFF)
	};

	uint16_t address = (uint16_t)VL53L5CX_UI_CMD_END - 
		(data_size + (uint16_t)12) + (uint16_t)1;

	/* Check if cmd buffer is large enough */
	if((data_size + (uint16_t)12) 
		> (uint16_t)VL53L5CX_TEMPORARY_BUFFER_SIZE
	)	return Err(Error::Status);



/* Copy data from structure to FW format (+4 bytes to add header) */
	for(uint16_t i = 0; i < data_size; i++)
	{
		const auto idx = (i / 4) + (4 - (i % 4));
		temp_buffer[i + 4] = data[idx];
	}

	/* Add headers and footer */
	(void)memcpy(&temp_buffer[0], headers, sizeof(headers));
	(void)memcpy(&temp_buffer[data_size + (uint16_t)4],
		footer, sizeof(footer));

	/* Send data to FW */
	if(const auto res = write_burst(address,
		temp_buffer,
		(uint32_t)((uint32_t)data_size + (uint32_t)12));
		res.is_err()) return Err(res.unwrap_err());

	if(const auto res = poll_for_answer(4, 1, 
		VL53L5CX_UI_CMD_STATUS, 0xff, 0x03);
		res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> Self::dci_replace_data(
		uint8_t				*data,
		uint32_t			index,
		uint16_t			data_size,
		const uint8_t				*new_data,
		uint16_t			new_data_size,
		uint16_t			new_data_pos)
{

	if(const auto res = dci_read_data(data, index, data_size);
		res.is_err()) return Err(res.unwrap_err());
	(void)memcpy(&(data[new_data_pos]), new_data, new_data_size);
	if(const auto res = dci_write_data(data, index, data_size);
		res.is_err()) return Err(res.unwrap_err());

    return Ok();
}


IResult<Enable> Self::get_detection_thresholds_enable(){
	if(const auto res = dci_read_data(temp_buffer,
			VL53L5CX_DCI_DET_THRESH_GLOBAL_CONFIG, 8);
		res.is_err()) return Err(res.unwrap_err());	
	return Ok(Enable::from(bool(temp_buffer[0x1])));
}

IResult<> Self::set_detection_thresholds_enable(Enable en)
{
	uint8_t tmp;
	uint8_t grp_global_config[] = {0x01, 0x00, 0x01, 0x00};

	if(en == EN){
		grp_global_config[0x01] = 0x01;
		tmp = 0x04;
	}else{
		grp_global_config[0x01] = 0x00;
		tmp = 0x0C;
	}

	/* Set global interrupt config */
	if(const auto res = dci_replace_data(temp_buffer,
			VL53L5CX_DCI_DET_THRESH_GLOBAL_CONFIG, 8,
			grp_global_config, 4, 0x00);
		res.is_err()) return Err(res.unwrap_err());

	/* Update interrupt config */
	if(const auto res = dci_replace_data(temp_buffer,
			VL53L5CX_DCI_DET_THRESH_CONFIG, 20,
			&tmp, 1, 0x11);
		res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> Self::get_detection_thresholds(
	const std::span<VL53L5CX_DetectionThresholds, VL53L5CX_NB_THRESHOLDS> p_thresholds
){

	/* Get thresholds configuration */
	if(const auto res = dci_read_data(
		ptr_cast<uint8_t *>(p_thresholds.data()),
		VL53L5CX_DCI_DET_THRESH_START, 
		(uint16_t)VL53L5CX_NB_THRESHOLDS *(uint16_t)sizeof(VL53L5CX_DetectionThresholds)
	);
		res.is_err()) return Err(res.unwrap_err());
	for(uint8_t i = 0; i < (uint8_t)VL53L5CX_NB_THRESHOLDS; i++)
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

#if 0
IResult<> Self::set_detection_thresholds(
	const std::span<const VL53L5CX_DetectionThresholds, VL53L5CX_NB_THRESHOLDS> p_thresholds
){
	static constexpr uint8_t grp_valid_target_cfg[] = {
		0x05, 0x05, 0x05, 0x05,
		0x05, 0x05, 0x05, 0x05
	};

	for(size_t i = 0; i < VL53L5CX_NB_THRESHOLDS; i++){
		switch(p_thresholds[i].measurement){
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
	if(const auto res = dci_write_data(grp_valid_target_cfg,
			VL53L5CX_DCI_DET_THRESH_VALID_STATUS, 
			(uint16_t)sizeof(grp_valid_target_cfg));
		res.is_err()) return Err(res.unwrap_err());

	/* Set thresholds configuration */
	if(const auto res = dci_write_data(
			ptr_cast<uint8_t *>(p_thresholds),
			VL53L5CX_DCI_DET_THRESH_START, 
			(uint16_t)(VL53L5CX_NB_THRESHOLDS *sizeof(VL53L5CX_DetectionThresholds))
		);
		res.is_err()) return Err(res.unwrap_err());
    return Ok();
}
#endif

IResult<> Self::motion_indicator_init(
		VL53L5CX_Motion_Configuration	& motion_config,
		Resolution resolution
){

	if(const auto res = motion_indicator_set_resolution(
			motion_config, resolution);
		res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> Self::motion_indicator_set_distance_motion(
		VL53L5CX_Motion_Configuration	& motion_config,
		uint16_t			distance_min_mm,
		uint16_t			distance_max_mm)
{
	float tmp;

	if(((distance_max_mm - distance_min_mm) > (uint16_t)1500)
			|| (distance_min_mm < (uint16_t)400)
                        || (distance_max_mm > (uint16_t)4000))
	{
		return Err(Error::InvalidParam);
	}
	else
	{           
		tmp = (float)((((float)distance_min_mm/(float)37.5348)
				-(float)4.0)*(float)2048.5);
                motion_config.ref_bin_offset = (int32_t)tmp;
                
                tmp = (float)((((((float)distance_max_mm-
			(float)distance_min_mm)/(float)10.0)+(float)30.02784)
			/((float)15.01392))+(float)0.5);
		motion_config.feature_length = (uint8_t)tmp;

		if(const auto res = dci_write_data(
			ptr_cast<const uint8_t *>(&motion_config),
			VL53L5CX_DCI_MOTION_DETECTOR_CFG,
			(uint16_t)sizeof(motion_config));
			res.is_err()) return Err(res.unwrap_err());
	}

    return Ok();
}

IResult<> Self::motion_indicator_set_resolution(
		VL53L5CX_Motion_Configuration	& motion_config,
		Resolution resolution){

	switch(resolution){
		case Resolution::_4x4:
			for(uint8_t i = 0; i < (uint8_t)VL53L5CX_RESOLUTION_4X4; i++){
				motion_config.map_id[i] = (int8_t)i;
			}
			(void)memset(motion_config.map_id + 16, -1, 48);
			break;

		case Resolution::_8x8:
			for(uint8_t i = 0; i < (uint8_t)VL53L5CX_RESOLUTION_8X8; i++){
				motion_config.map_id[i] = (int8_t)((((int8_t)
				i % 8)/2) + (4*((int8_t)i/16)));
			}
			break;

		default:
			return Err(Error::Status);
		
		break;
	}


	if(const auto res = dci_write_data(
		ptr_cast<const uint8_t *>(&motion_config),
		VL53L5CX_DCI_MOTION_DETECTOR_CFG, 
		(uint16_t)sizeof(motion_config)
	);	res.is_err()) return Err(res.unwrap_err());

    return Ok();
}



IResult<> Self::poll_for_answer(
	uint8_t					size,
	uint8_t					pos,
	uint16_t				address,
	uint8_t					mask,
	uint8_t					expected_value
){
	uint8_t timeout = 0;

	do {
		if(const auto res = read_burst(address, temp_buffer, 4);
			res.is_err()) return Err(res.unwrap_err());
		clock::delay(10ms);
		
		/* 2s timeout or FW error*/
		if((timeout >= (uint8_t)200) || (temp_buffer[2] >= (uint8_t) 0x7f))
		{
			return Err(Error::McuError);
			break;
		}else{
			timeout++;
		}
	}while ((temp_buffer[0x1]) != expected_value);

    return Ok();
}

/*
 * Inner function, not available outside this file. This function is used to
 * program the output using the macro defined into the 'platform.h' file.
 */

IResult<> Self::program_output_config()
{
	uint64_t header_config;

	const auto resolution = ({
		const auto res = get_resolution();
		if(res.is_err()) return Err(res.unwrap_err());
		res.unwrap();
	});


	data_read_size_ = 0;

	/* Enable mandatory output (meta and common data) */
	static constexpr uint32_t output_bh_enable[] = {
			0x0001FFFFU,
			0x00000000U,
			0x00000000U,
			0xC0000000U};

	/* Send addresses of possible output */
	static constexpr uint32_t output[] ={
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
	for (uint32_t i = 0; i < (uint32_t)(sizeof(output)/sizeof(uint32_t)); i++)
	{
		if ((output[i] == (uint8_t)0) 
			|| ((output_bh_enable[i/(uint32_t)32]
			&((uint32_t)1 << (i%(uint32_t)32))) == (uint32_t)0))
		{
			continue;
		}

		auto bh = std::bit_cast<Block_header>(output[i]);
		if (((uint8_t)bh.type >= (uint8_t)0x1) 
                    && ((uint8_t)bh.type < (uint8_t)0x0d))
		{
			if ((bh.idx >= (uint16_t)0x54d0) 
                            && (bh.idx < (uint16_t)(0x54d0 + 960)))
			{
				bh.size = static_cast<uint8_t>(resolution);
			}	
			else 
			{
				bh.size = (uint8_t)(static_cast<uint8_t>(resolution) 
                                  * (uint8_t)VL53L5CX_NB_TARGET_PER_ZONE);
			}

                        
			data_read_size_ += bh.type * bh.size;
		}
		else
		{
			data_read_size_ += bh.size;
		}

		data_read_size_ += (uint32_t)4;
	}
	data_read_size_ += (uint32_t)20;

	if(const auto res = dci_write_data(
		ptr_cast<const uint8_t *>(output), 
		VL53L5CX_DCI_OUTPUT_LIST, (uint16_t)sizeof(output));
		res.is_err()) return Err(res.unwrap_err());
        
	header_config =  (uint32_t)(sizeof(output)/sizeof(uint32_t)) + (uint64_t)1;
	header_config = header_config << 32;
	header_config += (uint64_t)data_read_size_;

	if(const auto res = dci_write_data(
		ptr_cast<const uint8_t *>(&header_config),
		VL53L5CX_DCI_OUTPUT_CONFIG, 
		(uint16_t)sizeof(header_config));
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = dci_write_data(
		ptr_cast<const uint8_t *>(&output_bh_enable),
		VL53L5CX_DCI_OUTPUT_ENABLES, 
		(uint16_t)sizeof(output_bh_enable));
		res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> Self::calibrate_xtalk(
		uint16_t			reflectance_percent,
		uint8_t				nb_samples,
		uint16_t			distance_mm)
{
	static constexpr uint8_t cmd[] = {0x00, 0x03, 0x00, 0x00};
	static constexpr uint8_t footer[] = {0x00, 0x00, 0x00, 0x0F, 0x00, 0x01, 0x03, 0x04};
	uint16_t timeout = 0;
	uint8_t continue_loop = 1;

	uint8_t frequency, sharp_prct;
	uint32_t integration_time_ms, xtalk_margin;
        
	uint16_t reflectance = reflectance_percent;
	uint8_t	samples = nb_samples;
	uint16_t distance = distance_mm;

	/* Get initial configuration */
	const auto resolution = ({
		const auto res = get_resolution();
		if(res.is_err()) return Err(res.unwrap_err());
		res.unwrap();
	});

	if(const auto res = get_ranging_frequency_hz(&frequency);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = get_integration_time_ms(&integration_time_ms);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = get_sharpener_percent(&sharp_prct);
		res.is_err()) return Err(res.unwrap_err());
	const TargetOrder target_order =({

		const auto res = get_target_order();
		if(res.is_err()) return Err(res.unwrap_err());
		res.unwrap();
	});

	if(const auto res = get_xtalk_margin(&xtalk_margin);
		res.is_err()) return Err(res.unwrap_err());

	const RangingMode ranging_mode = ({
		const auto res = get_ranging_mode();
		if(res.is_err()) return Err(res.unwrap_err());
		res.unwrap();
	});

	/* Check input arguments validity */
	if(((reflectance < (uint16_t)1) || (reflectance > (uint16_t)99))
		|| ((distance < (uint16_t)600) || (distance > (uint16_t)3000))
		|| ((samples < (uint8_t)1) || (samples > (uint8_t)16)))
	{
		return Err(Error::InvalidParam);
	}else{
		if(const auto res = set_resolution(
			Resolution::_8x8);
			res.is_err()) return Err(res.unwrap_err());

		/* Send Xtalk calibration buffer */
		(void)memcpy(temp_buffer, VL53L5CX_CALIBRATE_XTALK, 
				sizeof(VL53L5CX_CALIBRATE_XTALK));

		if(const auto res = write_burst(0x2c28,
			temp_buffer, 
			(uint16_t)sizeof(VL53L5CX_CALIBRATE_XTALK));
			res.is_err()) return Err(res.unwrap_err());
		if(const auto res = _poll_for_answer(
			VL53L5CX_UI_CMD_STATUS, 0x3);
			res.is_err()) return Err(res.unwrap_err());

		/* Format input argument */
		reflectance = reflectance * (uint16_t)16;
		distance = distance * (uint16_t)4;

		/* Update required fields */
		if(const auto res = dci_replace_data(temp_buffer,
			VL53L5CX_DCI_CAL_CFG, 8, 
			ptr_cast<const uint8_t *>(&distance), 2, 0x00);
			res.is_err()) return Err(res.unwrap_err());

		if(const auto res = dci_replace_data(temp_buffer,
			VL53L5CX_DCI_CAL_CFG, 8,
			ptr_cast<const uint8_t *>(&reflectance), 2, 0x02);
			res.is_err()) return Err(res.unwrap_err());

		if(const auto res = dci_replace_data(temp_buffer,
			VL53L5CX_DCI_CAL_CFG, 8, 
			&samples, 1, 0x04);
			res.is_err()) return Err(res.unwrap_err());

		/* Program output for Xtalk calibration */
		if(const auto res = program_output_config();
			res.is_err()) return Err(res.unwrap_err());

		/* Start ranging session */
		if(const auto res = write_burst(
			VL53L5CX_UI_CMD_END - (uint16_t)(4 - 1),
			cmd, sizeof(cmd));
			res.is_err()) return Err(res.unwrap_err());
		if(const auto res = _poll_for_answer(
			VL53L5CX_UI_CMD_STATUS, 0x3);
			res.is_err()) return Err(res.unwrap_err());

		/* Wait for end of calibration */
		do {
			if(const auto res = read_burst(
				0x0, temp_buffer, 4);
				res.is_err()) return Err(res.unwrap_err());

			if(temp_buffer[0] != VL53L5CX_STATUS_ERROR)
			{
				/* Coverglass too good for Xtalk calibration */
				if((temp_buffer[2] >= (uint8_t)0x7f) &&
					(((uint16_t)(temp_buffer[3] & 
					(uint16_t)0x80) >> 7) == (uint16_t)1))
				{
					(void)memcpy(xtalk_data, 
						VL53L5CX_DEFAULT_XTALK,
						VL53L5CX_XTALK_BUFFER_SIZE);
				}
				continue_loop = (uint8_t)0;
			}
			else if(timeout >= (uint16_t)400)
			{
				return Err(Error::Status);
				continue_loop = (uint8_t)0;
			}
			else
			{
				timeout++;
				clock::delay(50ms);
			}

		}while (continue_loop == (uint8_t)1);
	}

	/* Save Xtalk data into the Xtalk buffer */
        (void)memcpy(temp_buffer, VL53L5CX_GET_XTALK_CMD, 
			sizeof(VL53L5CX_GET_XTALK_CMD));
	if(const auto res = write_burst(0x2fb8,
		temp_buffer, 
		(uint16_t)sizeof(VL53L5CX_GET_XTALK_CMD));
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = _poll_for_answer(VL53L5CX_UI_CMD_STATUS, 0x03); res.is_err()) return Err(res.unwrap_err());

	if(const auto res = read_burst(VL53L5CX_UI_CMD_START,
		temp_buffer, 
		VL53L5CX_XTALK_BUFFER_SIZE + (uint16_t)4);
		res.is_err()) return Err(res.unwrap_err());

	(void)memcpy(&(xtalk_data[0]), &(temp_buffer[8]),
			VL53L5CX_XTALK_BUFFER_SIZE - (uint16_t)8);
	(void)memcpy(&(xtalk_data[VL53L5CX_XTALK_BUFFER_SIZE 
			- (uint16_t)8]), footer, sizeof(footer));

	/* Reset default buffer */
	if(const auto res = write_burst(0x2c34,
			VL53L5CX_DEFAULT_CONFIGURATION,
			VL53L5CX_CONFIGURATION_SIZE);
        res.is_err()) return Err(res.unwrap_err());
    
	if(const auto res = _poll_for_answer(VL53L5CX_UI_CMD_STATUS, 0x03);
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

IResult<> Self::_poll_for_answer(
		uint16_t 				address,
		uint8_t 				expected_value)
{
	uint8_t timeout = 0;

	do {
		if(const auto res = read_burst(address, temp_buffer, 4);
			res.is_err()) return Err(res.unwrap_err());
		clock::delay(10ms);
                /* 2s timeout or FW error*/
		if((timeout >= (uint8_t)200) 
				|| (temp_buffer[2] >= (uint8_t) 0x7f))
		{
			return Err(Error::McuError);
			break;
		}else{
			timeout++;
			}

	}while ((temp_buffer[0x1]) != expected_value);
        
	return Ok();
}

IResult<> Self::get_caldata_xtalk(
		uint8_t				*p_xtalk_data)
{
	uint8_t footer[] = {0x00, 0x00, 0x00, 0x0F, 0x00, 0x01, 0x03, 0x04};

	const auto resolution = ({
		const auto res = get_resolution();
		if(res.is_err()) return Err(res.unwrap_err());
		res.unwrap();
	});

	if(const auto res = set_resolution(Resolution::_8x8);
		res.is_err()) return Err(res.unwrap_err());

	(void)memcpy(temp_buffer, VL53L5CX_GET_XTALK_CMD, 
		sizeof(VL53L5CX_GET_XTALK_CMD));

	if(const auto res = write_burst(0x2fb8,
		temp_buffer,  sizeof(VL53L5CX_GET_XTALK_CMD));
		res.is_err()) return Err(res.unwrap_err());

	if(const auto res = _poll_for_answer(VL53L5CX_UI_CMD_STATUS, 0x03); 
		res.is_err()) return Err(res.unwrap_err());

	if(const auto res = read_burst(VL53L5CX_UI_CMD_START,
		temp_buffer, 
		VL53L5CX_XTALK_BUFFER_SIZE + (uint16_t)4);
		res.is_err()) return Err(res.unwrap_err());

	(void)memcpy(&(p_xtalk_data[0]), &(temp_buffer[8]),
			VL53L5CX_XTALK_BUFFER_SIZE-(uint16_t)8);
	(void)memcpy(&(p_xtalk_data[VL53L5CX_XTALK_BUFFER_SIZE - (uint16_t)8]),
			footer, sizeof(footer));

	if(const auto res = set_resolution(resolution);
		res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> Self::set_caldata_xtalk(
		uint8_t				*p_xtalk_data)
{
	const auto resolution = ({
		const auto res = get_resolution();
		if(res.is_err()) return Err(res.unwrap_err());
		res.unwrap();
	});

	(void)memcpy(xtalk_data, p_xtalk_data, VL53L5CX_XTALK_BUFFER_SIZE);
	if(const auto res = set_resolution(resolution);
		res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> Self::get_xtalk_margin(
		uint32_t			*p_xtalk_margin)
{

	if(const auto res = dci_read_data(temp_buffer,
		VL53L5CX_DCI_XTALK_CFG, 16);
		res.is_err()) return Err(res.unwrap_err());

	(void)memcpy(p_xtalk_margin, temp_buffer, 4);
	*p_xtalk_margin = *p_xtalk_margin/(uint32_t)2048;

    return Ok();
}

IResult<> Self::set_xtalk_margin(uint32_t xtalk_margin)
{
	uint32_t margin_kcps = xtalk_margin;

	if(margin_kcps > (uint32_t)10000)
	{
		return Err(Error::InvalidParam);
	}
	else
	{
		margin_kcps = margin_kcps*(uint32_t)2048;
		if(const auto res = dci_replace_data(temp_buffer,
			VL53L5CX_DCI_XTALK_CFG, 16, 
			ptr_cast<const uint8_t *>(&margin_kcps), 4, 0x00);
			res.is_err()) return Err(res.unwrap_err());
	}

    return Ok();
}



IResult<> Self::read_byte(const uint16_t addr, uint8_t *data){
	return Ok();
}

IResult<> Self::write_byte(const uint16_t addr, uint8_t data){
	return Ok();
}

IResult<> Self::read_burst(const uint16_t addr, uint8_t *data, uint16_t size){
	return Ok();
}

IResult<> Self::write_burst(const uint16_t addr, const uint8_t *data, uint16_t size){
	return Ok();
}
#endif