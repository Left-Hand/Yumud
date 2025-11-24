#pragma once

namespace ymd::tmp{

template<char...args>
struct static_string
{
    static constexpr const char str[]={args...};

    constexpr explicit operator std::string_view()const{
        return std::string_view(static_string::str, sizeof...(args));}
};
}