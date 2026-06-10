#include <string>
#include <cstddef>
#include <tuple>
#include <source_location>

struct AnyType
{
	template <typename T>
	operator T() {};
};

template <typename T>
static consteval auto CountMember(auto&&... Args) -> std::size_t
{
	if constexpr (!requires { T{ Args... }; })
	{
		return sizeof...(Args) - 1;
	}
	else
	{
		return CountMember<T>(Args..., AnyType{});
	}
}



template <typename Object_Type>
struct StaticWarp
{
	inline static std::decay_t<Object_Type> value; // NOLINT
};

template <typename Object_Type>
static constexpr auto GetTuple(Object_Type&& Object) -> auto // NOLINT
{
	constexpr auto field_num{ CountMember<std::decay_t<Object_Type>>() };

	if constexpr (field_num == 0)
	{
		return std::tuple<>{};
	}
	else if constexpr (field_num == 1)
	{
		auto&& [_1] = Object;
		return std::tuple{ &_1 };
	}
	else if constexpr (field_num == 2)
	{
		auto&& [_1, _2] = Object;
		return std::tuple{ &_1, &_2 };
	}
	else if constexpr (field_num == 3)
	{
		auto&& [_1, _2, _3] = Object;
		return std::tuple{ &_1, &_2, &_3 };
	}

}

template <auto... Vs>
static consteval auto GetFieldSigNameImp() -> std::string_view
{
#if __cpp_lib_source_location >= 201907L
	return std::source_location::current().function_name();
#elif defined(_MSC_VER)
	return __FUNCSIG__;
#else
	return __PRETTY_FUNCTION__;
#endif
}

template <class... Vs>
static consteval auto GetFieldSigNameImp() -> std::string_view
{
#if __cpp_lib_source_location >= 201907L
	return std::source_location::current().function_name();
#elif defined(_MSC_VER)
	return __FUNCSIG__;
#else
	return __PRETTY_FUNCTION__;
#endif
}

template <typename Object_Type, std::size_t IDX>
static consteval auto GetFieldSigName() -> std::string_view
{
	return GetFieldSigNameImp<std::get<IDX>(GetTuple(StaticWarp<Object_Type>::value))>();
}

template <typename Object_Type, std::size_t IDX>
static consteval auto GetFieldName() -> std::string_view
{
	constexpr auto symbol_name = GetFieldSigName<Object_Type, IDX>();
#if __clang__
	constexpr auto field_name_beg{ symbol_name.find("[Vs = <&value.") + 14 };
	constexpr auto field_name_end{ symbol_name.find(">]", field_name_beg) };
	constexpr auto filed_name_size{ field_name_end - field_name_beg };
	return symbol_name.substr(field_name_beg, filed_name_size);
#elif __GNUC__
	constexpr auto field_name_beg{ symbol_name.find("::", symbol_name.find("&>::value.") + 11) + 2 };
	constexpr auto field_name_end{ symbol_name.find(")]") };
	constexpr auto filed_name_size{ field_name_end - field_name_beg };
	return symbol_name.substr(field_name_beg, filed_name_size);
#elif _MSC_VER
	constexpr auto field_name_beg{ symbol_name.find("&value->") + 8 };
	constexpr auto field_name_end{ symbol_name.find('>', field_name_beg) };
	constexpr auto filed_name_size{ field_name_end - field_name_beg };
	return symbol_name.substr(field_name_beg, filed_name_size);
#else
	static_assert(false, "GetFieldName(): unknown compiler.");
#endif
}

template <typename Object_Type>
static consteval auto GetFieldCount() -> std::size_t
{
	return std::tuple_size_v<decltype(GetTuple(StaticWarp<Object_Type>::value))>;
}


template <typename Object_Type, std::size_t IDX>
using GetFieldType = std::remove_pointer_t<std::decay_t<decltype(std::get<IDX>(GetTuple(StaticWarp<Object_Type>::value)))>>;

template <typename Object_Type>
static constexpr auto FotEach(Object_Type&& Object, auto&& Functor) -> void // NOLINT
{
	auto tuple = GetTuple(Object);
	[&Functor, &tuple] <auto... Ns>(std::index_sequence<Ns...>) { (Functor(GetFieldName<Object_Type, Ns>(), *std::get<Ns>(tuple)), ...); }(std::make_index_sequence<GetFieldCount<Object_Type>()>{});
}
