#include "tb.h"

#include "sys/clock/clock.h"
#include "sys/debug/debug.hpp"

#include "algo/quicklz/quicklz_warpper.hpp"

void qlz_tb(OutputStream & logger){

	logger.println("qlz_tb begin");
	delay(20);
    std::vector<uint8_t> ss = {12,3,4};
	logger.println("raw:", ss);
	auto comp = quicklz_compress(ss);
	auto ret = quicklz_decompress(comp);
	
	logger.println("comp", comp);
	logger.println(ret);
}

void qlz_main(){
	DEBUGGER_INST.init(DEBUG_UART_BAUD);
	DEBUGGER.retarget(&DEBUGGER_INST);
	DEBUGGER.setSplitter(" ");
	qlz_tb(DEBUGGER);
}