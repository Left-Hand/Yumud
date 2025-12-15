#pragma once

#include "core/math/real.hpp"

namespace ymd::digipw{

template<typename T>
struct MpptPno;


template<>
struct [[nodiscard]] MpptPno<iq16> final{
public:
    constexpr MpptPno();

    MpptPno(const MpptPno & other) = delete;
    MpptPno(MpptPno && other) = default;

    constexpr void reset(){
		this->Ipv=0;
		this->Vpv=0;
		this->DeltaPmin=(0.00001_r);
		this->MaxVolt=(0.9_r);
		this->MinVolt=0;
		this->Stepsize=(0.002_r);
		this->VmppOut=0;
		this->DeltaP=0;
		this->PanelPower=0;
		this->PanelPower_Prev=0;
		this->inited = false;
	}
    constexpr void update(){
		if (!this->inited){
			this->VmppOut= this->Vpv - (0.02_r);
			this->PanelPower_Prev=this->PanelPower;
			this->inited = true;
		}else{
			this->PanelPower=(this->Vpv*this->Ipv);
			this->DeltaP=this->PanelPower-this->PanelPower_Prev;
			if (this->DeltaP > this->DeltaPmin){
				this->VmppOut=this->Vpv+this->Stepsize;
			}else{
				if (this->DeltaP < -this->DeltaPmin){
					this->Stepsize=-this->Stepsize;
					this->VmppOut=this->Vpv+this->Stepsize;
				}
			}
			this->PanelPower_Prev = this->PanelPower;
		}
		if(this->VmppOut < this->MinVolt) this->VmppOut = this->MinVolt;
		if(this->VmppOut > this->MaxVolt) this->VmppOut = this->MaxVolt;
	}

	[[nodiscard]] auto output() const {return this->VmppOut;}

private:
	iq16  Ipv;
	iq16  Vpv;
	iq16  DeltaPmin;
	iq16  MaxVolt;
	iq16  MinVolt;
	iq16  Stepsize;
	iq16  VmppOut;
	iq16  DeltaP;
	iq16  PanelPower;
	iq16  PanelPower_Prev;
	bool inited;
};

}