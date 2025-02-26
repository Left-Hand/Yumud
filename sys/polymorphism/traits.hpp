#pragma once

//https://github.com/Morglod/cpp_traits

#define MACRO_EVAL0(...) __VA_ARGS__
#define MACRO_EVAL1(...) MACRO_EVAL0(MACRO_EVAL0(MACRO_EVAL0(__VA_ARGS__)))
#define MACRO_EVAL2(...) MACRO_EVAL1(MACRO_EVAL1(MACRO_EVAL1(__VA_ARGS__)))
#define MACRO_EVAL3(...) MACRO_EVAL2(MACRO_EVAL2(MACRO_EVAL2(__VA_ARGS__)))
#define MACRO_EVAL4(...) MACRO_EVAL3(MACRO_EVAL3(MACRO_EVAL3(__VA_ARGS__)))
#define MACRO_EVAL(...)  MACRO_EVAL4(MACRO_EVAL4(MACRO_EVAL4(__VA_ARGS__)))

#define MACRO_MAP_END(...)
#define MACRO_MAP_OUT
#define MACRO_MAP_COMMA ,

#define MACRO_MAP_GET_END2() 0, MACRO_MAP_END
#define MACRO_MAP_GET_END1(...) MACRO_MAP_GET_END2
#define MACRO_MAP_GET_END(...) MACRO_MAP_GET_END1
#define MACRO_MAP_NEXT0(test, next, ...) next MACRO_MAP_OUT
#define MACRO_MAP_NEXT1(test, next) MACRO_MAP_NEXT0(test, next, 0)
#define MACRO_MAP_NEXT(test, next)  MACRO_MAP_NEXT1(MACRO_MAP_GET_END test, next)

#define MACRO_MAP0(f, x, peek, ...) f(x) MACRO_MAP_NEXT(peek, MACRO_MAP1)(f, peek, __VA_ARGS__)
#define MACRO_MAP1(f, x, peek, ...) f(x) MACRO_MAP_NEXT(peek, MACRO_MAP0)(f, peek, __VA_ARGS__)

#define MACRO_MAP_LIST_NEXT1(test, next) MACRO_MAP_NEXT0(test, MACRO_MAP_COMMA next, 0)
#define MACRO_MAP_LIST_NEXT(test, next)  MACRO_MAP_LIST_NEXT1(MACRO_MAP_GET_END test, next)

#define MACRO_MAP_LIST0(f, x, peek, ...) f(x) MACRO_MAP_LIST_NEXT(peek, MACRO_MAP_LIST1)(f, peek, __VA_ARGS__)
#define MACRO_MAP_LIST1(f, x, peek, ...) f(x) MACRO_MAP_LIST_NEXT(peek, MACRO_MAP_LIST0)(f, peek, __VA_ARGS__)

/**
 * Applies the function macro `f` to each of the remaining parameters.
 */
#define MACRO_MAP(f, ...) MACRO_EVAL(MACRO_MAP1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

/**
 * Applies the function macro `f` to each of the remaining parameters and
 * inserts commas between the results.
 */
#define MACRO_MAP_LIST(f, ...) MACRO_EVAL(MACRO_MAP_LIST1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))
/*
* The PP_NARG macro evaluates to the number of arguments that have been
* passed to it.
*
* Laurent Deniau, "__VA_NARG__," 17 January 2006, <comp.std.c> (29 November 2007).
*/
#define PP_NARG(...)    PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...)   PP_ARG_N(__VA_ARGS__)

#define PP_ARG_N( \
_1, _2, _3, _4, _5, _6, _7, _8, _9,_10,  \
_11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
_21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
_31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
_41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
_51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
_61,_62,_63,N,...) N

#define PP_RSEQ_N() \
63,62,61,60,                   \
59,58,57,56,55,54,53,52,51,50, \
49,48,47,46,45,44,43,42,41,40, \
39,38,37,36,35,34,33,32,31,30, \
29,28,27,26,25,24,23,22,21,20, \
19,18,17,16,15,14,13,12,11,10, \
9,8,7,6,5,4,3,2,1,0


/* Vladimir Kurzhkov 2022 MIT github.com/Morglod/cpp_traits */

#define _TRAIT_STRUCT_CONCAT_DELAY(x, y) x##y
#define _TRAIT_STRUCT_CONCAT(x, y) _TRAIT_STRUCT_CONCAT_DELAY(x, y)

