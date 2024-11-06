#include "tb.h"
#include "../../algo/quicklz/quicklz_warpper.hpp"
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
	auto & logger = DEBUGGER_INST;
	logger.init(DEBUG_UART_BAUD, CommMethod::Blocking);
	logger.setSplitter(" ");
	qlz_tb(logger);
}