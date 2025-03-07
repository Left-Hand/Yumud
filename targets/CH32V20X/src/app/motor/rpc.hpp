#pragma once

#include <span>
#include <utility>
#include <functional>
#include "utils.hpp"
#include "function_traits.hpp"
#include <utility>
#include <type_traits>

#include "sys/string/string.hpp"
#include "sys/stream/ostream.hpp"
#include "sys/stream/BufStream.hpp"
#include "sys/polymorphism/proxy.hpp"

namespace ymd::rpc{

//需送入函数中执行的参数 包含了对应的字段和可能的指定参数名
class CallParam{
protected:
    StringView value_;
    StringView spec_;
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


// class AccessProviderIntf{
// public:
//     virtual size_t size() const = 0;
// };

// class AccessProviderByString{
// public:

// };

using AccessProviderIntf = Params;

// class AccessReponserIntf{

// };

// class AccessReponserByString:public AccessReponserIntf{
// public:
// };

using AccessReponserIntf = OutputStream;

enum class AccessResult: uint8_t{
    OK,
    Fail,
};



//一个可以被检索的词条
class EntryIntf{
protected:
    String name_;
public:
    EntryIntf(const StringView & name):name_(name){;}
    EntryIntf(const StringView && name):name_(name){;}
    const StringView name(){
        return name_;
    }

    virtual AccessResult call(AccessReponserIntf & ar, const AccessProviderIntf & ap) = 0;
};

enum class EntryType:uint8_t{
    Func,
    MemFunc,
    Const,
    Var,
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
    PRO_DEF_MEM_DISPATCH(MemName, name);

    struct EntryFacade : pro::facade_builder
        ::support_copy<pro::constraint_level::nontrivial>
        ::add_convention<internal::MemCall, AccessResult(AccessReponserIntf &, const AccessProviderIntf &)>
        ::add_convention<internal::MemName, StringView()>
        ::build {};
}
using EntryProxy = pro::proxy<internal::EntryFacade>;


//一个属性
template<typename T>
class Property:public EntryIntf{
protected:
    T * value_;
public:
    Property(const StringView & name,T & value):
        EntryIntf(name),
        value_(&value){;}

    T & value(){
        return *value_;
    }

    AccessResult call(AccessReponserIntf & ar, const AccessProviderIntf & ap) final override{

        if constexpr(!std::is_const_v<T>){
            if(ap.size()){
                if(ap.size() == 1){
                    value() = static_cast<T>(ap[0]);
                    return AccessResult::OK;
                }else{
                    return AccessResult::Fail;
                }
            }
        }else{
            if(ap.size()){
                return AccessResult::Fail;
            }
        }

        ar << value();
        return AccessResult::OK;

    }
};



//一个方法的参数 描述了名称和默认值
template<typename T>
class MethodArgInfo:public EntryIntf{
protected:
    T preset_;
public:
    MethodArgInfo(const StringView & name,T preset):
        EntryIntf(name),
        preset_(preset){;}

    T preset(){
        return preset_;
    }
};






class MethodIntf:public EntryIntf{
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
    MethodIntf(const StringView & name):EntryIntf(name){;}

    virtual AccessResult call(AccessReponserIntf & ar, const AccessProviderIntf & ap) = 0;
};




template<typename Ret, typename ... Args>
class MethodByLambda : public MethodIntf {
protected:
    scexpr size_t N = std::tuple_size_v<std::tuple<Args...>>;

    using Callback = std::function<Ret(Args...)>;

    Callback callback_;

public:
    MethodByLambda(const StringView name, const Callback && callback)
        : MethodIntf(name), callback_(callback) {}
    AccessResult call(AccessReponserIntf & ar, const AccessProviderIntf & ap) final override {
        if (ap.size() != N) {
            return AccessResult::Fail;
        }

        auto tuple_params = convert_params<std::tuple<Args...>>(ap, std::index_sequence_for<Args...>{});

        if constexpr(std::is_void_v<Ret>){
            std::apply(callback_, tuple_params);
        } else {
            ar << std::apply(callback_, tuple_params);
        }
        return AccessResult::OK;
    }
};

template<typename Obj, typename Ret, typename ... Args>
class MethodByMemFunc : public MethodIntf {
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
    ) : MethodIntf(name), 
        obj_(obj),
        callback_(callback) {}
    AccessResult call(AccessReponserIntf & ar, const AccessProviderIntf & ap) final override {
        auto tuple_params = convert_params<std::tuple<Args...>>(ap, std::index_sequence_for<Args...>{});

        if constexpr(std::is_void_v<Ret>){
            std::apply([this](Args... args) { (obj_->*callback_)(args...); }, tuple_params);
        } else {
            ar << std::apply([this](Args... args) -> Ret { return (obj_->*callback_)(args...); }, tuple_params);
        }
        return AccessResult::OK;
    }
};



class EntryList:public EntryIntf{
protected:
    std::vector<EntryProxy> entries_;
public:
    EntryList(const StringView & name):EntryIntf(name){;}


    // template<typename ... Args>
    // EntryList(const StringView name, Args&& ... entries) :
    //     EntryIntf(name)
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
        EntryIntf(name)
        // entries_{(std::forward<Args>(entries)...)}
    {
        (entries_.push_back(entries), ...);
    }

    AccessResult call(AccessReponserIntf & ar, const AccessProviderIntf & ap) override{
        // ar << ap;
        // if(ap.size() < 2) return AccessResult::Fail;

        auto head_hash = StringView(ap[0]).hash();

        if(head_hash == "ls"_ha){
            for(auto & entry:entries_){
                ar.println(entry->name());
            }
            return AccessResult::OK;
        }

        for(auto & entry:entries_){
            auto ent_hash = entry->name().hash();
            if(head_hash == ent_hash){
                return entry->call(ar, {ap.begin() + 1, ap.end()});
            }
        }

        return AccessResult::Fail;
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


//一个帮助解析变参包的辅助元函数
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

// make_lambda 实现
template<typename Lambda>
auto make_function(const StringView name, Lambda&& lambda) {
    // 使用 std::decay 来移除 Lambda 类型的引用和 const 修饰符
    using DecayedLambda = typename std::decay<Lambda>::type;

    // 提取 Lambda 的返回类型和参数类型
    using Ret = typename function_traits<DecayedLambda>::return_type;
    using ArgsTuple = typename function_traits<DecayedLambda>::args_type;

    // 将 ArgsTuple 还原为参数包，并调用 make_proxy
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
auto make_ro_property(const StringView name, const T & val){
    return pro::make_proxy<internal::EntryFacade, Property<const T>>(
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



