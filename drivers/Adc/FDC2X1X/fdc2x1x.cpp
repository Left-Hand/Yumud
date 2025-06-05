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