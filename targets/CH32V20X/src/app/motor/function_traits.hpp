// XH-CppUtilities
// C++20 function_traits.h
// Author: xupeigong@sjtu.edu.cn
// Last Updated: 2024-09-12

#ifndef _XH_FUNCTION_TRAITS_H_
#define _XH_FUNCTION_TRAITS_H_

#include <tuple>
#include <type_traits>


namespace ymd{
struct qualifier {
  enum mask : uint8_t {
       const_mask = 0x01,
    volatile_mask = 0x02,
        lref_mask = 0x04,
        rref_mask = 0x08
  };

  enum id : uint8_t {
             value =           0x0,
       const_value =    const_mask,
    volatile_value = volatile_mask,
          cv_value = volatile_mask |    const_value,
              lref =     lref_mask,
        const_lref =     lref_mask |    const_value,
     volatile_lref =     lref_mask | volatile_value,
           cv_lref =     lref_mask |       cv_value,
              rref =     rref_mask,
        const_rref =     rref_mask |    const_value,
     volatile_rref =     rref_mask | volatile_value,
           cv_rref =     rref_mask |       cv_value
  };
};

struct funcqual {
  enum mask : uint8_t {
       const_mask = 0x01,
    volatile_mask = 0x02,
        lref_mask = 0x04,
        rref_mask = 0x08,
    noexcept_mask = 0x10
  };

