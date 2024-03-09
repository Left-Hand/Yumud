#ifndef __GPIO_ENUMS_H__

#define __GPIO_ENUMS_H__

enum Pin{
    PinNone,
    Pin0 = 1,
    Pin1 = Pin0 << 1,
    Pin2 = Pin1 << 1,
    Pin3 = Pin2 << 1,
    Pin4 = Pin3 << 1,
    Pin5 = Pin4 << 1,
    Pin6 = Pin5 << 1,
    Pin7 = Pin6 << 1,
    Pin8 = Pin7 << 1,
    Pin9 = Pin8 << 1,
    Pin10 = Pin9 << 1,
    Pin11 = Pin10 << 1,
    Pin12 = Pin11 << 1,
    Pin13 = Pin12 << 1,
    Pin14 = Pin13 << 1,
    Pin15 = Pin14 << 1
};

enum PinMode{
    InAnalog = 0b0000,
    InFloating = 0b0100,
    InPullUP = 0b1000,
    InPullDN = 0b1100,
    OutPP = 0b0011,
    OutOD = 0b0111,
    OutAfPP = 0b1011,
    OutAfOD = 0b1111,
};
#endif