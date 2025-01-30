#pragma once

#include "sys/math/real.hpp"

namespace ymd::foc{

// scexpr uint chopper_freq = 32768;
scexpr uint chopper_freq = 50000;

using Current = real_t;

struct UvwValue{
    #pragma pack(push, 1)
    real_t u, v, w;
    #pragma pack(pop)

    real_t operator [](const size_t idx) const {
        return *(&u + idx);
    }

    real_t & operator [](const size_t idx){
        return *(&u + idx);
    }
};

struct UvwCurrent: public UvwValue{};
struct UvwVoltage: public UvwValue{};


struct DqValue{
    #pragma pack(push, 1)
    real_t d, q;
    #pragma pack(pop)

    real_t operator [](const size_t idx) const {
        return *(&d + idx);
    }

    real_t & operator [](const size_t idx){
        return *(&d + idx);
    }
};

struct DqCurrent: public DqValue{};
struct DqVoltage: public DqValue{};

struct AbValue{
    #pragma pack(push, 1)
    real_t a, b;
    #pragma pack(pop)

    real_t operator [](const size_t idx) const {
        return *(&a + idx);
    }

    real_t & operator [](const size_t idx){
        return *(&a + idx);
    }
};

struct AbCurrent: public AbValue{};
struct AbVoltage: public AbValue{};


__inline AbCurrent uvw_to_ab(const UvwCurrent & uvw){
    return {(uvw.u - ((uvw.v + uvw.w) >> 1)) * real_t(2.0/3), (uvw.w - uvw.v) * real_t(1.731 / 3)};
};

DqCurrent ab_to_dq(const AbCurrent & ab, const real_t rad);
DqVoltage ab_to_dq(const AbVoltage & ab, const real_t rad);

AbCurrent dq_to_ab(const DqCurrent & dq, const real_t rad);
AbVoltage dq_to_ab(const DqVoltage & dq, const real_t rad);

void init_adc();
static __inline real_t sign_sqrt(const real_t x){
    return x < 0 ? -sqrt(-x) : sqrt(x);
};

static __inline real_t smooth(const real_t x){
    return x - sin(x);
}


}



namespace ymd::rpc{
template<char...args>
struct __static_string
{
    static constexpr const char str[]={args...};
    operator const char*()const{return __static_string::str;}
};

template<typename Object>
constexpr auto type_name_of(){


	constexpr std::string_view fully_name=__PRETTY_FUNCTION__;		
	constexpr std::size_t begin=[&]()
	{
		for(std::size_t i=0;i<fully_name.size();i++)
			if(fully_name[i]=='=')
				return i+2;
	}();
	constexpr std::size_t end=[&]()
	{
		for(std::size_t i=0;i<fully_name.size();i++)
			if(fully_name[i]==']')
				return i;
	}();
	constexpr auto type_name_view=fully_name.substr(begin,end-begin);
	constexpr auto indices=std::make_index_sequence<type_name_view.size()>();
	constexpr auto type_name=[&]<std::size_t...indices>(std::integer_sequence<std::size_t,indices...>)
	{
		constexpr auto str=__static_string<type_name_view[indices]...,'\0'>();
		return str;
	}(indices);
	return type_name;
}
}