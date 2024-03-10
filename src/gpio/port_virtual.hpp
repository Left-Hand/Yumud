#ifndef __PORT_VIRTUAL_HPP__

#define __PORT_VIRTUAL_HPP__

#include "port.hpp"
#include "gpio.hpp"
#include <memory>

class PortVirtual : public PortBase{
protected:
    std::unique_ptr<GpioVirtual> pin_ptrs[16] = {nullptr};

    void write(const uint16_t & data);
    const uint16_t read();
public:
    PortVirtual(){;}
    void init(){;}
    void writeByIndex(const int8_t index, const bool data) override;
    bool readByIndex(const int8_t index) override;
    void bindPin(GpioVirtual & gpio, const uint8_t index);

    void bindPin(Gpio & gpio, const uint8_t index);
    void setBits(const uint16_t & data) override;
    void clrBits(const uint16_t & data) override;

    void set(const Pin & pin) override;
    void clr(const Pin & pin) override;

    PortVirtual & operator = (const uint16_t & data){write(data); return *this;}

    operator uint16_t(){return read();}

    void setModeByIndex(const int8_t & index, const PinMode & mode);
};

#endif