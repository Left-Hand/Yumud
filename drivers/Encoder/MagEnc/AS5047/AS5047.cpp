#include "AS5047.hpp"


using namespace ymd::drivers;
using namespace ymd;



void AS5047::init() {

}

uint16_t AS5047::getPositionData(){
    return 0;
}

void AS5047::update() {

}



class TransmissionFrame{
protected:
    uint16_t addr_:14;
    union{
        uint16_t read_:1;
        uint16_t ef:1;
    };
    uint16_t parity_:1;

    scexpr bool calc_parity(const uint16_t data){
        int count = 0;
        for (int i = 0; i < 16; ++i) {
            if (data & (1 << i)) {
                count++;
            }
        }
        return (count % 2 == 0) ? true : false;
    }
};

class CommandFrame : public TransmissionFrame{
public:
    CommandFrame(const uint16_t addr, const uint8_t read){
        addr_ = addr;
        read_ = read;
        parity_ = 0;
        parity_ = calc_parity(*reinterpret_cast<uint16_t*>(this));
    }
};

class ReadFrame:public TransmissionFrame{
public:
    // ReadFrame()
};

void AS5047::write_reg(const RegAddress addr, const uint8_t data){

    // WRFormat format = {
    //     .addr = addr,
    //     .type = 0b0110
    // };

    // spi_drv_.write_single(format, CONT);
    // spi_drv_.write_single(data);
}


void AS5047::read_reg(const RegAddress addr, uint8_t & data){

    // WRFormat format = {
    //     .addr = addr,
    //     .type = 0b0011
    // };

    // spi_drv_.write_single(format, CONT);
    // spi_drv_.read_single(data);
}