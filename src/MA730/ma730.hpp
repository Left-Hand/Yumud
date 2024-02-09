#ifndef __MA730_HPP__
#define __MA730_HPP__

#include "../bus/bus_inc.hpp"

#ifndef REG8_BEGIN
#define REG8_BEGIN union{struct{
#endif

#ifndef REG8_END
#define REG8_END };uint8_t data;};
#endif

class MA730{
protected:
    BusDrv & bus_drv;

    struct Reg8{};

    // struct {

    // }
    void writeReg()
public:
    MA730();
    ~MA730();

};

#endif