#include "iqt.hpp"

namespace std
{
    iq_t sin(const iq_t & iq) {
        return iq_t(_IQsin(iq.getValue()));
    }

    iq_t cos(const iq_t & iq) {
        return iq_t(_IQcos(iq.getValue()));
    }

    iq_t tan(const iq_t & iq) {
        return iq_t(_IQsin(iq.getValue()) / _IQcos(iq.getValue()));
    }

    iq_t asin(const iq_t & iq) {
        return iq_t(_IQasin(iq.getValue()));
    }

    iq_t acos(const iq_t & iq) {
        return iq_t(_IQacos(iq.getValue()));
    }

    iq_t atan(const iq_t & iq) {
        return iq_t(_IQatan(iq.getValue()));
    }

    iq_t atan2(const iq_t & a, const iq_t & b) {
        return iq_t(_IQatan2(a.getValue(),b.getValue()));
    }

    iq_t sqrt(const iq_t & iq) {
        return iq_t(_IQsqrt(iq.getValue()));
    }

    iq_t abs(const iq_t & iq) {
        return iq_t(_IQabs(iq.getValue()));
    }

    bool isnormal(const iq_t & iq){
        return bool(iq.getValue());
    }

    bool signbit(const iq_t & iq){
        return bool(iq.getValue() < 0);
    }

    iq_t fmod(const iq_t & a, const iq_t & b){
        return iq_t(_IQmpy(_IQfrac(_IQdiv(a.getValue(), b.getValue())), b.getValue()));
    }

    iq_t mean(const iq_t & a, const iq_t & b){
        return iq_t((a.getValue() + b.getValue()) >> 1);
    }

    iq_t frac(const iq_t & iq){
        return iq_t(_IQfrac(iq.getValue()));
    }

    iq_t floor(const iq_t & iq){
        return iq_t(iq.getValue() - _IQfrac(iq.getValue()));
    }

    #ifdef USE_LOG
    
    iq_t log10(const iq_t & iq) {
        return iq_t(_IQlog10(iq.getValue()));
    }

    iq_t log(const iq_t & iq) {
        static _iq lge = _IQlog10(_IQ(LOGE));
        return iq_t(_IQdiv(_IQlog10(iq.getValue()), lge));
    }

    #endif
};