#include "paj7620.hpp"

#include "core/debug/debug.hpp"
#include "details/data.hpp"

// #define PAJ7620_DEBUG_EN

#ifdef PAJ7620_DEBUG_EN
#define PAJ7620_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define PAJ7620_PANIC(...) PANIC(__VA_ARGS__)
#define PAJ7620_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define PAJ7620_DEBUG(...)
#define PAJ7620_PANIC(...)  PANIC()
#define PAJ7620_ASSERT(cond, ...) ASSERT(cond, ##__VA_ARGS__)
#endif


using namespace ymd;
using namespace ymd::drivers;



using Error = PAJ7620::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> PAJ7620::read_reg(uint8_t addr, uint8_t & data){
	if(const auto res = i2c_drv_.read_reg<uint8_t>(addr, data);
		res.is_err()) return Err(res.unwrap_err());
	return Ok();
};

IResult<> PAJ7620::write_reg(uint8_t cmd, uint8_t data){
	if(const auto res = i2c_drv_.write_reg<uint8_t>(cmd, data);
		res.is_err()) return Err(res.unwrap_err());
	return Ok();
};

IResult<> PAJ7620::select_bank(Bank bank){
	switch(bank){
		case Bank::_0:
			return write_reg(PAJ7620_REGITER_BANK_SEL, 0);
		case Bank::_1:
			return write_reg(PAJ7620_REGITER_BANK_SEL, 1);
		default:
			__builtin_unreachable();
	}
}

// IResult<> PAJ7620::wakeup(){
	
// 	i2c_drv.validate();
// 	clock::delay(5ms);
// 	i2c_drv.validate();
// 	clock::delay(5ms);

// 	selectBank(0);//进入BANK0寄存器区域
// 	validate();
// }

IResult<> PAJ7620::unlock_i2c(){
	// i2c_drv_.release();
	if(const auto res = i2c_drv_.validate();
		res.is_err()) return Err(res.unwrap_err());
	return Ok();
}

IResult<> PAJ7620::update(){
	if(const auto res = read_reg(0x43, reinterpret_cast<uint8_t &>(flags));
		res.is_err()) return res;
	if(const auto res = unlock_i2c();
		res.is_err()) return res;
	return Ok();
}

IResult<PAJ7620::Flags> PAJ7620::detect(){
	return Ok(flags);
}

IResult<> PAJ7620::validate(){
	if(const auto res = i2c_drv_.validate();
		res.is_err()) return Err(res.unwrap_err());

	clock::delay(20ms);
	if(const auto res = select_bank(Bank::_0);
		res.is_err()) return res;

	uint8_t data0 = 0, data1 = 0;
	if(const auto res = read_reg(0, data0);
		res.is_err()) return res;
	if(const auto res = read_reg(1, data1);
		res.is_err()) return res;

	if(data0 != 0x20) return Err(Error::Data0ValidateFailed); 
	if(data1 != 0x76) return Err(Error::Data1ValidateFailed);

	return Ok();
}


IResult<> PAJ7620::init(){
	if(const auto res = validate();
		res.is_err()) return res;


	// uint8_t temp;

	// read_reg(0xef, temp);
	if(const auto res = select_bank(Bank::_0);
		res.is_err()) return res;
	for(const auto & [cmd, data] : INIT_ARRAY){
		if(const auto res = write_reg(cmd, data);
			res.is_err()) return res;
	}

	if(const auto res = select_bank(Bank::_0);
		res.is_err()) return res;

	return Ok();

	// read_reg(0xef, temp);
	// read_reg(0x32, temp);
	// PAJ7620_ASSERT(temp == 0x29, "init falied", temp);

	/**
	 * Setting normal mode or gaming mode at BANK1 register 0x65/0x66 R_IDLE_TIME[15:0]
	 * T = 256/System CLK = 32us, 
	 * Ex:
	 * Far Mode: 1 report time = (77+R_IDLE_TIME)T
	 * Report rate 120 fps:
	 * R_IDLE_TIME=1/(120*T)-77=183
	 * 
	 * Report rate 240 fps:
	 * R_IDLE_TIME=1/(240*T)-77=53
	 * 
	 * Near Mode: 1 report time = (112+R_IDLE_TIME)T
	 * 
	 * Report rate 120 fps:
	 * R_IDLE_TIME=1/(120*T)-120=148
	 * 
	 * Report rate 240 fps:
	 * R_IDLE_TIME=1/(240*T)-112=18
	 * 
	 */
	// selectBank(1);
	//paj7620WriteReg(0x65, 0xB7); // far mode 120 fps
	// write_reg(0x65, 0x12);  // near mode 240 fps

	// selectBank(0);
};