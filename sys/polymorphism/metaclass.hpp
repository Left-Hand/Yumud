#include <type_traits>
#include <typeinfo>
#include <exception>
#include <algorithm>
#include <unordered_map>
#include <typeindex>
#include <shared_mutex>
enum class OwnerShip
{
	Owner,
	Observer
};
struct is_v_ptr {};
struct owner {};
struct observer {};
static const std::size_t SSOBufferSize = sizeof(void*);
template < typename T >
concept has_v_ptr = std::is_base_of_v <is_v_ptr, std::remove_cvref_t<T>>;
template <typename T>
concept is_owner = std::is_base_of_v <owner, std::remove_cvref_t<T>>;
template <typename T>
concept is_observer = std::is_base_of_v <observer, std::remove_cvref_t<T>>;
template <typename T>
concept is_interface = std::is_base_of_v <owner, std::remove_cvref_t<T>> || std::is_base_of_v <observer, std::remove_cvref_t<T>>;
template<std::size_t N>
struct StringLiteral
{
    constexpr StringLiteral(const char(&str)[N])
    {
        std::copy_n(str, N, value);
    }
    char value[N];
};

#define PRIMITIVE_CAT(x, y) x ## y
#define CAT(x, y) PRIMITIVE_CAT(x, y)

#define GET_SEC(x, n, ...) n
#define ATTER(atter) atter
#define CHECK(...) ATTER(GET_SEC(__VA_ARGS__,0))
#define PROBE(x) x, 1

#define IS_EMPTY(x, ...) CHECK(CAT(PRIMITIVE_CAT(IS_EMPTY_, x), 0))
#define IS_EMPTY_0 PROBE()

#define EMPTY()
#define DEFER(id) id EMPTY()

#define FOR_EACH_Add(macro, macro_other_args, x, ...) CAT(FOR_EACH_Add_, IS_EMPTY(__VA_ARGS__)) (macro, macro_other_args, x, __VA_ARGS__)
#define FOR_EACH_Add_0(macro, macro_other_args, x, ...) macro(x, macro_other_args), DEFER(FOR_EACH_Add_I)() (macro, macro_other_args, __VA_ARGS__)
#define FOR_EACH_Add_1(macro, macro_other_args, x, ...) macro(x, macro_other_args)
#define FOR_EACH_Add_I() FOR_EACH_Add

#define FOR_EACH_Add_NO_INTERVAL(macro, macro_other_args, x, ...) CAT(FOR_EACH_Add_NO_INTERVAL_, IS_EMPTY(__VA_ARGS__)) (macro, macro_other_args, x, __VA_ARGS__)
#define FOR_EACH_Add_NO_INTERVAL_0(macro, macro_other_args, x, ...) macro(x, macro_other_args) DEFER(FOR_EACH_Add_NO_INTERVAL_I)() (macro, macro_other_args, __VA_ARGS__)
#define FOR_EACH_Add_NO_INTERVAL_1(macro, macro_other_args, x, ...) macro(x, macro_other_args)
#define FOR_EACH_Add_NO_INTERVAL_I() FOR_EACH_Add_NO_INTERVAL

#define FOR_EACH(macro, x, ...) CAT(FOR_EACH_, IS_EMPTY(__VA_ARGS__)) (macro, x, __VA_ARGS__)
#define FOR_EACH_0(macro, x, ...) macro(x), DEFER(FOR_EACH_I)() (macro, __VA_ARGS__)
#define FOR_EACH_1(macro, x, ...) macro(x)
#define FOR_EACH_I() FOR_EACH

#define FOR_EACH_NO_INTERVAL(macro, x, ...) CAT(FOR_EACH_NO_INTERVAL_, IS_EMPTY(__VA_ARGS__)) (macro, x, __VA_ARGS__)
#define FOR_EACH_NO_INTERVAL_0(macro, x, ...) macro(x) DEFER(FOR_EACH_NO_INTERVAL_I)() (macro, __VA_ARGS__)
#define FOR_EACH_NO_INTERVAL_1(macro, x, ...) macro(x)
#define FOR_EACH_NO_INTERVAL_I() FOR_EACH_NO_INTERVAL

