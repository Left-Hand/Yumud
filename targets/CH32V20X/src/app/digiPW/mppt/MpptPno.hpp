#pragma once

#include "sys/math/real.hpp"

namespace ymd::digipw{

class MpptPno{
protected:
	real_t  Ipv;
	real_t  Vpv;
	real_t  DeltaPmin;
	real_t  MaxVolt;
	real_t  MinVolt;
	real_t  Stepsize;
	real_t  VmppOut;
	real_t  DeltaP;
	real_t  PanelPower;
	real_t  PanelPower_Prev;
	bool inited;
public:
    MpptPno();

    MpptPno(const MpptPno & other) = delete;
    MpptPno(MpptPno && other) = default;

    void reset();
    void update();
};

}