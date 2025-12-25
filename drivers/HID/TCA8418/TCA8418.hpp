#pragma once

//这个驱动还未完成

//TCA8418是德州仪器的一款按键扫描芯片

// https://blog.csdn.net/qq_34888956/article/details/139613908

#include "details/TCA8418_phy.hpp"
#include "hal/gpio/vgpio.hpp"

namespace ymd::drivers{

class TCA8418 final:public TCA8418_Regs{
public:
    explicit TCA8418(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        transport_(hal::I2cDrv{i2c, addr}){;}

    IResult<> validate();
    IResult<> init();
private:
    IResult<size_t> get_pending_events_cnt();
    IResult<bool> is_fifo_empty();


    TCA8418_Transport transport_;
public:
    class TCA8418_VPort:public hal::GpioPortIntf{

    };
public:
    TCA8418(const TCA8418 & other) = delete;
    TCA8418(TCA8418 && other) = default;

    void begin(void);
    void begin(uint8_t rows, uint16_t cols, uint8_t config);
    uint8_t readKeypad(void);
    bool configureKeys(uint8_t rows, uint16_t cols, uint8_t config);

    bool readByte(uint8_t *data, uint8_t reg);
    void pinMode(uint32_t pin, uint8_t mode);
    void digitalWrite(uint32_t pin, uint8_t value);
    uint8_t digitalRead(uint32_t pin);
    void write(uint32_t value);
    uint32_t read(void);
    void toggle(uint32_t pin);
    void blink(uint32_t pin, uint16_t count, uint32_t duration);
    #ifdef TCA8418_INTERRUPT_SUPPORT
    void enableInterrupt(uint8_t pin, void(*selfCheckFunction)(void));
    void disableInterrupt();
    void pinInterruptMode(uint32_t pin, uint8_t mode, uint8_t level, uint8_t fifo);
    void pinInterruptMode(uint32_t pin, uint8_t mode);
    #endif
    void readGPIO();
    void updateGPIO();
    void dumpreg(void);
    uint8_t getInterruptStatus(void);
    void clearInterruptStatus(uint8_t flags);
    void clearInterruptStatus(void);
    uint8_t getKeyEvent(uint8_t event);
    uint8_t getKeyEvent(void);
    uint8_t getKeyEventCount(void);
    uint32_t getGPIOInterrupt(void);
    bool isKeyDown(uint8_t key);
    bool getKey(uint8_t *key);
    uint8_t getKey(void);
    
    protected:
    
    #ifdef TCA8418_INTERRUPT_SUPPORT

    /** Old value of _PIN variable */
    volatile uint32_t _oldPIN;
        
    /** ISR ignore flag */
    volatile uint8_t _isrIgnore;

    /** PCINT pin used for "INT" pin handling */
    uint8_t _pcintPin;

    /** Interrupts modes of pins ( LOW, CHANGE, FALLING, RISING)  */
    uint8_t _intMode[24];

    /** Interrupts callback functions */
    void (*_intCallback[24])(void);
    #endif  

private:
    uint32_t _PKG; // Pin Keypad or GPIO 0=GPIO, 1=Keypad
    uint32_t _PORT;
    uint32_t _PIN; // Pin State
    uint32_t _DDR; //Pin Direction INPUT or OUTPUT
    uint32_t _PUR; //Pull-Up Resistor Selection
    
    void writeByte(uint8_t data, uint8_t reg);
    bool read3Bytes(uint32_t *data, uint8_t reg);
    void write3Bytes(uint32_t data, uint8_t reg);
};

}