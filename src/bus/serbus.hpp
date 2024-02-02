#ifndef __SERBUS_HPP__

#define __SERBUS_HPP__

#include "../bus.hpp"
#include "../../defines/comm_inc.h"
#include "../../clock/clock.h"
#include "../../types/buffer/stack/stack_t.hpp"

class SerBus:public Bus{
protected:
    virtual void begin_use(const uint8_t & index) = 0;
    virtual void end_use() = 0;

    bool under_use = false;
public:
    Spi2():Bus(){;}

    __fast_inline void begin(const uint8_t & index) override {
        under_use = true;
        begin_use(index);
    }

    __fast_inline void end() override {
        end_use();
        under_use = false;
    }

    bool busy() override{
        return under_use;
    }
};