#define _TRAIT_COMMA_IF_DELAYED(...) __VA_OPT__( , __VA_ARGS__ )
#define _TRAIT_COMMA_IF_(...) _TRAIT_COMMA_IF_DELAYED(__VA_ARGS__)

#define _TRAIT_NAMED_ARGS1_RUN_1(a) a _1
#define _TRAIT_NAMED_ARGS1_RUN_2(a, ...) a _2, _TRAIT_NAMED_ARGS1_RUN_1(__VA_ARGS__)
#define _TRAIT_NAMED_ARGS1_RUN_3(a, ...) a _3, _TRAIT_NAMED_ARGS1_RUN_2(__VA_ARGS__)
#define _TRAIT_NAMED_ARGS1_RUN_4(a, ...) a _4, _TRAIT_NAMED_ARGS1_RUN_3(__VA_ARGS__)
#define _TRAIT_NAMED_ARGS1_RUN_5(a, ...) a _5, _TRAIT_NAMED_ARGS1_RUN_4(__VA_ARGS__)
#define _TRAIT_NAMED_ARGS1_RUN_6(a, ...) a _6, _TRAIT_NAMED_ARGS1_RUN_5(__VA_ARGS__)
#define _TRAIT_NAMED_ARGS1_RUN_7(a, ...) a _7, _TRAIT_NAMED_ARGS1_RUN_6(__VA_ARGS__)
#define _TRAIT_NAMED_ARGS1_RUN_8(a, ...) a _8, _TRAIT_NAMED_ARGS1_RUN_7(__VA_ARGS__)
#define _TRAIT_NAMED_ARGS1_RUN_9(a, ...) a _9, _TRAIT_NAMED_ARGS1_RUN_8(__VA_ARGS__)
#define _TRAIT_NAMED_ARGS1_RUN_10(a, ...) a _10, _TRAIT_NAMED_ARGS1_RUN_9(__VA_ARGS__)
#define _TRAIT_NAMED_ARGS1_RUN_(...) __VA_OPT__( _TRAIT_STRUCT_CONCAT(_TRAIT_NAMED_ARGS1_RUN_, PP_NARG(__VA_ARGS__)) (__VA_ARGS__)  )

#define _TRAIT_NAMED_ARGS1_(...) _TRAIT_NAMED_ARGS1_RUN_(__VA_ARGS__)

#define _TRAIT_NAMED_ARGS2_RUN_1(a) _1
#define _TRAIT_NAMED_ARGS2_RUN_2(a, ...) _2, _TRAIT_NAMED_ARGS2_RUN_1(__VA_ARGS__)
#define _TRAIT_NAMED_ARGS2_RUN_3(a, ...) _3, _TRAIT_NAMED_ARGS2_RUN_2(__VA_ARGS__)
#define _TRAIT_NAMED_ARGS2_RUN_4(a, ...) _4, _TRAIT_NAMED_ARGS2_RUN_3(__VA_ARGS__)
#define _TRAIT_NAMED_ARGS2_RUN_5(a, ...) _5, _TRAIT_NAMED_ARGS2_RUN_4(__VA_ARGS__)
#define _TRAIT_NAMED_ARGS2_RUN_6(a, ...) _6, _TRAIT_NAMED_ARGS2_RUN_5(__VA_ARGS__)
#define _TRAIT_NAMED_ARGS2_RUN_7(a, ...) _7, _TRAIT_NAMED_ARGS2_RUN_6(__VA_ARGS__)
#define _TRAIT_NAMED_ARGS2_RUN_8(a, ...) _8, _TRAIT_NAMED_ARGS2_RUN_7(__VA_ARGS__)
#define _TRAIT_NAMED_ARGS2_RUN_9(a, ...) _9, _TRAIT_NAMED_ARGS2_RUN_8(__VA_ARGS__)
#define _TRAIT_NAMED_ARGS2_RUN_10(a, ...) _10, _TRAIT_NAMED_ARGS2_RUN_9(__VA_ARGS__)
#define _TRAIT_NAMED_ARGS2_RUN_(...) __VA_OPT__( _TRAIT_STRUCT_CONCAT(_TRAIT_NAMED_ARGS2_RUN_, PP_NARG(__VA_ARGS__)) (__VA_ARGS__) )

#define _TRAIT_NAMED_ARGS2_(...) _TRAIT_NAMED_ARGS2_RUN_(__VA_ARGS__)

#define TRAIT_METHOD(_RET, _NAME, ...) (_RET, _NAME, __VA_ARGS__)

