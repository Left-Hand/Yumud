#pragma once

//https://github.com/Mr-enthalpy/Metaclass-Next-Generation-Polymorphism-in-C-

#include <type_traits>

class Any{
    private:
        void* content;
    public:
        Any() = default;
        
        template<typename T>
        Any(T& value) noexcept: content(&value) {}
    
        Any(const Any& other) = default;
        Any& operator=(const Any& other) = default;
        
        template<typename T>
        friend T& Any_cast(Any& any){
            using NonRef = std::decay_t<T>;
            return *static_cast<NonRef*>(any.content);
        }
        explicit operator bool() const{
            return content != nullptr;
        }
    protected:
        virtual ~Any() {};
    };
    
    #define METACLASS_PRIMITIVE_CAT(x, y) x ## y
    #define METACLASS_CAT(x, y) METACLASS_PRIMITIVE_CAT(x, y)
    
    #define MATA_CLASS_GET_SEC(x, n, ...) n
    #define ATTER(atter) atter
    #define CHECK(...) ATTER(MATA_CLASS_GET_SEC(__VA_ARGS__,0))
    #define PROBE(x) x, 1
    
    #define IS_EMPTY(x, ...) CHECK(METACLASS_CAT(METACLASS_PRIMITIVE_CAT(IS_EMPTY_, x), 0))
    #define IS_EMPTY_0 PROBE()
    
    #define EMPTY()
    #define DEFER(id) id EMPTY()
    
    #define __METACLASS_FOR_EACH(macro, x, ...) METACLASS_CAT(FOR_EACH_, IS_EMPTY(__VA_ARGS__)) (macro, x, __VA_ARGS__)
    #define FOR_EACH_0(macro, x, ...) macro(x), DEFER(FOR_EACH_I)() (macro, __VA_ARGS__)
    #define FOR_EACH_1(macro, x, ...) macro(x)
    #define FOR_EACH_I() __METACLASS_FOR_EACH
    
    #define FOR_EACH_NO_INTERVAL(macro, x, ...) METACLASS_CAT(FOR_EACH_NO_INTERVAL_, IS_EMPTY(__VA_ARGS__)) (macro, x, __VA_ARGS__)
    #define FOR_EACH_NO_INTERVAL_0(macro, x, ...) macro(x) DEFER(FOR_EACH_NO_INTERVAL_I)() (macro, __VA_ARGS__)
    #define FOR_EACH_NO_INTERVAL_1(macro, x, ...) macro(x)
    #define FOR_EACH_NO_INTERVAL_I() FOR_EACH_NO_INTERVAL
    
    #define __METACLASS_EVAL(...)  __METACLASS_EVAL1(__METACLASS_EVAL1(__METACLASS_EVAL1(__METACLASS_EVAL1(__VA_ARGS__))))
    #define __METACLASS_EVAL1(...) __METACLASS_EVAL2(__METACLASS_EVAL2(__METACLASS_EVAL2(__METACLASS_EVAL2(__VA_ARGS__))))
    #define __METACLASS_EVAL2(...) __METACLASS_EVAL3(__METACLASS_EVAL3(__METACLASS_EVAL3(__METACLASS_EVAL3(__VA_ARGS__))))
    #define __METACLASS_EVAL3(...) __METACLASS_EVAL4(__METACLASS_EVAL4(__METACLASS_EVAL4(__METACLASS_EVAL4(__VA_ARGS__))))
    #define __METACLASS_EVAL4(...) __METACLASS_Impl(__METACLASS_Impl(__METACLASS_Impl(__METACLASS_Impl(__VA_ARGS__))))
    #define __METACLASS_Impl(...) __VA_ARGS__
    
    #define __METACLASS_FOREACH(F, ...) __METACLASS_EVAL(__METACLASS_FOR_EACH(F, __VA_ARGS__))
    #define __METACLASS_FOREACH_NO_INTERVAL(F, ...) __METACLASS_EVAL(FOR_EACH_NO_INTERVAL(F, __VA_ARGS__))
    
    
    #define __METACLASS_List_push_back(x, ...) METACLASS_CAT(List_pusk_back_, IS_EMPTY(__VA_ARGS__)) (x, __VA_ARGS__)
    #define List_pusk_back_0(x, ...) __METACLASS_Impl x, __VA_ARGS__
    #define List_pusk_back_1(x, ...) __METACLASS_Impl x
    
    #define __METACLASS_Expand(...) __VA_ARGS__
    #define F_Declare_Impl(name, type_in, type_out) \
    using METACLASS_CAT(name, _type) = type_out(__METACLASS_List_push_back((Any&), __METACLASS_Expand type_in)); \
    METACLASS_CAT(name, _type)* name = nullptr;
    #define F_Declare(args) __METACLASS_Impl(F_Declare_Impl args)
    
    #define F_Init_Impl(name, type_in, type_out) \
    name(& Call<__METACLASS_List_push_back((T), __METACLASS_Expand type_in)>)
    #define F_Init(args) __METACLASS_Impl(F_Init_Impl args)
    
    #define	__METACLASS_Gener_Fun_Name(X) (METACLASS_CAT(F_,__COUNTER__), __METACLASS_Impl X)
    
    #define __METACLASS_F_Overload_0_Impl(name, type_in, type_out) \
    if constexpr( std::same_as<decltype(name(*this, std::forward<decltype(args)>(args)...)), type_out> ) \
    { \
    return name(*this, std::forward<decltype(args)>(args)...); \
    }
    #define F_Overload_0(args) __METACLASS_Impl(__METACLASS_F_Overload_0_Impl args)
    #define F_Overload_1_Impl(name, type_in, type_out) \
    else if constexpr( requires{ std::same_as<decltype(name(*this, std::forward<decltype(args)>(args)...)), type_out>;  } ) \
    { \
    return name(*this, std::forward<decltype(args)>(args)...); \
    }
    #define F_Overload_1(args) __METACLASS_Impl(F_Overload_1_Impl args)
    #define Overload_0(x, ...) F_Overload_0(x) __METACLASS_FOREACH_NO_INTERVAL(F_Overload_1, __VA_ARGS__)
    #define Overload_1(x, ...) F_Overload_0(x) 
    #define Overload(x, ...) METACLASS_CAT(Overload_, IS_EMPTY(__VA_ARGS__)) (x, __VA_ARGS__)
    
    #define Interface_Impl(Fun_name, ...) \
    namespace Interface_detail \
    { \
    struct Interface_##Fun_name : virtual public Any \
    { \
    private: \
    __METACLASS_FOREACH_NO_INTERVAL(F_Declare, __VA_ARGS__) \
    template<typename T, typename ...Args> \
    static auto Call(Any& self, Args ...args) \
    { \
    return Any_cast<T&>(self).Fun_name(std::forward<Args>(args)...); \
    }; \
    public: \
    Interface_##Fun_name() = default; \
    template<typename T> \
    Interface_##Fun_name(T& x) : Any(x), __METACLASS_FOREACH(F_Init, __VA_ARGS__) {} \
    Interface_##Fun_name(const Interface_##Fun_name& other) = default; \
    Interface_##Fun_name& operator=(const Interface_##Fun_name& other) = default; \
    auto Fun_name(auto&& ...args) \
    { \
    Overload(__VA_ARGS__) \
    } \
    protected: \
    virtual ~Interface_##Fun_name() {} \
    }; \
    };
    #define Interface(Fun_name, ...) Interface_Impl(Fun_name, __METACLASS_FOREACH(__METACLASS_Gener_Fun_Name, __VA_ARGS__))
    
    #define Make_X(X) Interface_##X(x)
    #define Inter_name(X) Interface_detail::Interface_##X
    #define Metaclass(class_name, ...) \
    struct class_name final: __METACLASS_FOREACH(Inter_name, __VA_ARGS__) \
    { \
    class_name() = default; \
    class_name(auto& x):  Any(x), __METACLASS_FOREACH(Make_X, __VA_ARGS__) {} \
    class_name(const class_name& other) = default; \
    class_name& operator=(const class_name& other) = default; \
    ~ class_name() {} \
    };
    
    #define Fn(X,Y) (X, Y)
    