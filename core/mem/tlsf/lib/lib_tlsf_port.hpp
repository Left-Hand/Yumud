#pragma once

namespace lib_tlsf {

/*
** Set assert macro, if it has not been provided by the user.
*/
#if 0

#if !defined (tlsf_assert)
#define tlsf_assert assert
#endif
#else
#define tlsf_assert(x) ((void)0)
#endif

#define printf(...) ((void)0)

}