#ifndef __TM8211_HPP__

#define __TM8211_HPP__

#include "../bus/bus_inc.h"
#include "../types/real.hpp"

class TM8211{
private:
    I2sDrv busdrv;

    int16_t ldata;
    int16_t rdata;
    uint32_t distort_mask = 0xFFFFFFFF;
    real_t voltH;
    real_t voltL;
    real_t voltComm;
    real_t voltDiff_2;

    __fast_inline int16_t VoltageToData(real_t volt){
        volt = CLAMP(volt, voltL, voltH);
        real_t k = ((volt - voltComm) / voltDiff_2);
        return (int16_t)(k * 0x7FFF);
    }

    __fast_inline int16_t DutyToData(real_t duty){
        duty = CLAMP(duty, real_t(0), real_t(1));
        real_t k = (duty - 0.5) / 0.5;
        return (int16_t)(k * 0x7FFF);
    }
public:
    TM8211(I2sDrv & _busdrv):busdrv(_busdrv){
        setRail(real_t(3.3 * 0.25f), real_t(3.3 * 0.75f)); 
    }

    void setRail(const real_t & _voltL, const real_t & _voltH){
        voltL = _voltL;
        voltH = _voltH;
        voltComm = (voltL + voltH) / 2;
        voltDiff_2 = (voltH - voltL) / 2;
    }

    void write(const uint32_t & data){
        busdrv.write(data);
    }

    void setChData(const uint8_t & index,const uint16_t & data){
        if(index) rdata = data;
        else ldata = data;

        write(((ldata << 16) | rdata) & distort_mask);
    }

    void setChVoltage(const uint8_t & index, const real_t & volt){
        setChData(index, VoltageToData(volt));
    }

    void setDistort(uint8_t level){
        uint16_t mask_16 = ~((1 << level) - 1);
        distort_mask = (mask_16 << 16) | mask_16;
    }
    void setVoltage(const real_t & lvolt, const real_t & rvolt){
        ldata = VoltageToData(lvolt);
        rdata = VoltageToData(rvolt);

        write(((ldata << 16) | rdata) & distort_mask);
    }

    // void setChDuty(const real_t & duty){

    // }
};
#endif