#pragma once

#include <cstdint>

struct R16_AWU_CSR{
    uint16_t :1;
    uint16_t AWUEN:1;
    uint16_t :14;
};

struct R16_AWU_WR{
    uint16_t APR:6;
    uint16_t :9;
};

struct R16_AWU_PSC{
    uint16_t TBR:4;
    uint16_t :12;
};


struct AWU_Def{
    volatile R16_AWU_CSR CSR;
    volatile R16_AWU_WR WR;
    volatile R16_AWU_PSC PSC;

    void enable(const Enable en){
        CSR.AWUEN = en;
    }

    void set_apr(const uint8_t apr){
        WR.APR = apr;
    }

    // 0000：不分频；
    // 0001：不分频；
    // 0010：2 分频；
    // 0011：4 分频；
    // 0100：8 分频；
    // 0101：16 分频；
    // 0110：32 分频；
    // 0111：64 分频；
    // 1000：128 分频；
    // 1001：256 分频；
    // 1010：512 分频；
    // 1011：1024 分频；
    // 1100：2048 分频；
    // 1101：4096 分频；
    // 1110：10240 分频；
    // 1111：61440 分频。
    void set_prescale(const uint8_t prescale){
        PSC.TBR = [&]->uint8_t{
            switch(prescale){
                case 1: return 0b0000;
                case 2: return 0b0001;
                case 4: return 0b0011;
                case 8: return 0b0100;
                case 16: return 0b0101;
                case 32: return 0b0110;
                case 64: return 0b0111;
                case 128: return 0b1000;
                case 256: return 0b1001;
                case 512: return 0b1010;
                case 1024: return 0b1011;
                case 2048: return 0b1100;
                case 4096: return 0b1101;
                case 10240: return 0b1110;
                case 61440: return 0b11111;
            }
        }();
    }
};