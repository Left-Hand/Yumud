#pragma once

#include "sys/polymorphism/proxy.hpp"

namespace ymd{
PRO_DEF_MEM_DISPATCH(_Memfunc_WriteChar, write1);
PRO_DEF_MEM_DISPATCH(_Memfunc_WriteChars, writeN);
PRO_DEF_MEM_DISPATCH(_Memfunc_Pending, pending);

PRO_DEF_MEM_DISPATCH(_Memfunc_ReadChar, read1);
PRO_DEF_MEM_DISPATCH(_Memfunc_ReadChars, readN);
PRO_DEF_MEM_DISPATCH(_Memfunc_Available, available);

struct WriteCharTraits : pro::facade_builder
    ::support_copy<pro::constraint_level::trivial>
    ::add_convention<_Memfunc_WriteChar, void(const char)>
    ::add_convention<_Memfunc_WriteChars, void(const char *, size_t)>
    ::add_convention<_Memfunc_Pending, size_t(void) const>
    ::build {};
    
struct ReadCharTraits : pro::facade_builder
    ::support_copy<pro::constraint_level::trivial>
    ::add_convention<_Memfunc_ReadChar, void(const char)>
    ::add_convention<_Memfunc_ReadChars, void(const char *, size_t)>
    ::add_convention<_Memfunc_Available, size_t(void) const>
    ::build {};


using WriteCharProxy = pro::proxy<WriteCharTraits>;
using ReadCharProxy = pro::proxy<ReadCharTraits>;

}