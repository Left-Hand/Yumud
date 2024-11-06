#pragma once


namespace yumud::traits{

class I2cTrait{
protected:
    virtual void reset() = 0;
    virtual void unlock_bus() = 0;
public:
    virtual void init(const unsigned long baudrate) = 0;

    I2cTrait() = default;
    DELETE_COPY_AND_MOVE(I2cTrait)
};

}