#pragma once

#include "rigid.hpp"

namespace nuedc::_2023E{



class GimbalService{
};

class GimbalServiceForMaster final: public GimbalService{
public:
    void process(const iq16 t){

    }
private:

};

class GimbalServiceForSlave final: public GimbalService{
public:
    void process(const iq16 t){

    }
private:

};


}