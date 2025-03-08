#pragma once

namespace ymd::foc{

class SensorlessObserverIntf{
public:
    virtual ~SensorlessObserverIntf() = default;
    virtual void reset() = 0;
    virtual void update(iq_t<16> Valpha, iq_t<16> Vbeta, iq_t<16> Ialpha, iq_t<16> Ibeta) = 0;
    virtual iq_t<16> theta() const = 0;
};

}