  enum id : uint8_t {
                      value =           0x0,
                const_value =    const_mask,
             volatile_value = volatile_mask,
                   cv_value = volatile_mask |    const_value,
                       lref =     lref_mask,
                 const_lref =     lref_mask |    const_value,
              volatile_lref =     lref_mask | volatile_value,
                    cv_lref =     lref_mask |       cv_value,
                       rref =     rref_mask,
                 const_rref =     rref_mask |    const_value,
              volatile_rref =     rref_mask | volatile_value,
                    cv_rref =     rref_mask |       cv_value,
             value_noexcept = noexcept_mask |          value,
       const_value_noexcept = noexcept_mask |    const_value,
    volatile_value_noexcept = noexcept_mask | volatile_value,
          cv_value_noexcept = noexcept_mask |       cv_value,
              lref_noexcept = noexcept_mask |           lref,
        const_lref_noexcept = noexcept_mask |     const_lref,
     volatile_lref_noexcept = noexcept_mask |  volatile_lref,
           cv_lref_noexcept = noexcept_mask |        cv_lref,
              rref_noexcept = noexcept_mask |           rref,
        const_rref_noexcept = noexcept_mask |     const_rref,
     volatile_rref_noexcept = noexcept_mask |  volatile_rref,
           cv_rref_noexcept = noexcept_mask |        cv_rref
  };
};

using qualifier_mask = qualifier::mask;
using qualifier_id   = qualifier::id;
using funcqual_mask = funcqual::mask;
using funcqual_id   = funcqual::id;

template <class T>
struct qualifier_of : std::integral_constant<qualifier_id, qualifier_id::value> {
  using       type = T;
  using decay_type = T;
};

#define QUALIFIER_OF(cv, ref, id)                         \
  template <class T>                                      \
  struct qualifier_of<cv T ref>                           \
    : std::integral_constant<qualifier_id, qualifier_id::id> { \
    using       type = cv T ref;                          \
    using decay_type = T;                                 \
  };

QUALIFIER_OF(const, , const_value)
QUALIFIER_OF(volatile, , volatile_value)
QUALIFIER_OF(const volatile, , cv_value)
QUALIFIER_OF(, &, lref)
QUALIFIER_OF(const, &, const_lref)
QUALIFIER_OF(volatile, &, volatile_lref)
QUALIFIER_OF(const volatile, &, cv_lref)
QUALIFIER_OF(, &&, rref)
QUALIFIER_OF(const, &&, const_rref)
QUALIFIER_OF(volatile, &&, volatile_rref)
QUALIFIER_OF(const volatile, &&, cv_rref)

#undef QUALIFIER_OF

template <class T>
struct funcqual_of;

#define FUNCQUAL_OF(qual, id)                           \
  template <class Ret, class... Args>                   \
  struct funcqual_of<Ret(Args...) qual>                 \
    : std::integral_constant<funcqual_id, funcqual_id::id> { \
    using       type = Ret(Args...) qual;               \
    using decay_type = Ret(Args...);                    \
  };                                                    \
  template <class Ret, class... Args>                   \
  struct funcqual_of<Ret(Args..., ...) qual>            \
    : std::integral_constant<funcqual_id, funcqual_id::id> { \
    using       type = Ret(Args..., ...) qual;          \
    using decay_type = Ret(Args..., ...);               \
  };

FUNCQUAL_OF(, value)
FUNCQUAL_OF(const, const_value)
FUNCQUAL_OF(volatile, volatile_value)
FUNCQUAL_OF(const volatile, cv_value)
FUNCQUAL_OF(&, lref)
FUNCQUAL_OF(const&, const_lref)
FUNCQUAL_OF(volatile&, volatile_lref)
FUNCQUAL_OF(const volatile&, cv_lref)
FUNCQUAL_OF(&&, rref)
FUNCQUAL_OF(const&&, const_rref)
FUNCQUAL_OF(volatile&&, volatile_rref)
FUNCQUAL_OF(const volatile&&, cv_rref)
FUNCQUAL_OF(noexcept, value_noexcept)
FUNCQUAL_OF(const noexcept, const_value_noexcept)
FUNCQUAL_OF(volatile noexcept, volatile_value_noexcept)
FUNCQUAL_OF(const volatile noexcept, cv_value_noexcept)
FUNCQUAL_OF(& noexcept, lref_noexcept)
FUNCQUAL_OF(const& noexcept, const_lref_noexcept)
FUNCQUAL_OF(volatile& noexcept, volatile_lref_noexcept)
FUNCQUAL_OF(const volatile& noexcept, cv_lref_noexcept)
FUNCQUAL_OF(&& noexcept, rref_noexcept)
FUNCQUAL_OF(const&& noexcept, const_rref_noexcept)
FUNCQUAL_OF(volatile&& noexcept, volatile_rref_noexcept)
FUNCQUAL_OF(const volatile&& noexcept, cv_rref_noexcept)

#undef FUNCQUAL_OF

template <class T>
inline constexpr qualifier_id qualifier_of_v = qualifier_of<T>::value;
template <class T>
using qualifier_decay_t = qualifier_of<T>::decay_type;
template <class T>
inline constexpr funcqual_id funcqual_of_v = funcqual_of<T>::value;
template <class T>
using funcqual_decay_t = funcqual_of<T>::decay_type;


template <class T>
struct is_function : std::is_function<T> {};

template <class T>
struct is_funcptr
  : std::conjunction<std::is_pointer<T>, is_function<std::remove_pointer_t<T>>> {};

template <class, class = void>
struct is_functor : std::false_type {};

template <class T>
struct is_functor<T, std::void_t<decltype(&T::operator())>> : std::true_type {};

template <class T>
struct is_memfunc : std::is_member_function_pointer<T> {};

template <class T>
inline constexpr bool is_function_v = is_function<T>::value;

template <class T>
inline constexpr bool is_funcptr_v = is_funcptr<T>::value;

template <class T>
inline constexpr bool is_functor_v = is_functor<T>::value;

template <class T>
inline constexpr bool is_memfunc_v = is_memfunc<T>::value;

// nth_of

template <size_t N, class... T>
struct nth_of : std::tuple_element<N, std::tuple<T...>> {};

template <size_t N, class... T>
using nth_of_t = nth_of<N, T...>::type;

// function traits

template <class T>
struct _function_traits : _function_traits<funcqual_decay_t<T>> {};

template <class T>
struct _funcptr_traits : _function_traits<std::remove_pointer_t<T>> {};

template <class T>
struct _functor_traits : _functor_traits<decltype(&T::operator())> {};

template <class T, class C>
struct _functor_traits<T C::*> : _function_traits<T> {};

template <class T>
struct _memfunc_traits;

template <class Ret, class... Args>
struct _function_traits<Ret(Args...)> {
  using        type = Ret(Args...);
  using return_type = Ret;
  using   arg_tuple = std::tuple<Args...>;
  template <size_t N>
  using    arg_type = nth_of_t<N, Args...>;
  inline static constexpr size_t arity = sizeof...(Args);
};

template <class Ret, class... Args>
struct _function_traits<Ret(Args..., ...)> {
  using        type = Ret(Args..., ...);
  using return_type = Ret;
  using   arg_tuple = std::tuple<Args...>;
//   using arg_type = Args...;
  template <size_t N>
  using    arg_type = nth_of_t<N, Args...>;
  inline static constexpr size_t arity = sizeof...(Args);
};

template <class T, class C>
struct _memfunc_traits<T C::*> : _function_traits<T> {
  using          type = T C::*;
  using    class_type = C;
  using function_type = T;
};

template <class T>
struct _function_traits_helper
  : std::conditional_t<is_function_v<T>, _function_traits<T>,
      std::conditional_t<is_funcptr_v<T>, _funcptr_traits<T>,
        std::conditional_t<is_functor_v<T>, _functor_traits<T>,
          std::conditional_t<is_memfunc_v<T>, _memfunc_traits<T>, std::false_type>>>> {};

template <class T>
using function_traits = _function_traits_helper<std::decay_t<T>>;

template <class T>
using function_traits_t = function_traits<T>::type;

template <class T>
using function_return_t = function_traits<T>::return_type;

template <class T>
using function_arg_tuple = function_traits<T>::arg_tuple;

// template <class T>
// using function_arg_t = function_traits<T>::arg_type;

template <class T, size_t N>
using function_arg_t = function_traits<T>::template arg_type<N>;

template <class T>
inline constexpr size_t function_arity_v = function_traits<T>::arity;

template <class T>
using memfunc_class_t = function_traits<T>::class_type;

template <class T>
using memfunc_function_t = function_traits<T>::function_type;


}; // namespace xh

#endif //_XH_FUNCTION_TRAITS_H_