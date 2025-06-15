#pragma once

#include <span>
#include <utility>
#include <functional>
#include <utility>
#include <type_traits>

#include "core/utils/typetraits/function_traits.hpp"
// #include "core/string/string.hpp"
#include "core/string/stringView.hpp"
#include "core/stream/ostream.hpp"
#include "core/stream/BufStream.hpp"
#include "core/polymorphism/proxy.hpp"

namespace ymd::rpc{

class ParamFromString{
protected:
    StringView value_;
public:
    ParamFromString(const char * value):
        value_(value){;}

    ParamFromString(const StringView value):
        value_(value){;}

    template<typename T>
    requires (std::is_convertible_v<StringView, T>)
    T to() const{
        return static_cast<T>(value_);
    }

    template<typename T>
    Option<T> try_to() const {
        if constexpr(std::is_convertible_v<StringView, T>) {
            return Some(static_cast<T>(value_));
        }
        return None;
    }

    friend OutputStream & operator << (OutputStream & os, const ParamFromString & param){
        return os << param.value_;
    }
};

using Param = ParamFromString;
using Params = std::span<const ParamFromString>;


class AccessProviderIntf{
public:
    virtual size_t size() const = 0;
    virtual ParamFromString operator[](size_t idx) const = 0;
};

// 先定义 SubHelper（不依赖 AccessProviderIntf 的完整定义）
class SubHelper final : public AccessProviderIntf {
public:
    constexpr SubHelper(const AccessProviderIntf & provider, 
            size_t offset, size_t end): 
        provider_(provider), offset_(offset), end_(end){;}
    size_t size() const {return end_ - offset_;}
    ParamFromString operator[](size_t idx) const{
        if(idx >= size()) PANIC("idx out of range");
        return ParamFromString(provider_[offset_ + idx]);
    }
private:
    const AccessProviderIntf & provider_;
    const size_t offset_;
    const size_t end_;
};

static constexpr SubHelper make_sub_provider(
    const AccessProviderIntf & owner, 
    const size_t offset, 
    const size_t end){
        return SubHelper(owner, offset, end);
}

static constexpr SubHelper make_sub_provider(
    const AccessProviderIntf & owner, 
    const size_t offset)
{
        return SubHelper(owner, offset, owner.size());
}
class AccessProvider_ByStringViews final: public AccessProviderIntf{
public: 
    AccessProvider_ByStringViews(const std::span<const StringView> views):
        views_(views){;}

    size_t size() const{
        return views_.size();
    }

    ParamFromString operator [](const size_t idx) const {
        return ParamFromString(views_[idx]);
    }
private:    
    std::span<const StringView> views_;
};

using AccessReponserIntf = OutputStream;


enum class AccessError: uint8_t{
    InvalidAccess,
    AccessOutOfRange,
    NotSupported,
    NoCallableExists,
    NoArgsInput,
    ExecutionFailed,
    ArgsCountNotMatch,
    CantModifyConst
};

DERIVE_DEBUG(AccessError)

template<typename T = void>
using AccessResult = Result<T, AccessError>;


// 主模板定义（处理非模板类和默认情况）
template <typename T, typename = void>
struct Visitor {
    static AccessResult<> visit(T& self, AccessReponserIntf& ar, const AccessProviderIntf& ap) {
        static_assert(sizeof(T) == 0, "No visitor specialization found for this type");
        return Err(AccessError::NotSupported);
    }
};

// 辅助类型特征检测
template <template<typename...> class, typename...>
struct is_instantiation_of : std::false_type {};

template <template<typename...> class U, typename... Ts>
struct is_instantiation_of<U, U<Ts...>> : std::true_type {};

template <typename T, template<typename...> class U>
using is_instantiation_of_t = typename is_instantiation_of<U, T>::type;


template<typename T>
struct Property final{
    Property(const StringView name,T * value):
        name_(name),
        value_(value){;}

    // Property(const Property & other) = delete;
    // Property(Property && other) = delete;

    T & get() const {
        return *value_;
    }