#define __TRAIT_METHOD_PICK_RET(_ret, ...) _ret
#define _TRAIT_METHOD_PICK_RET(_trait_tuple_) __TRAIT_METHOD_PICK_RET _trait_tuple_

#define __TRAIT_METHOD_PICK_NAME(_ret, _name, ...) _name
#define _TRAIT_METHOD_PICK_NAME(_trait_tuple_) __TRAIT_METHOD_PICK_NAME _trait_tuple_

#define __TRAIT_METHOD_PICK_ARGS(_ret, _name, ...) __VA_ARGS__
#define _TRAIT_METHOD_PICK_ARGS(_trait_tuple_) __TRAIT_METHOD_PICK_ARGS _trait_tuple_

#define _TRAIT_STRUCT_METHOD_POINTER_MEMBER(_method_tuple_) \
_TRAIT_METHOD_PICK_RET(_method_tuple_) (* _TRAIT_METHOD_PICK_NAME(_method_tuple_)) (void* obj_ _TRAIT_COMMA_IF_( _TRAIT_METHOD_PICK_ARGS(_method_tuple_) ) ) = &Self::_TRAIT_STRUCT_CONCAT(static_, _TRAIT_METHOD_PICK_NAME(_method_tuple_));

#define _TRAIT_STRUCT_METHOD_POINTER_STATIC(_method_tuple_) \
static _TRAIT_METHOD_PICK_RET(_method_tuple_) _TRAIT_STRUCT_CONCAT(static_, _TRAIT_METHOD_PICK_NAME(_method_tuple_)) (void* obj_ _TRAIT_COMMA_IF_( _TRAIT_NAMED_ARGS1_(_TRAIT_METHOD_PICK_ARGS(_method_tuple_)) ) ) { \
return ((T*)obj_)->_TRAIT_METHOD_PICK_NAME(_method_tuple_) ( _TRAIT_NAMED_ARGS2_(_TRAIT_METHOD_PICK_ARGS(_method_tuple_)) ); \
};

#define _TRAIT_STRUCT_METHOD_POINTER_NO_INIT(_method_tuple_) \
_TRAIT_METHOD_PICK_RET(_method_tuple_) (* _TRAIT_METHOD_PICK_NAME(_method_tuple_)) (void* obj_ _TRAIT_COMMA_IF_( _TRAIT_METHOD_PICK_ARGS(_method_tuple_) ) );

#define _TRAIT_STRUCT_PROXY_METHOD(_method_tuple_) \
inline _TRAIT_METHOD_PICK_RET(_method_tuple_) _TRAIT_METHOD_PICK_NAME(_method_tuple_) ( _TRAIT_NAMED_ARGS1_(_TRAIT_METHOD_PICK_ARGS(_method_tuple_)) ) { \
return vtable_-> _TRAIT_METHOD_PICK_NAME(_method_tuple_) (_get_obj_() _TRAIT_COMMA_IF_( _TRAIT_NAMED_ARGS2_(_TRAIT_METHOD_PICK_ARGS(_method_tuple_)) ) ); \
}

#define _TRAIT_STRUCT_BASE(_NAME, ...) \
template<typename T> \
struct __##_NAME##_vable_T { \
using Self = __##_NAME##_vable_T<T>; \
MACRO_MAP(_TRAIT_STRUCT_METHOD_POINTER_MEMBER, __VA_ARGS__) \
MACRO_MAP(_TRAIT_STRUCT_METHOD_POINTER_STATIC, __VA_ARGS__) \
}; \
\
struct __##_NAME##vtable_ { \
MACRO_MAP(_TRAIT_STRUCT_METHOD_POINTER_NO_INIT, __VA_ARGS__) \
}; \
\
struct _NAME { \
public:\
    _NAME() = delete; \
    MACRO_MAP(_TRAIT_STRUCT_PROXY_METHOD, __VA_ARGS__) \
    \
    template<typename T> \
    /**/ \
    _NAME(T * t) : obj_(t) { \
    static __##_NAME##_vable_T<T> impl; \
    vtable_ = reinterpret_cast<__##_NAME##vtable_*>(&impl); \
    } \
private: \
    __attribute__((always_inline)) void* _get_obj_() { return obj_; } \
    __attribute__((always_inline)) const void* _get_cobj_() { return obj_; } \
    void* obj_ = nullptr; \
    __##_NAME##vtable_* vtable_; \
};

#define TRAIT_STRUCT(_NAME, ...) _TRAIT_STRUCT_BASE(_NAME, __VA_ARGS__)
