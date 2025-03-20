#pragma once

#include "core/math/uint24_t.hpp"
#include <span>

namespace ymd{


template<typename T>
struct RegBase{
    using data_type = T;

    T data;
};

namespace details{

    // template<typename T>
// struct reg_data_type{};

    template<typename T>
    struct _reg_data_type{};
    
    template<typename T>
    requires std::is_integral_v<T>
    struct _reg_data_type<T>{using type = std::decay<T>;};
    
    template<typename T>
    requires std::is_base_of_v<RegBase, T>
    struct _reg_data_type<T>{using type = T::data_type;};
    
    template<typename T>
    struct _reg_data_type<RegBase<T>>{using type = T::data_type;};
}


template<typename T>
using reg_data_type_t = typename details::_reg_data_type<T>::type;

}
