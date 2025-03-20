#pragma once

#include <type_traits>
#include <functional>

namespace ymd::utils{

template<typename T>
class GetterConcept_t{
public:
    GetterConcept_t(const GetterConcept_t & other) = delete;
    GetterConcept_t(GetterConcept_t && other) = default;

    GetterConcept_t() = default;

    virtual ~GetterConcept_t() = default;

    virtual T operator()() = 0;

    operator T(){
        return this->operator()();
    }
};


template<typename T>
class LambdaGetter_t: public GetterConcept_t<T>{
public:
    using Getter = std::function<T(void)>;
protected:
    Getter _getter;
public:
    template<typename F>
    LambdaGetter_t(F && getter)
        : _getter(std::forward<F>(getter)) {}

    T operator ()() override {
        return _getter();
    }
};

template<typename ValueType>
auto make_getter(auto & obj, ValueType(std::remove_reference_t<decltype(obj)>::*member_func_ptr)()) {
    return LambdaGetter_t<ValueType>(
        [&obj, member_func_ptr]() {
            return (obj.*member_func_ptr)();
        });
}

template<typename ValueType, typename Func>
auto make_getter(Func && func) {
    return LambdaGetter_t<ValueType>(
        [func = std::forward<Func>(func)]() -> ValueType {
            return func();
        }
    );
}

}