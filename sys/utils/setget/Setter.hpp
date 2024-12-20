#pragma once

#include <type_traits>
#include "sys/utils/setget/Getter.hpp"

namespace ymd::utils{
// template<typename T>
// class GetterConcept_t;

template<typename T>
class SetterConcept_t{
public:
    SetterConcept_t(const SetterConcept_t & other) = delete;
    SetterConcept_t(SetterConcept_t && other) = default;

    SetterConcept_t() = default;

    virtual ~SetterConcept_t() = default;
    
    SetterConcept_t & operator =(const T & value){
        auto & self = *this;
        self(value);
        return self;
    }

    SetterConcept_t & operator =(GetterConcept_t<T> & getter){
        auto & self = *this;
        self = getter();
        return self;
    }

    virtual void operator ()(const T & value) = 0; 
};

template<typename T>
class LambdaSetter_t: public SetterConcept_t<T>{
public:
    using Setter = std::function<void(T)>;
    using SetterConcept_t<T>::operator =;

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