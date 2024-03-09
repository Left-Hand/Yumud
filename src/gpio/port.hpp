#ifndef __PORT_HPP__

#define __PORT_HPP__

#include "gpio.hpp"

class PortBase{
protected:
    
};

class Port : public PortBase{
protected:
    GPIO_TypeDef * instance;
public:

};

class PortVirtual : public PortBase{

};

#endif