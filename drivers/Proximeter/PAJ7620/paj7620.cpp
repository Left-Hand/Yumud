#include "paj7620.hpp"

#include "core/debug/debug.hpp"
#include "details/data.hpp"

#include <utility>

using namespace ymd;
using namespace ymd::drivers;

// #define PAJ7620_DEBUG

#ifdef PAJ7620_DEBUG
#undef PAJ7620_DEBUG
#define PAJ7620_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define PAJ7620_PANIC(...) PANIC(__VA_ARGS__)
#define PAJ7620_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define PAJ7620_DEBUG(...)
#define PAJ7620_PANIC(...)  PANIC()
#define PAJ7620_ASSERT(cond, ...) ASSERT(cond, ##__VA_ARGS__)
#endif

#define WRITE_REG(reg) this->write_reg(reg.address, reg).unwrap();
#define READ_REG(reg) this->read_reg(reg.address, reg).unwrap();

void PAJ7620::read_reg(uint8_t addr, uint8_t & data){
	const auto err = i2c_drv_.read_reg<uint8_t>(addr, data);
	if(err.is_err()){
		PAJ7620_DEBUG(err);
	}
};

void PAJ7620::write_reg(uint8_t cmd, uint8_t data){
	const auto err = i2c_drv_.write_reg<uint8_t>(cmd, data);
	// PAJ7620_DEBUG(cmd, data);
	if(err.is_err()){
		PAJ7620_DEBUG(err);
	}
};

void PAJ7620::select_bank(uint8_t bank) {
	switch(bank){
		case 9:
			write_reg(PAJ7620_REGITER_BANK_SEL, 0);
			break;
		case 1:
			write_reg(PAJ7620_REGITER_BANK_SEL, 1);
			break;
		default:
			break;
	}
}

// void PAJ7620::wakeup(){
	
// 	i2c_drv.verify();
// 	delay(5);
// 	i2c_drv.verify();
// 	delay(5);

// 	selectBank(0);//进入BANK0寄存器区域
// 	verify();
// }

hal::HalResult PAJ7620::unlock_i2c(){
	// i2c_drv_.release();
	return i2c_drv_.verify();
}

void PAJ7620::update(){
	read_reg(0x43, flags);
	unlock_i2c();
}

PAJ7620::Flags PAJ7620::detect(){
	return (flags);
}

bool PAJ7620::verify(){
	if(auto passed = i2c_drv_.verify().is_ok(); !passed){
		PAJ7620_DEBUG("PAJ7620 not found");
		return false;
	}

	delay(20);
	select_bank(0);
	// // selectBank(0);

	uint8_t data0 = 0, data1 = 0;
	read_reg(0, data0);
	read_reg(1, data1);

	PAJ7620_DEBUG(data0, data1);
	return (data0 == 0x20) and (data1 == 0x76);
}


void PAJ7620::init(){
	PAJ7620_ASSERT(verify(), "paj7620 not found");


	// uint8_t temp;

	// read_reg(0xef, temp);
	select_bank(0);
	for(const auto & [cmd, data] : INIT_ARRAY){
		write_reg(cmd, data);
	}

	select_bank(0);
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