#define EXPAND0(...) __VA_ARGS__
#define EXPAND1(...) EXPAND0(EXPAND0(__VA_ARGS__))
#define EXPAND2(...) EXPAND1(EXPAND1(__VA_ARGS__))
#define EXPAND3(...) EXPAND2(EXPAND2(__VA_ARGS__))
#define EXPAND4(...) EXPAND3(EXPAND3(__VA_ARGS__))
#define EXPAND5(...) EXPAND4(EXPAND4(__VA_ARGS__))

#define NOEVAL0(...) __VA_ARGS__
#define NOEVAL1(...) NOEVAL0(NOEVAL0(__VA_ARGS__))
#define NOEVAL2(...) NOEVAL1(NOEVAL1(__VA_ARGS__))
#define NOEVAL3(...) NOEVAL2(NOEVAL2(__VA_ARGS__))
#define NOEVAL4(...) NOEVAL3(NOEVAL3(__VA_ARGS__))
#define NOEVAL5(...) NOEVAL4(NOEVAL4(__VA_ARGS__))

#define EVAL5(...) __VA_ARGS__
#define EVAL4(...) EVAL5(EVAL5(__VA_ARGS__))
#define EVAL3(...) EVAL4(EVAL4(__VA_ARGS__))
#define EVAL2(...) EVAL3(EVAL3(__VA_ARGS__))
#define EVAL1(...) EVAL2(EVAL2(__VA_ARGS__))
#define EVAL0(...) EVAL1(EVAL1(__VA_ARGS__))

#define EXEVAL(...)__VA_ARGS__
#define EXEVAL1(...) EXEVAL(EXEVAL(__VA_ARGS__))
#define EXEVAL2(...) EXEVAL1(EXEVAL1(__VA_ARGS__))
#define EXEVAL3(...) EXEVAL2(EXEVAL2(__VA_ARGS__))
#define EXEVAL4(...) EXEVAL3(EXEVAL3(__VA_ARGS__))
#define EXEVAL5(...) EXEVAL4(EXEVAL4(__VA_ARGS__))

#define FOREACH(F, ...) EVAL0(FOR_EACH(F, __VA_ARGS__))
#define FOREACH_ADD(F, macro_other_args, ...) EXPAND5(FOR_EACH_Add(F, macro_other_args, __VA_ARGS__))
#define FOREACH_NOINTERVAL(F, ...) NOEVAL5(FOR_EACH_NO_INTERVAL(F, __VA_ARGS__))
#define FOREACH_ADD_NOINTERVAL(F, macro_other_args, ...) EXEVAL5(FOR_EACH_Add_NO_INTERVAL(F, macro_other_args, __VA_ARGS__))

#define Impl(...) __VA_ARGS__

#define List_push_back(x, ...) CAT(List_pusk_back_, IS_EMPTY(__VA_ARGS__)) (x, __VA_ARGS__)
#define List_pusk_back_0(x, ...) Impl x, __VA_ARGS__
#define List_pusk_back_1(x, ...) Impl x

#define Expand(...) __VA_ARGS__

#define Expand_In_List_0_Impl(type, name) type name
#define Expand_In_List_0(...) Expand_In_List_0_Impl __VA_ARGS__
#define Expand_In_List_1(...)
#define Expand_In_List(...) CAT(Expand_In_List_, IS_EMPTY(__VA_ARGS__)) (__VA_ARGS__)

#define Expand_Args_List_0_Impl(type, name) std::forward<decltype(name)>(name)
#define Expand_Args_List_0(...) Expand_Args_List_0_Impl __VA_ARGS__
#define Expand_Args_List_1(...)
#define Expand_Args_List(...) CAT(Expand_Args_List_, IS_EMPTY(__VA_ARGS__)) (__VA_ARGS__)

#define F_Extend(x, name) (Impl name, CAT(F_,__COUNTER__), Impl x)
#define Fn_Impl(True_Name, Inside_Name, ...) \
	((template<typename T, typename ...Args> \
	static auto Inside_Name(void* self, Args ...args) \
	{ \
		return (*static_cast<std::decay_t<T>*>(self)).True_Name(std::forward<Args>(args)...); \
	}), \
    (FOREACH_ADD(F_Extend, (Inside_Name, True_Name), __VA_ARGS__)))
#define Fn(F_Name, ...) Fn_Impl(F_Name, CAT(F_Name,__COUNTER__), __VA_ARGS__)

