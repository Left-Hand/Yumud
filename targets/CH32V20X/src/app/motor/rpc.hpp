#pragma once

#include "sys/string/string.hpp"
#include "sys/stream/ostream.hpp"
#include "sys/stream/BufStream.hpp"
#include <span>
#include <functional>
#include <utility>
#include "utils.hpp"
#include "function_traits.hpp"
#include "sys/polymorphism/proxy.hpp"
#include <utility>
#include <type_traits>

namespace ymd::rpc{


//�����뺯����ִ�еĲ��� �����˶�Ӧ���ֶκͿ��ܵ�ָ��������
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

    template<typename T>
    CallParam(const T & value): value_(String(value)), spec_(std::nullopt){;}

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

//һ�����Ա������Ĵ���
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

enum class EntryType:uint8_t{
    Func,
    MemFunc,
    Const,
    Var,
};

// template<typename... Args>
// size_t mysprintf(char *str, Args &&... params) {
//     // ʹ�� snprintf ���и�ʽ�����
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


//һ������
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
        // switch(params.size()){
        //     case 0:
        //         os << value();
        //         return 0;
        //     case 1:
        //         value() = static_cast
        //     default:
        //         return -1;
        // }

        if constexpr(!std::is_const_v<T>){
            if(params.size()){
                if(params.size() == 1){
                    value() = static_cast<T>(params[0]);
                    return 0;
                }else{
                    return -1;
                }
            }
        }else{
            if(params.size()){
                return -1;
            }
        }

        os << value();
        return 0;

    }
};

//һ�������Ĳ��� ���������ƺ�Ĭ��ֵ
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
            return -1;
        }

        auto tuple_params = convert_params<std::tuple<Args...>>(params, std::index_sequence_for<Args...>{});
        Ret ret = std::apply(callback_, tuple_params);

        os << ret;
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
        auto tuple_params = convert_params<std::tuple<Args...>>(params, std::index_sequence_for<Args...>{});

        if constexpr(std::is_void_v<Ret>){
            std::apply([this](Args... args) { (obj_->*callback_)(args...); }, tuple_params);
        } else {
            os << std::apply([this](Args... args) -> Ret { return (obj_->*callback_)(args...); }, tuple_params);
        }
        return 0;
    }
};



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

    void add(EntryProxy & entry){
        entries_.push_back(entry);
    }
};


struct EntryRID{
protected:
    const uint32_t hash_;
public:

    template<typename T>
    constexpr EntryRID(const T & ent):
        hash_(hash_impl(ent))
    {
        ;
    }

    uint32_t hash() const{ return hash_; }
    operator uint32_t () const{ return hash_; }
};


struct EntryRef{
    const EntryRID rid;
    EntryProxy p;
};

struct EntryRefs{
protected:
    std::vector<EntryRef> refs_;
public:
    EntryRefs() = default;
    EntryRefs(const EntryRefs &) = delete;
    EntryRefs(EntryRefs &&) = delete;

    EntryRef & operator [](const size_t idx){return refs_.at(idx);}
    const EntryRef & operator [](const size_t idx) const {return refs_.at(idx);}
};

class DataBase{
protected:

};


//һ������������ΰ��ĸ���Ԫ����
namespace internal{
template<typename Ret, typename ArgsTuple, template<typename, typename...> class MethodByLambda, typename Lambda>
struct make_method_by_lambda_impl;

template<typename Ret, template<typename, typename...> class MethodByLambda, typename... Args, typename Lambda>
struct make_method_by_lambda_impl<Ret, std::tuple<Args...>, MethodByLambda, Lambda> {
    static auto make(const StringView name, Lambda&& lambda) {
        return pro::make_proxy<internal::EntryFacade, MethodByLambda<Ret, Args...>>(
            name,
            std::forward<Lambda>(lambda)
        );
    }
};

}

// make_lambda ʵ��
template<typename Lambda>
auto make_function(const StringView name, Lambda&& lambda) {
    // ʹ�� std::decay ���Ƴ� Lambda ���͵����ú� const ���η�
    using DecayedLambda = typename std::decay<Lambda>::type;

    // ��ȡ Lambda �ķ������ͺͲ�������
    using Ret = typename function_traits<DecayedLambda>::return_type;
    using ArgsTuple = typename function_traits<DecayedLambda>::args_type;

    // �� ArgsTuple ��ԭΪ�������������� make_proxy
    return internal::make_method_by_lambda_impl<Ret, ArgsTuple, MethodByLambda, Lambda>::make(
        name,
        std::forward<Lambda>(lambda)
    );
}


template<typename Ret, typename ... Args>
auto make_function(const StringView name, Ret(*callback)(Args...)) {
    return pro::make_proxy<internal::EntryFacade, MethodByLambda<Ret, Args...>>(
        name,
        static_cast<Ret(*)(Args...)>(callback)
    );
}

template<typename Ret, typename ... Args>
auto make_function( const StringView name, auto & obj, Ret(std::remove_reference_t<decltype(obj)>::*member_func_ptr)(Args...)) {
    return pro::make_proxy<internal::EntryFacade, MethodByMemFunc<std::remove_cvref_t<decltype(obj)>, Ret, Args...>>(
        name,
        &obj,
        member_func_ptr
    );
}

template<typename T>
auto make_property(const StringView name, T & val){
    return pro::make_proxy<internal::EntryFacade, Property<T>>(
        name, 
        val
    );
}

template<typename T>
requires std::is_const_v<T>
auto make_property(const StringView name, T & val){
    return pro::make_proxy<internal::EntryFacade, Property<T>>(
        name, 
        val
    );
}


template<typename ... Args>
auto make_list(const StringView name, const Args & ... entries){
    return pro::make_proxy<internal::EntryFacade, EntryList>(
        name, 
        (entries)...
    );
}
}