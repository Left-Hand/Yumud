#pragma once


class ADXL345:public Axis6{
public:
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

protected:

    enum class RegAddress:uint8_t{
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


    struct DeviceIDReg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct TapThresholdReg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct OffsetXReg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct OffsetYReg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct OffsetZReg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct TapDurationReg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct TapLatencyReg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct TapWindowReg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct ActivityThresholdReg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct InactivityThresholdReg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct InactivityTimeReg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct AxisEnablerReg:public Reg8{
        REG8_BEGIN
        uint8_t InactiveZ :1;
        uint8_t InactiveY :1;
        uint8_t InactiveX :1;
        uint8_t InactiveACorDC:1;
        uint8_t activeX :1;
        uint8_t activeY :1;
        uint8_t activeZ :1;
        uint8_t activeACorDC:1;
        REG8_END
    };

    struct FreefallThresholdReg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct FreefallTimeReg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct AxisControlForSingle2DoubleTapReg:public Reg8{
        REG8_BEGIN
        uint8_t tapZenabled :1;
        uint8_t tapYenabled :1;
        uint8_t tapXenabled :1;
        uint8_t doubleCheck :1;
        uint8_t __resv__ :4;
        REG8_END
    };

    struct SourceOfSingle2DoubleTapReg:public Reg8{
        REG8_BEGIN
        uint8_t tapZSource :1;
        uint8_t tapYSource :1;
        uint8_t tapXSource :1;
        uint8_t aSleep:1;
        uint8_t actZsource :1;
        uint8_t actYsource :1;
        uint8_t actXsource :1;
        uint8_t __resv__:1;
        REG8_END
    };

    struct DataRateReg:public Reg8{
        REG8_BEGIN
        uint8_t dataRate :4;
        uint8_t lowPower :1;
        uint8_t __resv__ :3;
        REG8_END
    };

    struct PowerSavingReg:public Reg8{
        REG8_BEGIN
        uint8_t wakeupFreq :2;
        uint8_t sleep :1;
        uint8_t measure:1;
        uint8_t autoSleep:1;
        uint8_t link:1;
        REG8_END
    };

    struct InterruptEnablerReg:public Reg8{
        REG8_BEGIN
        uint8_t overrun :1;
        uint8_t watermark :1;
        uint8_t freefall :1;
        uint8_t Inactivity:1;
        uint8_t Activity:1;
        uint8_t doubleTap:1;
        uint8_t SingleTap:1;
        uint8_t dataReady:1;
        REG8_END
    };

    struct InterruptMapReg:public Reg8{
        REG8_BEGIN
        uint8_t overrun :1;
        uint8_t watermark :1;
        uint8_t freefall :1;
        uint8_t Inactivity:1;
        uint8_t Activity:1;
        uint8_t doubleTap:1;
        uint8_t SingleTap:1;
        uint8_t dataReady:1;
        REG8_END
    };

    struct SourceOfInterruptReg:public Reg8{
        REG8_BEGIN
        uint8_t overrun :1;
        uint8_t watermark :1;
        uint8_t freefall :1;
        uint8_t Inactivity:1;
        uint8_t Activity:1;
        uint8_t doubleTap:1;
        uint8_t SingleTap:1;
        uint8_t dataReady:1;
        REG8_END
    };

    struct DataFormatReg:public Reg8{
        REG8_BEGIN
        uint8_t range :2;
        uint8_t justify:1;
        uint8_t fullResolution:1;
        uint8_t __resv__ :1;
        uint8_t intInvert :1;
        uint8_t spiMode:1;
        uint8_t selfTest:1;
        REG8_END
    };

    struct DataX0Reg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct DataX1Reg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct DataY0Reg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct DataY1Reg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct DataZ0Reg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct DataZ1Reg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct FifoCtrlReg:public Reg8{
        REG8_BEGIN
        uint8_t samples:5;
        uint8_t trigger :1;
        uint8_t fifoMode:2;
        REG8_END
    };

    struct FifoStatusReg:public Reg8{
        REG8_BEGIN
        uint8_t entires:6;
        uint8_t __resv__ :1;
        uint8_t fifoTrig:1;
        REG8_END
    };

    struct{
        DeviceIDReg deviceIDReg;
        TapThresholdReg tapThresholdReg;
        OffsetXReg offsetXReg;
        OffsetYReg offsetYReg;
        OffsetZReg offsetZReg;
        TapDurationReg tapDurationReg;
        TapLatencyReg tapLatencyReg;
        TapWindowReg tapWindowReg;
        ActivityThresholdReg activityThresholdReg;
        InactivityThresholdReg inactivityThresholdReg;
        InactivityTimeReg inactivityTimeReg;
        AxisEnablerReg axisEnablerReg;
        FreefallThresholdReg freefallThresholdReg;
        FreefallTimeReg freefallTimeReg;
        AxisControlForSingle2DoubleTapReg axisControlForSingle2DoubleTapReg;
        SourceOfSingle2DoubleTapReg sourceOfSingle2DoubleTapReg;
        DataRateReg dataRateReg;
        PowerSavingReg powerSavingReg;
        InterruptEnablerReg interruptEnablerReg;
        InterruptMapReg interruptMapReg;
        SourceOfInterruptReg sourceOfInterruptReg;
        DataFormatReg dataFormatReg;
        DataX0Reg dataX0Reg;
        DataX1Reg dataX1Reg;
        DataY0Reg dataY0Reg;
        DataY1Reg dataY1Reg;
        DataZ0Reg dataZ0Reg;
        DataZ1Reg dataZ1Reg;
        FifoCtrlReg fifoCtrlReg;
        FifoStatusReg fifoStatusReg;
    };

protected:
    BusDrv & bus_drv;


    void writeReg(const RegAddress & reg_address, const Reg8 & reg_data){
        bus_drv.write({(uint8_t)((uint8_t)reg_address & 0x7F), *(uint8_t *)&reg_data});
    }

    void readReg(const RegAddress & reg_address, Reg8 & reg_data){
        bus_drv.write((uint8_t)((uint8_t)reg_address | 0x80), false);
        uint8_t temp = 0;
        bus_drv.read(temp);
        uint8_t * reg_ptr = (uint8_t *)&reg_data;
        *reg_ptr = temp;
    }
public:

    ADXL345(BusDrv & _bus_drv): bus_drv(_bus_drv){;}
    uint8_t getDeviceID(){
        readReg(RegAddress::DeviceID, deviceIDReg);
        return deviceIDReg.data;
    }

    void getAccel(real_t & x, real_t & y, real_t & z) override;
    void getGyro(real_t & x, real_t & y, real_t & z) override;

};
