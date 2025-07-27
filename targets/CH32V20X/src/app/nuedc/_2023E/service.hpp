#pragma once

#include "rigid.hpp"

#include "core/stream/stream.hpp"

namespace nuedc::_2023E{



class GimbalService{
};

class GimbalServiceForMaster final: public GimbalService{
public:
    void process(const real_t t){

    }
private:

};

class GimbalServiceForSlave final: public GimbalService{
public:
    void process(const real_t t){

    }
private:

};


}