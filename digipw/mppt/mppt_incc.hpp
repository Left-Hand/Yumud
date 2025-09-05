#pragma once

#include "core/math/real.hpp"

namespace ymd::digipw{

class MpptIncc{
public:
    MpptIncc(){
        reset();
    }

    MpptIncc(const MpptIncc & other) = delete;
    MpptIncc(MpptIncc && other) = default;

    void reset();
    void update();

private:
	real_t  Ipv = 0;
	real_t  Vpv = 0;
	real_t  IpvH = 0;
	real_t  IpvL = 0;
	real_t  VpvH = 0;
	real_t  VpvL = 0;
	real_t  MaxVolt = 0;
	real_t  MinVolt = 0;
	real_t  Stepsize = 0;
	real_t  VmppOut = 0;
	real_t  Cond = 0;
	real_t  IncCond = 0;
	real_t  DeltaV = 0;
	real_t  DeltaI = 0;
	real_t  VpvOld = 0;
	real_t  IpvOld = 0;
	real_t  StepFirst = 0;
	bool inited = false;
} ;

}