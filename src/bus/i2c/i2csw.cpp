// #include "softI2c.hpp"

// #define within(time, fun) for (t = (time); (fun) && --t;)

// softI2c::softI2c(Pin & _scl,Pin & _sda):scl(_scl), sda(_sda){;}

// void softI2c::begin(){
//     sda.configType(GPIO_PIN_TYPE_STD_WPD);
//     sda.configStrength(GPIO_STRENGTH_2MA);
//     sda.configMode(GPIO_DIR_MODE_OUT);
//     scl.configType(GPIO_PIN_TYPE_STD);
//     scl.configStrength(GPIO_STRENGTH_2MA);
//     scl.configMode(GPIO_DIR_MODE_OUT);
//     scl.set();
//     sda.set();
// }


// void softI2c::wbyte_non_delay(const uint8_t & data){
//     // Serial.println(data);
//     sda.write(0x80 & data);
//     clk();
//     sda.write(0x40 & data);
//     clk();
//     sda.write(0x20 & data);
//     clk();
//     sda.write(0x10 & data);
//     clk();
    
//     sda.write(0x08 & data);
//     clk();
//     sda.write(0x04 & data);
//     clk();
//     sda.write(0x02 & data);
//     clk();
//     sda.write(0x01 & data);
//     clk();

//     wait_ack();
// }

// uint8_t softI2c::rbyte_non_delay(){
        
//     sda.configMode(GPIO_DIR_MODE_IN);
//     uint8_t ret = 0;


//     ret |= sda.read();
//     clk();
//     ret <<= 1; ret |= sda.read(); 
//     clk();
//     ret <<= 1; ret |= sda.read(); 
//     clk();
//     ret <<= 1; ret |= sda.read(); 
//     clk();
//     ret <<= 1; ret |= sda.read(); 

//     clk();
//     ret <<= 1; ret |= sda.read(); 
//     clk();
//     ret <<= 1; ret |= sda.read(); 
//     clk();
//     ret <<= 1; ret |= sda.read(); 
//     clk();
//     sda.configMode(GPIO_DIR_MODE_OUT);
//     return ret;
// }

// // void softI2c::read(uint8_t * data_ptr, const uint8_t & len){
// //     for(uint8_t i = 0; i < len; i++){
// //         data_ptr[i] = _receive(bool(i != len - 1));
// //     }
// // }

// // void softI2c::init() {
// // 	scl.init(GPIO_Mode_Out_OD);
// // 	scl.set();
// // }

// // void softI2c::start() {
// // 	sda.set();
// // 	sda.init(GPIO_Mode_Out_OD);

// // 	sda.reset();
// // 	scl.reset();
// // }

// // void softI2c::stop() {
// // 	sda.reset();
// // 	sda.init(GPIO_Mode_Out_OD);

// // 	scl.set();
// // 	sda.set();
// // }

// // void softI2c::transmit(uint8_t data) {

// // 	sda.init(GPIO_Mode_Out_OD);

// // 	for (uint8_t i = 0; i < 8; i++) {
// // 		sda.write(data & 0x80);
// // 		scl.set();
// // 		scl.reset();
// // 		data <<= 1;
// // 	}
// // }

// // uint8_t softI2c::receive() {
// // 	sda.init(GPIO_Mode_IN_FLOATING);
// // 	uint8_t data = 0;
// // 	for (u8 i = 0; i < 8; i++) {
// // 		scl.set();
// // 		data <<= 1;
// // 		data |= sda.getInput() ? 0x01 : 0x00;
// // 		scl.reset();
// // 	}
// // 	return data;
// // }

// // u16 softI2c::waitAck() {
// // 	sda.init(GPIO_Mode_IN_FLOATING);
// // 	scl.set();

// // 	vu16 t;
// // 	within(_FLAG_TIMEOUT, sda.getInput() == Bit_SET);

// // 	scl.reset();
// // 	return t;
// // }

// // uint8_t softI2c::write(uint8_t address, uint8_t *data, uint8_t length,
// // 		uint8_t send_stop) {
// // 	vu16 t;
// // 	within(_FLAG_TIMEOUT, sda.getInput() == Bit_RESET);
// // 	if (!t) return 1;

// // 	this->start();
// // 	this->transmit(address << 1);

// // 	if (this->waitAck() == 0) return 2;

// // 	while (length--) {
// // 		this->transmit(*data++);
// // 		if (this->waitAck() == 0) return 3;
// // 	}

// // 	if (send_stop) this->stop();

// // 	return t;
// // }

// // void softI2c::read(uint8_t address, uint8_t *data, uint8_t length,uint8_t send_stop) {
// // 	this->start();
// // 	this->transmit((address << 1) | 0x01);
// // 	this->waitAck();

// // 	while (length--) {
// // 		*data++ = this->receive();
// // 		this->sendAck(length);
// // 	}

// // 	if (send_stop) this->stop();
// // }

// // void softI2c::sendAck(uint8_t ack) {
// // 	scl.reset();

// // 	sda.set(ack ? Bit_RESET : Bit_SET);
// // 	sda.init(GPIO_Mode_Out_OD);

// // 	scl.set()
// // 	scl.reset()
// // }
