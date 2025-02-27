#include "opa.hpp"

namespace ymd::hal{

#ifdef ENABLE_OPA1
Opa_t<1> opa1 = {};
#endif

#ifdef ENABLE_OPA2
Opa_t<2> opa2 = {};
#endif


}