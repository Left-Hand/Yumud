#include "observer.hpp"
#include "../stepper.hpp"


using namespace ymd::foc;


void InverseObserver::count(){

}


void OverrunObserver::count(){

}

void OverTempObserver::count(){

}

void StallOberserver::count(){

}


// class FOCStepper;

// struct ShutdownFlag{
// protected:
//     FOCStepper & m_stp;
//     bool state = false;

// public:
//     ShutdownFlag(FOCStepper & stp):m_stp(stp){;}

//     ShutdownFlag & operator = (const bool _state);
//     operator bool() const{
//         return state;
//     }
// };
