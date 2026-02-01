#pragma once

#include "opa.hpp"


namespace ymd::hal{


#ifdef OPA1_PRESENT
extern Opa<1> opa1;
#endif

#ifdef OPA2_PRESENT
extern Opa<2> opa2;
#endif
}