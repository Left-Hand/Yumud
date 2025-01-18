#pragma once

namespace ymd::foc{

class SensorlessObserverIntf{
public:
    virtual void reset() = 0;
    virtual void update(iq_t Valpha, iq_t Vbeta, iq_t Ialpha, iq_t Ibeta) = 0;
    virtual iq_t theta() const = 0;
};

}