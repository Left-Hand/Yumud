#pragma once

#include "rigid.hpp"

#include "core/stream/stream.hpp"
#include "core/stream/StringStream.hpp"
#include "robots/repl/repl_thread.hpp"

namespace nudec::_2023E{



class GimbalThread{
};

class GimbalThreadForMaster final: public GimbalThread{
public:
    void process(const real_t t){

    }
private:

};

class GimbalThreadForSlave final: public GimbalThread{
public:
    void process(const real_t t){

    }
private:

};


}