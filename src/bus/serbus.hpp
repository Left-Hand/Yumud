#ifndef __SERBUS_HPP__

#define __SERBUS_HPP__

#include "bus.hpp"

class SerBus:public Bus{
protected:
    virtual void begin_use(const uint8_t & index = 0) = 0;
    virtual void end_use() = 0;
    virtual bool usable(const uint8_t & index = 0) = 0;

public:

    __fast_inline Error begin(const uint8_t & index = 0) override {
        if(usable(index)){
            begin_use(index);
            return Bus::ErrorType::OK;
        }else{
            return Bus::ErrorType::IS_USING;
        }
    }

    __fast_inline void end() override {
        end_use();
    }

};

#endif