    StringView name() const{
        return name_;
    }
private:
    StringView name_;
    T * value_;
};

// Property<T> 非const特化
template <typename T>
struct Visitor<Property<T>, std::enable_if_t<!std::is_const_v<T>>> {
    static AccessResult<> visit(const Property<T>& self, AccessReponserIntf& ar, const AccessProviderIntf& ap) {
        if (ap.size() != 1) return Err(AccessError::InvalidAccess);
        self.get() = ap[0].template to<std::decay_t<T>>();
        ar << self.get();
        return Ok();
    }
};

// Property<T> const特化
template <typename T>
struct Visitor<const Property<T>, void> {
    static AccessResult<> visit(const Property<T>& self, AccessReponserIntf& ar, const AccessProviderIntf& ap) {
        if (ap.size()) 
            return Err(AccessError::CantModifyConst);
        ar << self.get();
        return Ok();
    }
};


template<typename Tuple, std::size_t... Is>
static Tuple convert_params_to_tuple(const auto & params, std::index_sequence<Is...>) {
    return std::make_tuple((params[Is]).template to<
        typename std::tuple_element<Is, Tuple>::type>()...);
}



template<typename Ret, typename ... Args>
struct MethodByLambda final{
protected:
    static constexpr size_t N = sizeof...(Args);

    using Callback = std::function<Ret(Args...)>;

private:
public:
    StringView name_;
    Callback callback_;
    MethodByLambda(const StringView name, const Callback && callback)
        : name_(name), callback_(callback) {}

    StringView name() const { return name_; }
};

template <typename Ret, typename... Args>
struct Visitor<MethodByLambda<Ret, Args...>> {
    using Self = MethodByLambda<Ret, Args...>;
    static AccessResult<> visit(const Self & self, 
        AccessReponserIntf& ar, 
        const AccessProviderIntf& ap
    ) {
        if (ap.size() != sizeof...(Args)) {
            return Err(AccessError::ArgsCountNotMatch);
        }

        auto tuple_params = convert_params_to_tuple<std::tuple<Args...>>(
            ap, std::index_sequence_for<Args...>{});

        if constexpr(std::is_void_v<Ret>){
            std::apply(self.callback_, tuple_params);
        } else {
            ar << std::apply(self.callback_, tuple_params);
        }
        return Ok();
    }
};


template<typename Obj, typename Ret, typename ... Args>
struct MethodByMemFunc final{

    static constexpr size_t N = sizeof...(Args);

    using Callback = Ret (Obj::*)(Args...);

private:
    StringView name_;
    Obj * obj_;
    Callback callback_;
public:
    MethodByMemFunc(
        const StringView name, 
        Obj * obj, 
        Callback callback
    ) : name_(name), 
        obj_(obj),
        callback_(callback) {}

    StringView name() const{return name_;}

    // 新增的 call 方法
    template<typename... CallArgs>
    auto call(CallArgs&&... args) const {
        static_assert(sizeof...(CallArgs) == sizeof...(Args), "Argument count mismatch");
        return std::invoke(callback_, obj_, std::forward<CallArgs>(args)...);
    }
};


// MethodByMemFunc 非const特化
template <typename Obj, typename Ret, typename ... Args>
struct Visitor<MethodByMemFunc<Obj, Ret, Args...>> {
    using Self = MethodByMemFunc<Obj, Ret, Args...>;
    static AccessResult<> visit(const Self & self, 
        AccessReponserIntf& ar, 
        const AccessProviderIntf& ap
    ) {
        if (ap.size() != sizeof...(Args)) {
            return Err(AccessError::ArgsCountNotMatch);
        }

        auto tuple_params = convert_params_to_tuple<std::tuple<Args...>>(
            ap, std::index_sequence_for<Args...>{});

        if constexpr(std::is_void_v<Ret>){
            std::apply([&self](Args... args) { self.call(args...); }, tuple_params);
        } else {
            ar << std::apply([&self](Args... args) -> Ret { return self.call(args...); }, tuple_params);
        }
        return Ok();
    }
};

template<typename... Entries>
struct EntryList final{
    EntryList(const StringView name):
    name_(name){;}

