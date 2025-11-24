#include "fdc2x1x.hpp"

using namespace ymd;
using namespace ymd::drivers;


#ifdef FDC2X1X_DEBUG
#define FDC2X1X_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define FDC2X1X_DEBUG(...)
#endif

using Error = FDC2X1X::Error;

template<typename T = void>
using IResult = Result<T, Error>;

static constexpr auto INIT_MAP = std::to_array<std::pair<uint8_t, uint16_t>>({
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
	for(const auto & [addr,val] : INIT_MAP){
		if(const auto res = write_reg(addr,val);
			res.is_err()) return res;
	}

	return Ok();
}


IResult<bool> FDC2X1X::is_conv_done(){
	if(const auto res = read_reg(StatusReg::ADDRESS, status_reg.as_bits_mut());
		res.is_err()) return Err(res.unwrap_err());
	return Ok(bool(status_reg.data_ready));
}

IResult<bool> FDC2X1X::is_conv_done(uint8_t idx){
	if(idx > 3) return Err(Error::ChannelSelectionOutOfRange);

	if(const auto res = read_reg(StatusReg::ADDRESS, status_reg.as_bits_mut());
		res.is_err()) return Err(res.unwrap_err());
	switch(idx){
		case 0: return Ok(bool(status_reg.ch0_unread_conv));
		case 1: return Ok(bool(status_reg.ch1_unread_conv));
		case 2: return Ok(bool(status_reg.ch2_unread_conv));
		case 3: return Ok(bool(status_reg.ch3_unread_conv));
		default: __builtin_unreachable();
	}
}

IResult<> FDC2X1X::reset(){
	auto reg = RegCopy(reset_dev_reg);
	reg.reset_dev = true;
	return write_reg(ResetDevReg::ADDRESS,(reg.to_bits()));
}

IResult<uint32_t> FDC2X1X::get_data(uint8_t idx){
	if(idx > 3)  return Err(Error::ChannelSelectionOutOfRange);

	uint32_t ret = 0;
	auto & highreg = conv_data_regs[idx].high;
	auto & lowreg = conv_data_regs[idx].low;

	if(const auto res = read_reg(highreg.ADDRESS, (highreg.as_bits_mut()));
		res.is_err()) return Err(res.unwrap_err());
	ret |= (highreg.data_msb << 16);
	if(const auto res = read_reg(lowreg.ADDRESS, (lowreg.as_bits_mut()));
		res.is_err()) return Err(res.unwrap_err());
	ret |= lowreg.data_lsb;

	return Ok(ret);
}