#define Oper_Impl(oper, name, ...) \
	((template<typename T, typename Arg> \
	static auto name(void* self, Arg arg) \
	{ \
        if constexpr(requires{(*static_cast<std::decay_t<T>*>(self)) oper arg;})\
		    return (*static_cast<std::decay_t<T>*>(self)) oper arg; \
		else return arg oper (*static_cast<std::decay_t<T>*>(self));\
	}), \
	(FOREACH_ADD(F_Extend, (name, operator oper), __VA_ARGS__)))
#define Oper(oper, ...) Oper_Impl(oper, CAT(operator_,__COUNTER__), __VA_ARGS__)

#define Pre_Oper_Impl(oper, name, ...) \
	((template<typename T> \
	static auto name(void* self) \
	{ \
        if constexpr(requires{oper(*static_cast<std::decay_t<T>*>(self));})\
		    return oper(*static_cast<std::decay_t<T>*>(self)); \
		else return (*static_cast<std::decay_t<T>*>(self))oper;\
	}), \
	(FOREACH_ADD(F_Extend, (name, operator oper), __VA_ARGS__)))
#define Pre_Oper(oper, ...) Pre_Oper_Impl(oper, CAT(operator_,__COUNTER__), __VA_ARGS__)

#define Paren_Impl(name, ...) \
	((template<typename T, typename ...Args> \
	static auto name(void* self, Args ...args) \
	{ \
		return (*static_cast<std::decay_t<T>*>(self))(std::forward<Args>(args)...); \
	}),\
	(FOREACH_ADD(F_Extend, (name, operator()), __VA_ARGS__)))
#define Paren(...) Paren_Impl(CAT(operator_,__COUNTER__), __VA_ARGS__)

#define Bracket_Impl(name, ...) \
	((template<typename T, typename ...Args> \
	static auto name(void* self, Args ...args) \
	{ \
		return(*static_cast<std::decay_t<T>*>(self))[std::forward<Args>(args)...]; \
	}), \
	(FOREACH_ADD(F_Extend, (name, operator[]), __VA_ARGS__)))
#define Bracket(...) Bracket_Impl(CAT(operator_,__COUNTER__), __VA_ARGS__)

#define Gener_Warpper_Impl(warpper, override_set) Impl warpper
#define Gener_Warpper(X) Gener_Warpper_Impl X

#define Declare_Impl(inside_name, true_name, f_ptr, type_in, type_out) \
    using CAT(f_ptr, _type) = type_out(List_push_back((void*), Expand type_in)); \
    CAT(f_ptr, _type)* f_ptr = nullptr;
#define Declare(args, x) Declare_Impl args
#define Gener_Delcare_Impl(warpper, override_set) FOREACH_ADD_NOINTERVAL(Declare, x, Impl override_set)
#define Gener_Delcare(X) Gener_Delcare_Impl X

#define Init_Impl(inside_name, true_name, f_ptr, type_in, type_out) f_ptr(& inside_name<List_push_back((std::decay_t<T>), Expand type_in)>)
#define Init(args, x) Init_Impl args
#define Gener_Init_Impl(warpper, override_set) FOREACH_ADD(Init, x, Impl override_set)
#define Gener_Init(x) Gener_Init_Impl x

#define Invoke_Impl(inside_name, true_name, f_ptr, type_in, type_out) \
	type_out true_name(FOREACH(Expand_In_List, Impl type_in)) \
	{ \
		return v_ptr->f_ptr(List_push_back((Object()), FOREACH(Expand_Args_List, Impl type_in))); \
	}
#define Invoke(ags, x) Invoke_Impl ags

#define Args_Add_0_Impl(type, x) (type, CAT(Arg_, __COUNTER__))
#define Args_Add_0(...) FOREACH_ADD(Args_Add_0_Impl, x, __VA_ARGS__)
#define Args_Add_1(...)
#define Args_Add(...) CAT(Args_Add_, IS_EMPTY(__VA_ARGS__)) (__VA_ARGS__)
#define Gener_Args_Impl(inside_name, true_name, f_ptr, type_in, type_out) (inside_name, true_name, f_ptr,(Args_Add type_in), type_out)
#define Gener_Args(x) Gener_Args_Impl x
#define Gener_Invoke_Impl(warpper, override_set) FOREACH_ADD_NOINTERVAL(Invoke, x, FOREACH(Gener_Args, Impl override_set))
#define Gener_Invoke(...) Gener_Invoke_Impl __VA_ARGS__