    template<typename... Args>
    EntryList(const StringView name, Args&&... entries) :
        name_(name),
        // entries_(std::forward_as_tuple(entries...)) {}
          entries_(std::forward<Args>(entries)...) {} // Correctly forward arguments


    const auto & entries() const { return entries_; } 

    const StringView name() const { return name_; }
private:
    StringView name_;
    std::tuple<Entries...> entries_;
};

template<typename... Entries>
struct Visitor<EntryList<Entries...>> {
    using Self = EntryList<Entries...>;
    static AccessResult<> visit(const Self & self, 
        AccessReponserIntf& ar, 
        const AccessProviderIntf& ap
    ) {
        if(ap.size() == 0) 
            return Err(AccessError::NoArgsInput);

        const auto head_hash = ap[0].to<StringView>().hash();
        // Modify the first block for "ls" command
        if (head_hash == "ls"_ha) {
            std::apply([&ar](auto&&... entry) { ar.println(entry.name()...); }, self.entries());
            return Ok();
        }
        return std::apply([&](auto&&... entry) -> AccessResult<> {
            AccessResult<> res = Err(AccessError::NoCallableExists);
            ( [&]() -> void {
                    auto ent_hash = entry.name().hash();
                    if (head_hash == ent_hash) {
                        res = Visitor<std::decay_t<decltype(entry)>>::visit(
                            entry, ar, make_sub_provider(ap, 1));
                    }
                }(), ...
            );
            return res; 
        }, self.entries());
    }
};

template<typename Obj>
struct Object{

};


namespace details{
template<typename Ret, typename ArgsTuple, template<typename, typename...> 
    class MethodByLambda, typename Lambda>
struct make_method_by_lambda_impl;

template<typename Ret, template<typename, typename...> 
    class MethodByLambda, typename... Args, typename Lambda>
struct make_method_by_lambda_impl<Ret, std::tuple<Args...>, MethodByLambda, Lambda> {
    static auto make(const StringView name, Lambda&& lambda) {
        return MethodByLambda<Ret, Args...>(
            name,
            std::forward<Lambda>(lambda)
        );
    }
};

}

template<typename Lambda>
auto make_function(const StringView name, Lambda&& lambda) {
    using DecayedLambda = typename std::decay<Lambda>::type;

    using Ret = typename magic::functor_ret_t<DecayedLambda>;
    using ArgsTuple = typename magic::functor_args_tuple_t<DecayedLambda>;

    return details::make_method_by_lambda_impl<Ret, ArgsTuple, MethodByLambda, Lambda>::make(
        name,
        std::forward<Lambda>(lambda)
    );
}


template<typename Ret, typename ... Args>
auto make_function(const StringView name, Ret(*callback)(Args...)) {
    return MethodByLambda<Ret, Args...>(
        name,
        static_cast<Ret(*)(Args...)>(callback)
    );
}

template<typename Ret, typename ... Args, typename TObj>
auto make_memfunc(
    const StringView name, 
    TObj * pobj, 
    Ret(TObj::*member_func_ptr)(Args...)
) {
    return MethodByMemFunc<TObj, Ret, Args...>(
        name,
        pobj,
        member_func_ptr
    );
}

template<typename T>
auto make_property(const StringView name, T * val){
    return Property<T>(
        name, 
        val
    );
}

template<typename T>
auto make_ro_property(const StringView name, const T * val){
    return Property<const T>(
        name, 
        val
    );
}


template<typename ... Args>
auto make_list(const StringView name, Args && ... entries){
    return EntryList<Args...>(
        name, 
        // std::forward<Args>(entries)...
        entries...
    );
}


// 统一访问接口
template <typename T>
AccessResult<> visit(T&& self, AccessReponserIntf& ar, const AccessProviderIntf& ap) {
    return Visitor<std::remove_cvref_t<T>>::visit(
        std::forward<T>(self), ar, ap);
}

}



