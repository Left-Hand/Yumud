#pragma once

#include "core/tmp/tmp_primitive.hpp"
#include "core/tmp/implfor.hpp"


namespace ymd{
template<typename T>
static constexpr bool is_not_implemented_v = requires { 
    T::NOT_IMPLEMENTED;
};

template<typename T>
static constexpr bool is_implemented_v = not is_not_implemented_v<T>;
}


namespace ymd::convert{
template<typename To>
struct TryInto{
    static constexpr bool NOT_IMPLEMENTED = true;
};

template<typename From>
struct TryFrom{
    static constexpr bool NOT_IMPLEMENTED = true;
};



template<typename To, typename From>
static constexpr auto try_cast(const From& from){
    if constexpr(is_implemented_v<ImplFor<TryFrom<To>, From>>)
        return ImplFor<TryFrom<From>, To>::try_from(from);
    else if constexpr(is_implemented_v<ImplFor<TryInto<To>, From>>)
        return ImplFor<TryInto<To>, From>::try_into(from);
    else
        static_assert(tmp::false_v<To>, "cant convert");
}

}

