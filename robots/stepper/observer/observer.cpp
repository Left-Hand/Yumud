#include "observer.hpp"
#include "../stepper.hpp"


ShutdownFlag & ShutdownFlag::operator = (const bool _state){
    state = _state;

    if(state) m_stp.shutdown();
    else m_stp.wakeup();

    return *this;
}
