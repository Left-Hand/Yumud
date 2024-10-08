#ifndef __DEVICE_INC_H__
#define __DEVICE_INC_H__

#include "../sys/core/system.hpp"
#include "../hal/bus/i2c/i2cdrv.hpp"
#include "../hal/bus/spi/spidrv.hpp"
#include "../hal/bus/uart/uart.hpp"

#ifndef REG8_BEGIN
#define REG8_BEGIN union{struct{
#endif

#ifndef REG8_END
#define REG8_END };uint8_t data;};
#endif

#ifndef REG16_BEGIN
#define REG16_BEGIN union{struct{
#endif

#ifndef REG16_END
#define REG16_END };uint16_t data;};
#endif

struct Reg8{};
struct Reg16{};

struct Fraction {
public:
    int numerator;
    int denominator;
    Fraction(const int _numerator,const int _denominator) : numerator(_numerator), denominator(_denominator) {}

    int operator * (const int & value){
        return numerator * value / denominator;
    }

    Fraction operator * (const Fraction & value){
        return Fraction(numerator * value.numerator, denominator * value.denominator);
    }
};



#define I2CDEV_CONTSRTUCTER(name)\
    name(I2cDrv & _bus_drv):bus_drv(_bus_drv){;}\
    name(I2cDrv && _bus_drv):bus_drv(_bus_drv){;}\
    name(I2c & _i2c):bus_drv(I2cDrv(_i2c, name::default_addr)){;}\

#define SPIDEV_CONTSRTUCTER(name)\
    name(SpiDrv & _spi_drv):spi_drv(_spi_drv){;}\
    name(SpiDrv && _spi_drv):spi_drv(_spi_drv){;}\

#endif