#pragma once

#include "sumtype.hpp"

#define DEF_ERROR_WITH_KIND(name, kind)\
class name:public Sumtype<kind>{\
public:\
    using Kind = kind;\
    using Super = Sumtype<Kind>;\
    using Super::Super;\
    using enum Kind;\
};\


#define DEF_ERROR_WITH_KINDS(name, ...)\
class name:public Sumtype<__VA_ARGS__>{\
public:\
    using Super = Sumtype<__VA_ARGS__>;\
    using Super::Super;\
};\

#define DEF_ERROR_SUMWITH_HALERROR(name, kind)\
class name:public Sumtype<kind, hal::HalError>{\
public:\
    using Kind = kind;\
    using Super = Sumtype<Kind, hal::HalError>;\
    using Super::Super;\
    using enum Kind;\
};\


