#pragma once


#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"
#include "drivers/IMU/details/AnalogDeviceIMU.hpp"


namespace ymd::drivers{

struct ADXL345_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x1D);
    static constexpr uint8_t VALID_DEVICE_ID = 0xE5;

    enum class RegAddr:uint8_t{
        DeviceID = 0x00,
        TapThreshold = 0x1D,
        OffsetX = 0x1E,
        OffsetY = 0x1F,
        OffsetZ = 0x20,
        TapDuration = 0x21,
        TapLatency = 0x22,
        TapWindow = 0x23,
        ActivityThreshold = 0x24,
        InactivityThreshold = 0x25,
        InactivityTime = 0x26,
        AxisEnabler = 0x27,
        FreefallThreshold = 0x28,
        FreefallTime = 0x29,
        AxisControlForSingle2DoubleTap = 0x2A,
        SourceOfSingle2DoubleTap = 0x2B,
        DataRate = 0x2C,
        PowerSaving = 0x2D,
        InterruptEnabler = 0x2E,
        InterruptMap = 0x2F,
        SourceOfInterrupt = 0x30,
        DataFormat = 0x31,
        DataX0 = 0x32,
        DataX1 = 0x33,
        DataY0 = 0x34,
        DataY1 = 0x35,
        DataZ0 = 0x36,
        DataZ1 = 0x37,
        FifoCtrl = 0x37,
        FifoStatus = 0x38
    };

    using Error = ImuError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class DataRate:uint8_t{
        HZ0_1 = 0,HZ0_2, HZ0_39, HZ0_78,HZ1_56,HZ6_25,
        HZ12_5,HZ25,HZ50,HZ100,HZ200,HZ400,HZ800,HZ1600,HZ3200
    };

    enum class WakeupFreq:uint8_t{
        HZ8, HZ4, HZ2, HZ1
    };

    enum class MeasureRange:uint8_t{
        G2, G4, G8, G16
    };

    enum class FifoMode:uint8_t{
        Bypass,Fifo, Stream, Trigger
    };



};

struct ADXL345_Regset final :public ADXL345_Prelude{
    struct R8_DeviceID:public Reg8<>{
        uint8_t id;
    }DEF_R8(deviceid_reg)

    struct R8_TapThreshold:public Reg8<>{
        uint8_t data;
    };

    struct R8_OffsetX:public Reg8<>{
        uint8_t data;
    };

    struct R8_OffsetY:public Reg8<>{
        uint8_t data;
    };

    struct R8_OffsetZ:public Reg8<>{
        uint8_t data;
    };

    struct R8_TapDuration:public Reg8<>{
        uint8_t data;
    };

    struct R8_TapLatency:public Reg8<>{
        uint8_t data;
    };

    struct R8_TapWindow:public Reg8<>{
        uint8_t data;
    };

    struct R8_ActivityThreshold:public Reg8<>{
        uint8_t data;
    };

    struct R8_InactivityThreshold:public Reg8<>{
        uint8_t data;
    };

    struct R8_InactivityTime:public Reg8<>{
        uint8_t data;
    };

    struct R8_AxisEnabler:public Reg8<>{
        
        uint8_t InactiveZ :1;
        uint8_t InactiveY :1;
        uint8_t InactiveX :1;
        uint8_t InactiveACorDC:1;
        uint8_t activeX :1;
        uint8_t activeY :1;
        uint8_t activeZ :1;
        uint8_t activeACorDC:1;
        
    };

    struct R8_FreefallThreshold:public Reg8<>{
        uint8_t data;
    };

    struct R8_FreefallTime:public Reg8<>{
        uint8_t data;
    };

    struct R8_AxisControlForSingle2DoubleTap:public Reg8<>{
        
        uint8_t tapZenabled :1;
        uint8_t tapYenabled :1;
        uint8_t tapXenabled :1;
        uint8_t doubleCheck :1;
        uint8_t __resv__ :4;
        
    };

    struct R8_SourceOfSingle2DoubleTap:public Reg8<>{
        
        uint8_t tapZSource :1;
        uint8_t tapYSource :1;
        uint8_t tapXSource :1;
        uint8_t aSleep:1;
        uint8_t actZsource :1;
        uint8_t actYsource :1;
        uint8_t actXsource :1;
        uint8_t __resv__:1;
        
    };

    struct R8_DataRate:public Reg8<>{
        
        uint8_t dataRate :4;
        uint8_t lowPower :1;
        uint8_t __resv__ :3;
        
    };

    struct R8_PowerSaving:public Reg8<>{
        
        uint8_t wakeupFreq :2;
        uint8_t sleep :1;
        uint8_t measure:1;
        uint8_t autoSleep:1;
        uint8_t link:1;
        
    };

    struct R8_InterruptEnabler:public Reg8<>{
        
        uint8_t overrun :1;
        uint8_t watermark :1;
        uint8_t freefall :1;
        uint8_t Inactivity:1;
        uint8_t Activity:1;
        uint8_t doubleTap:1;
        uint8_t SingleTap:1;
        uint8_t dataReady:1;
        
    };

    struct R8_InterruptMap:public Reg8<>{
        
        uint8_t overrun :1;
        uint8_t watermark :1;
        uint8_t freefall :1;
        uint8_t Inactivity:1;
        uint8_t Activity:1;
        uint8_t doubleTap:1;
        uint8_t SingleTap:1;
        uint8_t dataReady:1;
        
    };

    struct R8_SourceOfInterrupt:public Reg8<>{
        
        uint8_t overrun :1;
        uint8_t watermark :1;
        uint8_t freefall :1;
        uint8_t Inactivity:1;
        uint8_t Activity:1;
        uint8_t doubleTap:1;
        uint8_t SingleTap:1;
        uint8_t dataReady:1;
        
    };

    struct R8_DataFormat:public Reg8<>{
        
        uint8_t range :2;
        uint8_t justify:1;
        uint8_t fullResolution:1;
        uint8_t __resv__ :1;
        uint8_t intInvert :1;
        uint8_t spiMode:1;
        uint8_t selfTest:1;
        
    };

    struct R8_DataX0:public Reg8<>{
        uint8_t data;
    };

    struct R8_DataX1:public Reg8<>{
        uint8_t data;
    };

    struct R8_DataY0:public Reg8<>{
        uint8_t data;
    };

    struct R8_DataY1:public Reg8<>{
        uint8_t data;
    };

    struct R8_DataZ0:public Reg8<>{
        uint8_t data;
    };

    struct R8_DataZ1:public Reg8<>{
        uint8_t data;
    };

    struct R8_FifoCtrl:public Reg8<>{
        
        uint8_t samples:5;
        uint8_t trigger :1;
        uint8_t fifoMode:2;
        
    };

    struct R8_FifoStatus:public Reg8<>{
        
        uint8_t entires:6;
        uint8_t __resv__ :1;
        uint8_t fifoTrig:1;
        
    }DEF_R8(fifo_status_reg)

};

}