#pragma once

#include <functional>
#include <type_traits>

#include "core/utils/setget/Getter.hpp"
#include "core/polymorphism/proxy.hpp"

#include "core/math/real.hpp"


namespace ymd::utils{
// template<typename T>
// class GetterIntf_t;


namespace internal{
    PRO_DEF_MEM_DISPATCH(MemPeriod, period);
    PRO_DEF_MEM_DISPATCH(MemUpdate, update);

    template<typename T>
    struct TweenerFacade_t : pro::facade_builder
        ::add_convention<internal::MemPeriod, real_t() const>
        ::add_convention<internal::MemUpdate, void(real_t)>
        ::build {};
}

template<typename T>
class SetterIntf_t{
public:
    using Type = T;
    SetterIntf_t(const SetterIntf_t & other) = default;
    SetterIntf_t(SetterIntf_t && other) = default;

    SetterIntf_t() = default;

    virtual ~SetterIntf_t() = default;
    
    SetterIntf_t & operator =(const T & value){
        auto & self = *this;
        self(value);
        return self;
    }

    SetterIntf_t & operator =(GetterIntf_t<T> & getter){
        auto & self = *this;
        self = getter();
        return self;
    }

    virtual void operator ()(const T & value) = 0; 
};

template<typename T>
class LambdaSetter_t: public SetterIntf_t<T>{
public:
    using Setter = std::function<void(T)>;
    // static constexpr auto a = sizeof(std::function<void(real_t)>);
    using SetterIntf_t<T>::operator =;

protected:
    Setter _setter;
public:
    template<typename F>
    LambdaSetter_t(F && setter)
        : _setter(std::forward<F>(setter)) {}

    // LambdaSetter_t & operator =(const T & value){
    //     auto & self = *this;
    //     self(value);
    //     return self;
    // }

    void operator ()(const T & value) override{
        _setter(value);
    }
};

template<typename ValueType>
auto make_setter(auto & obj, void(std::remove_reference_t<decltype(obj)>::*member_func_ptr)(const ValueType &)) {
    return LambdaSetter_t<ValueType>(
        [&obj, member_func_ptr](const ValueType & value) {
            (obj.*member_func_ptr)(value);
        });
}

}