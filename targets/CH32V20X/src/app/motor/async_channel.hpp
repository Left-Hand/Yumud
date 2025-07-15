#pragma once

#include "core/polymorphism/proxy.hpp"

namespace ymd::async{
namespace details{
PRO_DEF_MEM_DISPATCH(_Memfunc_Push, push);
PRO_DEF_MEM_DISPATCH(_Memfunc_Pending, pending);
PRO_DEF_MEM_DISPATCH(_Memfunc_Freeleft, freeleft);

PRO_DEF_MEM_DISPATCH(_Memfunc_Pop, pop);
PRO_DEF_MEM_DISPATCH(_Memfunc_Available, available);

}

// template<typename T>
// struct SinkFacade : pro::facade_builder
//     ::add_convention<details::_Memfunc_Push, void()>
//     ::add_convention<details::_Memfunc_Pending, size_t(void) const>
//     ::add_convention<details::_Memfunc_Freeleft, size_t(void) const>
//     ::build {};
    
// struct SourceFacade : pro::facade_builder
//     ::add_convention<details::_Memfunc_ReadChar, void(char &)>
//     ::add_convention<details::_Memfunc_ReadChars, void(char *, size_t)>
//     ::add_convention<details::_Memfunc_Available, size_t(void) const>
//     ::build {};

}