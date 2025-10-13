#include "AS5047.hpp"


using namespace ymd;
using namespace ymd::drivers;

using Error = AS5047::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> AS5047::init() {
    TODO();
    return Ok();
}

uint16_t AS5047::get_position_data(){
    TODO();
    return 0;
}

IResult<> AS5047::update() {
    TODO();
    return Ok();
}



class TransmissionFrame{
protected:
    uint16_t addr_:14;
    union{
        uint16_t read_:1;
        uint16_t ef:1;
    };
    uint16_t parity_:1;

    static constexpr bool calc_parity(const uint16_t data){
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

IResult<> AS5047::write_reg(const RegAddr addr, const uint8_t data){

    // WRFormat format = {
    //     .addr = addr,
    //     .type = 0b0110
    // };

    // spi_drv_.write_single(format, CONT);

    // spi_drv_.write_single(data);
    TODO();
    return Ok();
}


IResult<> AS5047::read_reg(const RegAddr addr, uint8_t & data){

    // WRFormat format = {
    //     .addr = addr,
    //     .type = 0b0011
    // };

    // spi_drv_.write_single(format, CONT);
    // spi_drv_.read_single(data);
    TODO();
    return Ok();
}