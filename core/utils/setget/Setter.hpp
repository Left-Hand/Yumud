#pragma once

#include <functional>
#include <type_traits>

#include "core/utils/setget/Getter.hpp"
#include "core/polymorphism/proxy.hpp"

#include "core/math/real.hpp"


namespace ymd::utils{
// template<typename T>
// class GetterIntf;


namespace details{
    PRO_DEF_MEM_DISPATCH(MemPeriod, period);
    PRO_DEF_MEM_DISPATCH(MemUpdate, update);

    template<typename T>
    struct TweenerFacade : pro::facade_builder
        ::add_convention<details::MemPeriod, real_t() const>
        ::add_convention<details::MemUpdate, void(real_t)>
        ::build {};
}

template<typename T>
class SetterIntf{
public:
    using Type = T;
    SetterIntf(const SetterIntf & other) = default;
    SetterIntf(SetterIntf && other) = default;

    SetterIntf() = default;

    virtual ~SetterIntf() = default;
    
    SetterIntf & operator =(const T & value){
        auto & self = *this;
        self(value);
        return self;
    }

    SetterIntf & operator =(GetterIntf<T> & getter){
        auto & self = *this;
        self = getter();
        return self;
    }

    virtual void operator ()(const T & value) = 0; 
};

template<typename T>
class LambdaSetter: public SetterIntf<T>{
public:
    using Setter = std::function<void(T)>;
    using SetterIntf<T>::operator =;

protected:
    Setter setter_;
public:
    template<typename Fn>
    LambdaSetter(Fn && setter)
        : setter_(std::forward<Fn>(setter)) {}

    void operator ()(const T & value) override{
        setter_(value);
    }
};

template<typename ValueType>
auto make_setter(auto & obj, void(std::remove_reference_t<decltype(obj)>::*member_func_ptr)(const ValueType &)) {
    return LambdaSetter<ValueType>(
        [&obj, member_func_ptr](const ValueType & value) {
            (obj.*member_func_ptr)(value);
        });
}

}