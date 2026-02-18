#pragma once

namespace ymd::tmp{
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
	static constexpr auto type_name=[&]<std::size_t...indices>(std::integer_sequence<std::size_t,indices...>)
	{
		constexpr auto str = tmp::static_string<type_name_view[indices]...,'\0'>();
		return str;
	}(indices);
	return std::string_view(type_name);
}

}