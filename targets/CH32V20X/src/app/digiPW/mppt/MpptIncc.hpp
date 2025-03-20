#pragma once

#include "core/math/real.hpp"

namespace ymd::digipw{

class MpptIncc{
protected:
	real_t  Ipv;
	real_t  Vpv;
	real_t  IpvH;
	real_t  IpvL;
	real_t  VpvH;
	real_t  VpvL;
	real_t  MaxVolt;
	real_t  MinVolt;
	real_t  Stepsize;
	real_t  VmppOut;
	real_t  Cond;
	real_t  IncCond;
	real_t  DeltaV;
	real_t  DeltaI;
	real_t  VpvOld;
	real_t  IpvOld;
	real_t  StepFirst;
	bool inited;
public:
    MpptIncc(){
        reset();
    }

    MpptIncc(const MpptIncc & other) = delete;
    MpptIncc(MpptIncc && other) = default;

    void reset();
    void update();
} ;

}