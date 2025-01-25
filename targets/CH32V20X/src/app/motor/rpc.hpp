#pragma once

#include "sys/string/string.hpp"
#include "sys/stream/ostream.hpp"
#include <span>
#include <functional>
#include <utility>
#include "utils.hpp"
#include "function_traits.hpp"
#include "sys/polymorphism/proxy.hpp"

namespace ymd::rpc{


//需送入函数中执行的参数 包含了对应的字段和可能的指定参数名
class CallParam{
protected:
    String value_;
    String spec_;
public:
    CallParam(const char * value):
        value_(value),
        spec_(std::nullopt){;}

    CallParam(const StringView value):
        value_(value),
        spec_(std::nullopt){;}

    CallParam(const String && value):
        value_(std::move(value)),
        spec_(std::nullopt){;}
    CallParam(const StringView value, const StringView spec):
        value_(value),
        spec_(spec){;}

    const auto value() const{
        return value_;
    }

    const auto spec() const{
        return spec_;
    }

    template<typename T>
    operator T() const{
        return static_cast<T>(value_);
    }

    friend OutputStream & operator << (OutputStream & os, const CallParam & param){
        return os << param.value_;
    }
};

using Param = CallParam;
using Params = std::span<const CallParam>;

//一个可以被检索的词条
class Entry{
protected:
    String name_;
public:
    Entry(const StringView & name):name_(name){;}
    Entry(const StringView && name):name_(name){;}
    const StringView name(){
        return name_;
    }

    virtual int call(OutputStream & os, const Params params) = 0;
};


// template<typename... Args>
// size_t mysprintf(char *str, Args &&... params) {
//     // 使用 snprintf 进行格式化输出
//     return snprintf(str, std::numeric_limits<size_t>::max(), std::forward<Args>(params)...);
// }

// template<typename T>
// size_t mysprintf(char *str, const T &num) {
//     return 0;
// }

// template<>
// size_t mysprintf(char *str, const iq_t & num) {
//     return StringUtils::qtoa(str, num, 3);
// }

namespace internal{
    PRO_DEF_MEM_DISPATCH(MemCall, call);

    struct EntryFacade : pro::facade_builder
        ::support_copy<pro::constraint_level::nontrivial>
        ::add_convention<internal::MemCall, int(OutputStream &, const std::span<const CallParam>)>
        ::build {};
}
using EntryProxy = pro::proxy<internal::EntryFacade>;


//一个属性
template<typename T>
class Property:public Entry{
protected:
    T * value_;
public:
    Property(const StringView & name,T & value):
        Entry(name),
        value_(&value){;}

    T & value(){
        return *value_;
    }

    int call(OutputStream & os, const Params params) override{
        os << value();
        return 0;
    }
};

//一个方法的参数 描述了名称和默认值
template<typename T>
class MethodArgInfo:public Entry{
protected:
    T preset_;
public:
    MethodArgInfo(const StringView & name,T preset_):
        Entry(name),
        preset_(preset_){;}

    T preset(){
        return preset_;
    }
};






class MethodConcept:public Entry{
public:

protected:

    template<typename T>
    static __inline T convert(const Param & param) {
        return static_cast<T>(param);
    }

    template<typename T, std::size_t... Is>
    static T convert_params(const Params params, std::index_sequence<Is...>) {
        return std::make_tuple(convert<typename std::tuple_element<Is, T>::type>(params[Is])...);
    }
public:
    MethodConcept(const StringView & name):Entry(name){;}

    virtual int call(OutputStream & os, const Params params) = 0;
};




template<typename Ret, typename ... Args>
class MethodByLambda : public MethodConcept {
protected:
    scexpr size_t N = std::tuple_size_v<std::tuple<Args...>>;

    using Callback = std::function<Ret(Args...)>;

