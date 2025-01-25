#pragma once

#include "./drivers/Encoder/Encoder.hpp"

namespace ymd::drivers{


class HallEncoderIntf:public EncoderIntf{
public:
    virtual int getSector() = 0;
    virtual int getSectorCount() = 0;

    void getLapPosition() final override{
        return real_t(getSector()) / getSectorCount();
    }
};

class SimulatedHall:public HallEncoderIntf{
protected:
    EncoderIntf & enc_;
    int sector_ = 0;
public:
    SimulatedHall(EncoderIntf & enc):enc_(enc){}
    void update(){
        enc_.update();
        const auto lap_pos = enc_.getLapPosition();
        sector_ = int(lap_pos * N);
    }

    void init(){
        enc_.init();
    }

    bool stable(){
        return enc_.stable();
    }
};

}