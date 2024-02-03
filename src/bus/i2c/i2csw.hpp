// /*
//  * i2c-soft.h
//  *
//  *  Created on: Jun 9, 2013
//  *      Author: agu
//  */

// #ifndef I2C_SOFT_H_
// #define I2C_SOFT_H_

// #include "i2c.hpp"
// #include "../../gpio/gpio.hpp"

// class I2cSw: public SerBus{
// private:
    
//     Gpio & scl;
//     Gpio & sda;
//     int8_t occupied = -1;

//     void wbyte_non_delay(const uint8_t & data);
//     uint8_t rbyte_non_delay();

//     __fast_inline void clk(){
//         scl = true;
//         scl = false;
//     }

//     __fast_inline void ack(){
//         sda = false;
//         scl = true;
//         scl = false;
//     }

//     __fast_inline void nack(void) {
//         sda = true;
//         scl = true;
//         scl = false;
//     }

//     __fast_inline void start(const uint8_t & _address) override{
//         occupied = _address;
//         sda = true;
//         scl = true;
//         sda = false;
//         scl = false;
//         _write(_address);
//     }

//     void stop() override{
//         sda = false;
//         scl = true;
//         sda = true;
//         occupied = -1;
//     }

// protected :
//     __fast_inline void begin_use(const uint8_t & index = 0) override {start(index);}
//     __fast_inline void end_use() override {stop();}
//     bool usable(const uint8_t & index = 0) override {
//         return (occupied >= 0 ? (occupied == (int8_t)index) : true);
//     }

//     public:

//         I2cSw(Pin & _scl,Pin & _sda):scl(_scl), sda(_sda){;};

//         void begin() override;

//         void _write_single(const uint8_t & data){
//             _write(data);
//         }

//         void _write_head(const uint8_t & head){
//             _write(head);
//         }

//         void _write_cont(const uint8_t & cont){
//             _write(cont);
//         }
        
//         void _write_tail(const uint8_t * tail_ptr, const uint8_t & len, bool lsb = false){
//             _write(tail_ptr, len, lsb);
//         }


//         uint8_t _read_single(){
//            return  _read(false);
//         }   

//         void _read_head(uint8_t * data_ptr){
//             *data_ptr = _read(true);
//         }
        
//         void _read_tail(uint8_t * tail_ptr, const uint8_t & len, bool lsb = false){
//             _read(tail_ptr, len, lsb);
//         }

//         void _write(const uint8_t * data_ptr, const uint8_t & len, bool lsb = false) override{

//             for(uint8_t i = 0; i < len; i++){
//                 wbyte_non_delay(data_ptr[lsb ? len - i - 1:i]);
//                 // Serial.print((char)data_ptr[inv ? len - i - 1:i]);
//             }
//         }

//         uint8_t _read(const bool & toAck) override{
//             uint8_t ret = 0;
//             if (!delay) 
//                 ret = rbyte_non_delay();
//             else{}
//             if (toAck) ack();
//             else nack();
//             return ret;
//         }

//         void _read(uint8_t * data_ptr, const uint8_t & len, bool lsb = false) override{
//             for(uint8_t i = 0; i < len; i++){
//                 data_ptr[!lsb ? len - i - 1:i] = _read((i != len - 1));
//             }
//             // Serial.println(String((int)data_ptr[0]) + ", " + String((int)data_ptr[1]));
//         }

//         bool wait_ack(){
//             bool ret;

//             sda.configMode(GPIO_DIR_MODE_IN);
//             scl.set();
//             ret = sda.read();
//             sda.configMode(GPIO_DIR_MODE_OUT);
//             scl.reset();

//             return ret;
//         }
// };

// #endif 