    Callback callback_;

public:
    MethodByLambda(const StringView name, const Callback && callback)
        : MethodConcept(name), callback_(callback) {}
    int call(OutputStream & os, const Params params) final override {
        if (params.size() != N) {
        //     os << "Error: Incorrect number of arguments";
        //     return 0;  // 返回 0 表示错误
            return -1;
        }

        auto tuple_params = convert_params<std::tuple<Args...>>(params, std::index_sequence_for<Args...>{});
        Ret ret = std::apply(callback_, tuple_params);

        os << ret;
        // os << params;
        // os << "hi";
        // return os.tellp();  // 返回输出的字节数
        return 0;
    }
};

template<typename Obj, typename Ret, typename ... Args>
class MethodByMemFunc : public MethodConcept {
protected:
    scexpr size_t N = std::tuple_size_v<std::tuple<Args...>>;

    using Callback = Ret (Obj::*)(Args...);

    Obj * obj_;
    Callback callback_;
public:
    MethodByMemFunc(
        const StringView name, 
        Obj * obj, 
        Callback callback
    ) : MethodConcept(name), 
        obj_(obj),
        callback_(callback) {}
    int call(OutputStream & os, const std::span<const CallParam> params) final override {
        // if (params.size() != N) {
        // //     os << "Error: Incorrect number of arguments";
        // //     return 0;  // 返回 0 表示错误
        //     return -1;
        // }
        auto tuple_params = convert_params<std::tuple<Args...>>(params, std::index_sequence_for<Args...>{});

        if constexpr(std::is_void_v<Ret>){
            std::apply([this](Args... args) { (obj_->*callback_)(args...); }, tuple_params);
        } else {
            os << std::apply([this](Args... args) -> Ret { return (obj_->*callback_)(args...); }, tuple_params);
        }

        // os << params;

        // os << "hi";
        // return os.tellp();  // 返回输出的字节数
        return 0;
    }
};


template<typename Ret, typename ... Args>
auto make_rpc_function(auto && callback, const StringView name = ""){
    // using Ret = ymd::function_return_t<Callback>;
    // using Args = typename ymd::function_arg_t<Callback>::type;
    return pro::make_proxy<internal::EntryFacade, MethodByLambda<Ret, Args...>>(
        name,
        std::forward<decltype(callback)>(callback)
    );
}

template<typename Ret, typename ... Args>
auto make_rpc_memfunc(auto & obj, Ret(std::remove_reference_t<decltype(obj)>::*member_func_ptr)(Args...), const StringView name = "") {
    return pro::make_proxy<internal::EntryFacade, MethodByMemFunc<std::remove_cvref_t<decltype(obj)>, Ret, Args...>>(
        name,
        &obj,
        member_func_ptr
    );
}

template<typename T>
auto make_property(T & val, const StringView name = ""){
    return pro::make_proxy<internal::EntryFacade, Property<T>>(
        name, 
        val
    );
}

class EntryList:public Entry{
protected:
    std::vector<EntryProxy> entries_;
public:
    EntryList(const StringView & name):Entry(name){;}


    // template<typename ... Args>
    // EntryList(const StringView name, Args&& ... entries) :
    //     Entry(name)
    //     // entries_{std::move(entries)...}
    // {
    //     // (entries_.push_back(std::forward<Args>(entries)), ...);
    //     // for(auto & entry:std::forward_as_tuple(std::forward<Args>(entries)...))
    //     for(auto & entry:entries){
    //         entries_.push_back(std::move(entry));
    //     }
    // }

    template<typename ... Args>
    EntryList(const StringView name, Args& ... entries) :
        Entry(name)
        // entries_{(std::forward<Args>(entries)...)}
    {
        (entries_.push_back(entries), ...);
    }

    int call(OutputStream & os, const Params params) override{
        for(auto & entry:entries_){
            entry->call(os,params);
        }
        return 0;
    }

    void add(EntryProxy && entry){
        entries_.push_back(std::move(entry));
    }

    void add(EntryProxy & entry){
        entries_.push_back(entry);
    }
};

template<typename ... Args>
auto make_list(const StringView name, Args && ... entries){
    return pro::make_proxy<internal::EntryFacade, EntryList>(
        name, 
        std::forward<Args>(entries)...
    );
}
// template<typename Callback>
// auto make_protocol_function(Callback && callback) {
//     return make_protocol_function("", std::forward<Callback>(callback));
// }
}