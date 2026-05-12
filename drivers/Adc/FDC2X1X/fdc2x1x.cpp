#include "fdc2x1x.hpp"

using namespace ymd;
using namespace ymd::drivers;

#define FDC2X1X_DEBUG_EN


#ifdef FDC2X1X_DEBUG_EN

#define FDC2X1X_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define FDC2X1X_PANIC(...) PANIC{__VA_ARGS__}
#define FDC2X1X_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#else
#define FDC2X1X_DEBUG(...)
#define FDC2X1X_PANIC(...)  PANIC_NSRC()
#define FDC2X1X_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif


using Error = FDC2X1X::Error;

template<typename T = void>
using IResult = Result<T, Error>;

static constexpr auto INIT_LIST = std::to_array<std::pair<uint8_t, uint16_t>>({
	{0x08,0x8329},
	{0x09,0x8329},
	{0x0A,0x8329},
	{0x0B,0x8329},
	{0x10,0x000A},
	{0x11,0x000A},
	{0x12,0x000A},
	{0x13,0x000A},
	{0x14,0x1001},
	{0x15,0x1001},
	{0x16,0x1001},
	{0x17,0x1001},
	{0x19,0x0000},
	{0x1B,0xC20D},
	{0x1E,0x8000},
	{0x1F,0x8000},
	{0x20,0x8000},
	{0x21,0x8000},
	{0x1A,0x1401},
});


IResult<> FDC2X1X::init(){
	for(const auto & [reg_addr,reg_val] : INIT_LIST){
		if(const auto res = write_reg(reg_addr,reg_val);
			res.is_err()) return res;
	}

	return Ok();
}


IResult<bool> FDC2X1X::is_conv_done(){
	auto & reg = regs_.status_reg;
	if(const auto res = read_reg(Regset::R16_Status::REG_ADDR, reg.as_bits_mut());
		res.is_err()) return Err(res.unwrap_err());
	return Ok(bool(reg.data_ready));
}

IResult<bool> FDC2X1X::is_conv_done(uint8_t idx){
	if(idx > 3) FDC2X1X_PANIC("channel index out of range", idx);
	auto & reg = regs_.status_reg;
	if(const auto res = read_reg(Regset::R16_Status::REG_ADDR, reg.as_bits_mut());
		res.is_err()) return Err(res.unwrap_err());
	switch(idx){
		case 0: return Ok(bool(reg.ch0_unread_conv));
		case 1: return Ok(bool(reg.ch1_unread_conv));
		case 2: return Ok(bool(reg.ch2_unread_conv));
		case 3: return Ok(bool(reg.ch3_unread_conv));
	}
	__builtin_unreachable();
}

IResult<> FDC2X1X::reset(){
	auto reg = RegCopy(regs_.reset_dev_reg);
	reg.reset_dev = true;
	return write_reg(Regset::R16_ResetDev::REG_ADDR,(reg.to_bits()));
}

IResult<uint32_t> FDC2X1X::get_data(uint8_t idx){
	if(idx > 3)  FDC2X1X_PANIC("channel index out of range", idx);

	auto & highreg = regs_.conv_data[idx].high;
	auto & lowreg = regs_.conv_data[idx].low;
	
	if(const auto res = read_reg(highreg.REG_ADDR, (highreg.as_bits_mut()));
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = read_reg(lowreg.REG_ADDR, (lowreg.as_bits_mut()));
		res.is_err()) return Err(res.unwrap_err());
	
	const uint32_t ret = 
		(static_cast<uint32_t>(highreg.data_msb) << 16)
		| static_cast<uint32_t>(lowreg.data_lsb);

	return Ok(ret);
}