#include "../tb.h"

// #include 
// https://taylorconor.com/blog/enum-reflection/

template<char...args>
struct __static_string
{
    static constexpr const char str[]={args...};
    
    constexpr
    operator const char*()const{return __static_string::str;}
};


template <typename E, E V>
constexpr auto _enum_item_name() {
	constexpr std::string_view fully_name = __PRETTY_FUNCTION__;

	constexpr std::size_t begin=[&](){
		for(std::size_t i=fully_name.size() - 1;i>1;i--){
            const char chr = fully_name[i];
			if(chr ==' ' || chr == ')')
				return i + 1;
        }
	}();
	constexpr std::size_t end=[&](){
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



// #define enum_of_item(item) decltype(item)

template <typename E, E V> 
constexpr const char * enum_item_name_v = _enum_item_name<E, V>();



template <typename E, E V> 
constexpr bool _enum_is_valid() {
	constexpr std::string_view fully_name = __PRETTY_FUNCTION__;

	constexpr std::size_t begin=[&](){
		for(std::size_t i=fully_name.size() - 1;i>1;i--){
            const char chr = fully_name[i];
			if(chr ==' ' || chr == ')')
				return i + 1;
        }
	}();
	constexpr std::size_t end=[&](){
		for(std::size_t i=0;i<fully_name.size();i++)
			if(fully_name[i]==']')
				return i;
	}();
	
	constexpr auto name=fully_name.substr(begin,end-begin);
    auto is_digit = [](char c) {
        return c >= '0' && c <= '9';
    };

    for(size_t i = 0; i < name.length(); i++){
        if(!is_digit(name[i])) return true;
    }
    return false;
}

// template <E V> 
// constexpr const char * enum_item_name_v2 = _enum_item_name<decltype(V), V>();

template <typename E, E V> 
constexpr bool enum_is_valid_v = _enum_is_valid<E, V>();


template <typename E> 
consteval int _enum_count_valid() {
    return 0;
}
  
template <typename E, E A, E... B> 
consteval int _enum_count_valid() {
    bool is_valid = _enum_is_valid<E, A>();
    return _enum_count_valid<E, B...>() + (int)is_valid;
}

template <typename E, int... I> 
consteval int _enum_internal_element_count(std::integer_sequence<int, I...> unused) {
    return _enum_count_valid<E, (E)I...>();
}


template <typename E>
constexpr int enum_count_v = _enum_internal_element_count<E>(std::make_integer_sequence<int, 100>());


// template<typename E, auto...args>
// struct __static_string_pool{
//     using Key = E;
//     using Value = const char *;

//     using Pair = std::pair<Key, Value>;

//     constexpr size_t N = sizeof...(args); 
//     static constexpr std::array<Pair> pool={static_cast<const char *>(args)...};
    
//     constexpr Value
//     operator [](const size_t idx) const{return pool[idx];}
// };


// template <typename E, size_t N = enum_count_v<E>>
// consteval auto make_string_pool(){

// }
// template <E V>
// using enum_type_t = typename E;



enum Fruit {
    BANANA = 5,
    APPLE = 12,
};
  
void enum_main(){
    [[maybe_unused]] constexpr const char * banana = enum_item_name_v<Fruit, Fruit::BANANA>;
    [[maybe_unused]] constexpr const char * _10 = enum_item_name_v<Fruit, Fruit(10)>;

    constexpr size_t count = enum_count_v<Fruit>;
    // constexpr const char * banana2 = enum_item_name_v2<Fruit::BANANA>;
    // using type = decltype(Fruit::BANANA);
    // constexpr const char * banana = enum_item_name_v<Fruit, Fruit::BANANA>;
 
    // static_assert(!, "!enum_is_valid_v<Fruit, Fruit::10>()");
    // const auto f = __PRETTY_FUNCTION__;
    static_assert(enum_is_valid_v<Fruit, Fruit::BANANA>, "enum_is not _valid_v<Fruit, Fruit::BANANA>()");
    static_assert(!enum_is_valid_v<Fruit, Fruit(10)>, "enum_is_valid_v<Fruit, Fruit::10>()");
    // static_assert(!enum_is_valid_v<Fruit, Fruit>, "!enum_is_valid_v<Fruit, Fruit::10>()");
}