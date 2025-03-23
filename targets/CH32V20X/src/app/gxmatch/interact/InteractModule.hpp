#pragma once

#include "buzzer.hpp"
#include "keys.hpp"

namespace gxm{

class InteractModule{
protected:
public:
    void showTask();
    void changeTask();
    void abortTask();
};

}