#pragma once

#include "coil.hpp"


namespace ymd::drivers{
class Coil2PConcept: public CoilConcept{
public:
    virtual Coil2PConcept& operator= (const real_t duty) = 0;
};

}