#define Assign_Impl(inside_name, true_name, f_ptr, type_in, type_out) f_ptr (reflect<List_push_back((#true_name, type_out), Impl type_in)>)
#define Assign(x, y) Assign_Impl x 
#define Gener_Assign_Impl(warpper, override_set) FOREACH_ADD(Assign, x, Impl override_set)
#define Gener_Assign(x) Gener_Assign_Impl x

#define Reflect_Impl(inside_name, true_name, f_ptr, type_in, type_out) \
	template<> \
	auto reflect<List_push_back((#true_name, type_out), Impl type_in)>() const \
	{ \
		return f_ptr; \
	}
#define Reflect(args, x) Reflect_Impl args
#define Gener_Reflect_Impl(warpper, override_set) FOREACH_ADD_NOINTERVAL(Reflect, x, Impl override_set)
#define Gener_Reflect(X) Gener_Reflect_Impl X

#define Interface(intername, ...) \
template <OwnerShip ownership> \
struct intername;\
\
template<>\
struct intername<OwnerShip::Owner> :owner\
{\
private:\
    struct v_ptr_list : is_v_ptr\
    {\
        template<StringLiteral Fun_name, typename ...Args> \
		auto reflect() const;\
        const std::type_info* type = &typeid(void*);\
        void (*destroy)(void*) = nullptr;\
        void* (*copy)(const void*) = nullptr;\
        void (*placement)(void*, const void*) = nullptr;\
        bool sso_tag = false;\
        FOREACH_NOINTERVAL(Gener_Delcare,__VA_ARGS__)\
        FOREACH_NOINTERVAL(Gener_Warpper, __VA_ARGS__)\
        FOREACH_NOINTERVAL(Gener_Reflect, __VA_ARGS__)\
        template <typename T>\
            requires (!has_v_ptr<T>)\
        v_ptr_list(T&& x) : List_push_back((type(&typeid(std::decay_t<T>))),FOREACH(Gener_Init, __VA_ARGS__))\
        {\
            using NonRef = std::decay_t<T>;\
            if (sso_tag = (SSOBufferSize >= sizeof(NonRef)))\
                placement = +[](void* buffer, const void* object) { new(buffer) NonRef(*static_cast<const NonRef*>(object)); };\
            else\
            {\
                destroy = +[](void* object) { delete static_cast<NonRef*>(object); };\
                copy = +[](const void* object) { return (void*)(new NonRef(*static_cast<const NonRef*>(object))); };\
            }\
        }\
        template <typename T>\
            requires has_v_ptr<T> && (!std::is_same_v<std::remove_cvref_t<T>, v_ptr_list>)\
        v_ptr_list(T&& x) noexcept :List_push_back((type(x.type)), FOREACH(Gener_Assign,__VA_ARGS__))\
        {\
            destroy = x.destroy;\
            copy = x.copy;\
            sso_tag = x.sso_tag;\
        }\
        v_ptr_list() = default;\
        v_ptr_list(const v_ptr_list& other) noexcept = default;\
        v_ptr_list(v_ptr_list&& other) noexcept = default;\
        v_ptr_list& operator=(const v_ptr_list& other) noexcept = default;\
        v_ptr_list& operator=(v_ptr_list&& other) noexcept = default;\
    };\
    static std::unordered_map<std::type_index, v_ptr_list> ptr_map;\
    static std::shared_mutex vtableMutex;\
	template<typename T>\
    static v_ptr_list* instance(T&& x, std::type_index index) \
    { \
        static v_ptr_list fat_ptr = ptr_map[index] = v_ptr_list(std::forward<T>(x)); \
        return &fat_ptr;\
    }\
    template<typename T>\
    static v_ptr_list* make_v_ptr(T&& x, std::type_index index)\
    {\
        if constexpr(!has_v_ptr<T>)\
			return instance(std::forward<T>(x), index);\
        else \
        {\
            std::shared_lock<std::shared_mutex> lock(vtableMutex);\
            auto it = ptr_map.find(index);\
            if (it != ptr_map.end())\
                return &it->second;\
            lock.unlock();\
            std::unique_lock<std::shared_mutex> ulock(vtableMutex);\
            it = ptr_map.find(index);\
            if (it == ptr_map.end())\
                return &(ptr_map[index] = v_ptr_list(std::forward<decltype(x)>(x)));\
            return &it->second;\
        }\
    }\
    void* Object()\
    {\
        if (v_ptr->sso_tag)\
            return &object;\
        return object;\
    }\
    friend struct intername<OwnerShip::Observer>;\
public:\
    using V_ptr = v_ptr_list;\
    void* object;\
    v_ptr_list* v_ptr;\
    intername() noexcept = default;\
    template <typename T>\
        requires (!is_interface<T>)\
    intername(T&& x) : v_ptr(make_v_ptr(std::forward<T>(x), std::type_index(typeid(std::decay_t<T>))))\
    {\
        if constexpr (sizeof(T) > SSOBufferSize)\
            object = new std::decay_t<T>(std::forward<T>(x));\
        else new(&object) std::decay_t<T>(std::forward<T>(x));\
    }\
    template<typename T>\
        requires is_owner<T> && (!std::is_same_v<std::remove_cvref_t<T>, intername>)\
    intername(T&& other) : v_ptr(make_v_ptr(*other.v_ptr, std::type_index(*(other.v_ptr->type))))\
    {\
        if constexpr (std::is_rvalue_reference_v<T&&>)\
        {\
            object = std::exchange(other.object, nullptr);\
            other.v_ptr = nullptr;\
        }\
        else\
        {\
            if (!v_ptr->sso_tag)\
                object = v_ptr->copy(other.object);\
            else object = other.object;\
        }\
    }\
    template<typename T>\
        requires is_observer<T> && (!std::is_same_v<std::remove_cvref_t<T>, intername<OwnerShip::Observer>>)\
    intername(T&& other) : v_ptr(make_v_ptr(*other.v_ptr, std::type_index(*(other.v_ptr->type))))\
    {\
        if (!v_ptr->sso_tag)\
        {\
            if constexpr (std::is_rvalue_reference_v<T&&>)\
            {\
                object = std::exchange(other.object, nullptr);\
                other.v_ptr = nullptr;\
            }\
            else object = v_ptr->copy(other.object);\
        }\
        else\
        {\
            v_ptr->placement(&object, other.object);\
            if constexpr (std::is_rvalue_reference_v<T&&>)\
            {\
                other.object = nullptr;\
                other.v_ptr = nullptr;\
            }\
        }\
    }\
    intername(const intername& other) : v_ptr(other.v_ptr)\
    {\
        if (v_ptr->sso_tag)\
            object = other.object;\
        else object = v_ptr->copy(&other);\
    }\
    intername(intername&& other) noexcept : v_ptr(std::exchange(other.v_ptr, nullptr)), object(std::exchange(other.object, nullptr)) {}\
    intername(const intername<OwnerShip::Observer>& other);\
    intername(intername<OwnerShip::Observer>&& other) noexcept;\
    template<typename T>\
        requires (!is_interface<T>)\
    intername& operator=(T&& other)\
    {\
        this->~intername();\
        v_ptr = make_v_ptr(std::forward<T>(other), std::type_index(typeid(std::decay_t<T>)));\
        if constexpr (sizeof(T) > SSOBufferSize)\
            object = new std::decay_t<T>(std::forward<T>(other));\
        else new(&object) std::decay_t<T>(std::forward<T>(other));\
        return *this;\
    }\
    template<typename T>\
        requires is_owner<T> && (!std::is_same_v<std::remove_cvref_t<T>, intername>)\
    intername& operator=(T&& other)\
    {\
        this->~intername();\
        v_ptr = make_v_ptr(*other.v_ptr, std::type_index(*(other.v_ptr->type)));\
        if constexpr (std::is_rvalue_reference_v<T&&>)\
        {\
            object = std::exchange(other.object, nullptr);\
            other.v_ptr = nullptr;\
        }\
        else\
        {\
            if (!v_ptr->sso_tag)\
                object = v_ptr->copy(other.object);\
            else object = other.object;\
        }\
        return *this;\
    }\
    template<typename T>\
        requires is_observer<T> && (!std::is_same_v<std::remove_cvref_t<T>, intername<OwnerShip::Observer>>)\
    intername& operator=(T&& other)\
    {\
        this->~intername();\
        v_ptr = make_v_ptr(*other.v_ptr, std::type_index(*(other.v_ptr->type)));\
        if (!v_ptr->sso_tag)\
        {\
            if constexpr (std::is_rvalue_reference_v<T&&>)\
            {\
                object = std::exchange(other.object, nullptr);\
                other.v_ptr = nullptr;\
            }\
            else object = v_ptr->copy(other.object);\
        }\
        else\
        {\
            v_ptr->placement(&object, other.object);\
            if constexpr (std::is_rvalue_reference_v<T&&>)\
            {\
                other.object = nullptr;\
                other.v_ptr = nullptr;\
            }\
        }\
        return *this;\
    }\
    intername& operator=(const intername& other)\
    {\
        this->~intername();\
        v_ptr = other.v_ptr;\
        if (v_ptr->sso_tag)\
            object = other.object;\
        else object = v_ptr->copy(&other);\
        return *this;\
    }\
    intername& operator=(intername&& other) noexcept\
    {\
        this->~intername();\
        v_ptr = std::exchange(other.v_ptr, nullptr);\
        object = std::exchange(other.object, nullptr);\
        return *this;\
    }\
    intername& operator=(const intername<OwnerShip::Observer>& other);\
    intername& operator=(intername<OwnerShip::Observer>&& other) noexcept;\
    FOREACH_NOINTERVAL(Gener_Invoke, __VA_ARGS__)\
    template<typename T>\
    friend auto& as(const intername& x)\
	{\
		using Noref = std::decay_t<T>;\
		if(x.v_ptr->type!=&typeid(Noref))\
			throw std::bad_cast();\
		return *((Noref*)(x.Object()));\
	}\
    template<typename T>\
    friend auto& as(intername& x)\
	{\
		using Noref = std::decay_t<T>;\
		if(x.v_ptr->type!=&typeid(Noref))\
			throw std::bad_cast();\
		return *((Noref*)(x.Object()));\
	}\
    template<typename T>\
	friend auto as(intername&& x)\
	{\
		using Noref = std::decay_t<T>;\
		if(x.v_ptr->type!=&typeid(Noref))\
			throw std::bad_cast();\
		auto temp = std::move(*((Noref*)(x.Object())));\
        x.object = nullptr;\
		x.v_ptr = nullptr;\
		return temp;\
	}\
    ~intername()\
    {\
        if (v_ptr && object)\
        {\
            if (!v_ptr->sso_tag)\
                v_ptr->destroy(object);\
        }\
        object = nullptr;\
        v_ptr = nullptr;\
    }\
};\
\
std::unordered_map<std::type_index, intername<OwnerShip::Owner>::V_ptr> intername<OwnerShip::Owner>::ptr_map;\
std::shared_mutex intername<OwnerShip::Owner>::vtableMutex;\
\
template<>\
struct intername<OwnerShip::Observer> :observer\
{\
private:\
    using v_ptr_list = intername<OwnerShip::Owner>::v_ptr_list;\
    void* Object()\
    {\
        return object;\
    }\
public:\
    void* object;\
    v_ptr_list* v_ptr;\
    intername() noexcept = default;\
    template <typename T>\
        requires (!is_interface<T>)\
    intername(T& x) : v_ptr(intername<OwnerShip::Owner>::make_v_ptr(std::forward<T>(x), std::type_index(typeid(std::decay_t<T>)))), object(&x) { }\
    template <typename T>\
        requires std::is_rvalue_reference_v<T&&>\
    intername(T&& x) = delete;\
    template<typename T>\
        requires is_owner<T> && (!std::is_same_v<std::remove_cvref_t<T>, intername<OwnerShip::Owner>>)\
    intername(T& other) : v_ptr(intername<OwnerShip::Owner>::make_v_ptr(*other.v_ptr, std::type_index(*(other.v_ptr->type))))\
    {\
        if (v_ptr->sso_tag)\
            object = (void*)(&other.object);\
        else object = other.object;\
    }\
    template<typename T>\
        requires is_observer<T> && (!std::is_same_v<std::remove_cvref_t<T>, intername>)\
    intername(T& other) : object(other.object), v_ptr(intername<OwnerShip::Owner>::make_v_ptr(*other.v_ptr, std::type_index(*(other.v_ptr->type)))) {}\
    intername(const intername<OwnerShip::Owner>& other) noexcept : v_ptr(other.v_ptr)\
    {\
        if (v_ptr->sso_tag)\
            object = (void*)(&other.object);\
        else object = other.object;\
    }\
    intername(const intername& other) noexcept = default;\
    template<typename T>\
        requires (!is_interface<T>)\
    intername& operator=(T& other)\
    {\
        object = &other;\
        v_ptr = intername<OwnerShip::Owner>::make_v_ptr(std::forward<T>(other), std::type_index(typeid(std::decay_t<T>)));\
        return *this;\
    }\
    template<typename T>\
        requires is_owner<T> && (!std::is_same_v<std::remove_cvref_t<T>, intername<OwnerShip::Owner>>)\
    intername& operator=(T& other)\
    {\
        if (v_ptr->sso_tag)\
            object = (void*)(&other.object);\
        else object = other.object;\
        v_ptr = intername<OwnerShip::Owner>::make_v_ptr(*other.v_ptr, std::type_index(*(other.v_ptr->type)));\
        return *this;\
    }\
    template<typename T>\
        requires is_observer<T> && (!std::is_same_v<std::remove_cvref_t<T>, intername>)\
    intername& operator=(T& other)\
    {\
        object = other.object;\
        v_ptr = intername<OwnerShip::Owner>::make_v_ptr(*other.v_ptr, std::type_index(*(other.v_ptr->type)));\
        return *this;\
    }\
    intername& operator=(const intername<OwnerShip::Owner>& other)\
    {\
        object = other.object;\
        v_ptr = other.v_ptr;\
        return *this;\
    }\
    intername& operator=(const intername& other) noexcept = default;\
    template <typename T>\
        requires std::is_rvalue_reference_v<T&&>\
    intername& operator=(T&& other) = delete;\
    FOREACH_NOINTERVAL(Gener_Invoke, __VA_ARGS__)\
    template<typename T>\
    friend auto& as(const intername& x)\
    {\
        using Noref = std::decay_t<T>;\
        if(x.v_ptr->type!=&typeid(Noref))\
			throw std::bad_cast();\
        return *((const Noref*)(x.Object()));\
    }\
    template<typename T>\
    friend auto& as(intername& x)\
    {\
        using Noref = std::decay_t<T>;\
        if(x.v_ptr->type!=&typeid(Noref))\
			throw std::bad_cast();\
        return *((Noref*)(x.Object()));\
    }\
    template<typename T>\
    friend auto as(intername&& x)\
    {\
        using Noref = std::decay_t<T>;\
        if(x.v_ptr->type!=&typeid(Noref))\
			throw std::bad_cast();\
        auto temp = std::move(*((Noref*)(x.Object())));\
        x.object = nullptr;\
		x.v_ptr = nullptr;\
		return temp;\
    }\
    ~intername()\
    {\
        object = nullptr;\
        v_ptr = nullptr;\
    }\
};\
\
intername<OwnerShip::Owner>::intername(const intername<OwnerShip::Observer>& other) : v_ptr(other.v_ptr)\
{\
    if (!v_ptr->sso_tag)\
        object = v_ptr->copy(other.object);\
    else v_ptr->placement(&object, other.object);\
}\
\
intername<OwnerShip::Owner>::intername(intername<OwnerShip::Observer>&& other) noexcept :\
    v_ptr(std::exchange(other.v_ptr, nullptr))\
{\
    if (!v_ptr->sso_tag)\
        object = std::exchange(other.object, nullptr);\
    else\
    {\
        v_ptr->placement(&object, other.object);\
        other.object = nullptr;\
    }\
}\
\
intername<OwnerShip::Owner>& intername<OwnerShip::Owner> ::operator = (const intername <OwnerShip::Observer>& other)\
{\
    this->~intername();\
    v_ptr = other.v_ptr;\
    if (!v_ptr->sso_tag)\
        object = v_ptr->copy(other.object);\
    else v_ptr->placement(&object, other.object);\
    return *this;\
}\
\
intername<OwnerShip::Owner>& intername<OwnerShip::Owner> ::operator = (intername <OwnerShip::Observer>&& other) noexcept\
{\
    this->~intername();\
    v_ptr = std::exchange(other.v_ptr, nullptr);\
    if (!v_ptr->sso_tag)\
        object = std::exchange(other.object, nullptr);\
    else\
    {\
        v_ptr->placement(&object, other.object);\
        other.object = nullptr;\
    }\
    return *this;\
}