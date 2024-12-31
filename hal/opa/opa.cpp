#include "opa.hpp"

namespace ymd{

#ifdef ENABLE_OPA1
Opa opa1 = {OPA1};
#endif

#ifdef ENABLE_OPA2
Opa opa2 = {OPA2};
#endif


}