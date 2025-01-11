#include "MpptIncc.hpp"

using namespace ymd::digipw;

void MpptIncc::reset(){
    this->Ipv=0;
    this->Vpv=0;
    this->IpvH=(1.65_r);
    this->IpvL=0;
    this->VpvH=0;
    this->VpvL=0;
    this->MaxVolt=0;
    this->MinVolt=0;
    this->Stepsize=0;
    this->VmppOut=0;
    this->Cond=0;
    this->IncCond=0;
    this->DeltaV=0;
    this->DeltaI=0;
    this->VpvOld=0;
    this->IpvOld=0;
    this->StepFirst=(0.02_r);
    this->inited=false;
}

void MpptIncc::update(){
    if (!this->inited)
    {
        this->VmppOut= this->Vpv - this->StepFirst;
        this->VpvOld=this->Vpv;
        this->IpvOld=this->Ipv;
        this->inited = true;
    }
    else
    {
        this->DeltaV = this->Vpv-this->VpvOld ; 				/* PV voltage change */
        this->DeltaI = this->Ipv-this->IpvOld ; 				/* PV current change */
//	   		if(this->DeltaV <this->VpvH && this->DeltaV >-this->VpvL )  /* Tolerence limits PV Voaltge Variation */
//	    	this->DeltaV= 0;
//	   		if(this->DeltaI <this->IpvH && this->DeltaI >-this->IpvL )  /* Tolerence limits PV current Variation */
//	    	this->DeltaI = 0;
        if(this->DeltaV == 0)    						/* DeltaV=0 */
        {
            if(this->DeltaI !=0 )    					/* dI=0 operating at MPP */
            {
                if ( this->DeltaI > 0 )  				/* climbing towards MPP */
                {this->VmppOut=this->Vpv+this->Stepsize;}		/* Increment Vref */
                else
                {this->VmppOut=this->Vpv-this->Stepsize;} 		/* Decrement Vref */
            }
        }
        else
        {
            this->Cond = this->Ipv / (this->Vpv);					/*Cond =-(this->Ipv<<10)/this->Vpv;*/
            this->IncCond = this->DeltaI / (this->DeltaV);			/*IncCond =(DeltaI<<10)/DeltaV;*/
            if (this->IncCond != this->Cond)				/* Operating at MPP */
            {
                if (this->IncCond > (-this->Cond))			/* left of MPP */
                {
                this->VmppOut=this->Vpv+this->Stepsize;			/* change Vref */
                }
                else								/* Right of MPP */
                {
                this->VmppOut=this->Vpv-this->Stepsize;			/* change Vref */
                }
            }
        }
        this->VpvOld = this->Vpv;
        this->IpvOld = this->Ipv;
    }
}