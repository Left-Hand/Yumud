#pragma once

#include "core/polymorphism/proxy.hpp"

namespace ymd{

namespace details{
PRO_DEF_MEM_DISPATCH(_Memfunc_TryWriteChar, try_write_char);
PRO_DEF_MEM_DISPATCH(_Memfunc_TryWriteChars, try_write_chars);
PRO_DEF_MEM_DISPATCH(_Memfunc_FreeCapacity, free_capacity);

PRO_DEF_MEM_DISPATCH(_Memfunc_TryReadChar, try_read_char);
PRO_DEF_MEM_DISPATCH(_Memfunc_TryReadChars, try_read_chars);
PRO_DEF_MEM_DISPATCH(_Memfunc_Available, available);
}

struct WriteCharFacade : pro::facade_builder
    ::add_convention<details::_Memfunc_TryWriteChar, size_t(const char)>
    ::add_convention<details::_Memfunc_TryWriteChars, size_t(const char *, size_t)>
    ::add_convention<details::_Memfunc_FreeCapacity, size_t(void) const>
    ::build {};
    
struct ReadCharFacade : pro::facade_builder
    // ::support_copy<pro::constraint_level::trivial>
    ::add_convention<details::_Memfunc_TryReadChar, size_t(char &)>
    ::add_convention<details::_Memfunc_TryReadChars, size_t(char *, size_t)>
    ::add_convention<details::_Memfunc_Available, size_t(void) const>
    ::build {};


using WriteCharProxy = pro::proxy<WriteCharFacade>;
using ReadCharProxy = pro::proxy<ReadCharFacade>;

}