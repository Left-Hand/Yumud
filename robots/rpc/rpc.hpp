#pragma once

#include <span>
#include <utility>
#include <functional>
#include <utility>
#include <type_traits>

#include "core/magic/function_traits.hpp"
#include "core/magic/enum_traits.hpp"
#include "core/utils/Result.hpp"
#include "core/string/string_view.hpp"
#include "core/string/utils/strconv2.hpp"
#include "core/stream/ostream.hpp"

namespace ymd::magic{
// 辅助类型特征检测
template <template<typename...> class, typename...>
struct is_instantiation_of : std::false_type {};

template <template<typename...> class U, typename... Ts>
struct is_instantiation_of<U, U<Ts...>> : std::true_type {};

template <typename T, template<typename...> class U>
using is_instantiation_of_t = typename is_instantiation_of<U, T>::type;
}


namespace ymd::rpc{

enum class EntryAccessError: uint8_t{
    NoArgForSetter,
    NotImplemented,
    NoCallableFounded,
    EmptyArgs,
    ArgsCountNotMatch,
    CantModifyReadOnly
};


DEF_DERIVE_DEBUG(EntryAccessError)


enum class EntryInteractError: uint8_t{
    ValueIsGreatThanLimit,
    ValueIsLessThanLimit
};

DEF_DERIVE_DEBUG(EntryInteractError)

DEF_ERROR_WITH_KINDS(Error, EntryAccessError, EntryInteractError, strconv2::DestringError)

template<typename T = void>
using IResult = Result<T, Error>;


struct ParamFromString final{
    constexpr explicit ParamFromString(const char * str):
        str_(str){;}

    constexpr explicit ParamFromString(const StringView str):
        str_(str){;}

    template<typename T>
    constexpr IResult<T> defmt_to() const{
        const auto res = strconv2::defmt_str<T>(str_);
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok(res.unwrap());
    }

    friend OutputStream & operator << (OutputStream & os, const ParamFromString & param){
        return os << param.str_;
    }

    constexpr StringView str() const{return str_;}
private:
    StringView str_;
};

using Param = ParamFromString;
using Params = std::span<const ParamFromString>;


class AccessProviderIntf{
public:
    virtual size_t size() const = 0;
    virtual ParamFromString operator[](size_t idx) const = 0;
};

// 先定义 SubHelper（不依赖 AccessProviderIntf 的完整定义）
class AccessProvider_BySubSpan final : public AccessProviderIntf {
public:
    constexpr AccessProvider_BySubSpan(
        const AccessProviderIntf & provider, 
        size_t offset, 
        size_t end
    ): 
        provider_(provider), 
        offset_(offset), 
        end_(end){;}

    constexpr size_t size() const {return end_ - offset_;}

    constexpr ParamFromString operator[](size_t idx) const{
        if(idx >= size()) __builtin_unreachable();
        return ParamFromString(provider_[offset_ + idx]);
    }
private:
    const AccessProviderIntf & provider_;
    const size_t offset_;
    const size_t end_;
};

static constexpr AccessProvider_BySubSpan make_sub_provider(
    const AccessProviderIntf & owner, 
    const size_t offset, 
    const size_t end
){
    return AccessProvider_BySubSpan(owner, offset, end);
}

static constexpr AccessProvider_BySubSpan make_sub_provider(
    const AccessProviderIntf & owner, 
    const size_t offset
){
    return AccessProvider_BySubSpan(owner, offset, owner.size());
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


template<typename T>
struct Property{
    constexpr explicit Property(
        const StringView name,
        T * value
    ):
        name_(name),
        value_(value){;}

    [[nodiscard]] constexpr T & get() const {
        return *value_;
    }

    [[nodiscard]] constexpr StringView name() const{
        return name_;
    }
private:
    StringView name_;
    T * value_;
};

template<typename T>
struct PropertyWithLimit final:public Property<T>{
    static_assert(std::is_const_v<T> == false, "value must be setable");

    constexpr explicit PropertyWithLimit(
        const StringView name,
        T * value, 
        std::pair<T, T> limits
    ):
        Property<T>(name, value),
        limits_(limits)
        {;}

    [[nodiscard]] constexpr T min() const {
        return limits_.first;
    }

    [[nodiscard]] constexpr T max() const {
        return limits_.second;
    }
private:
    std::pair<T, T> limits_;
};



// 递归转换辅助类

template<typename Tuple>
struct ConvertHelper {
    // 递归情况：处理至少一个索引
    template <size_t I, size_t... Js, typename... Args>
    static constexpr Result<Tuple, Error>
    apply(const auto& ap, std::index_sequence<I, Js...>, Args&&... args) {
        // 转换当前参数
        using Element = std::tuple_element_t<I, Tuple>;
        static_assert(is_result_v<Element> == false);
        Result<Element, Error> res = ParamFromString(ap[I]).template defmt_to<Element>();
        if (res.is_err()) {
            return Err(res.unwrap_err());  // 遇到错误立即返回
        }
        // 递归处理剩余参数
        return apply(ap, std::index_sequence<Js...>{}, 
                        std::forward<Args>(args)..., res.unwrap());
    }
private:
    // 终止条件：所有索引处理完成
    template <typename... Args>
    static constexpr Result<Tuple, Error>
    apply(const auto& /*ap*/, std::index_sequence<>, Args&&... args) {
        // 使用完美转发构造目标元组
        return Ok(Tuple{std::forward<Args>(args)...});
    }
};


template<typename Tuple, std::size_t... Is>
static constexpr Result<Tuple, Error>
convert_params_to_tuple(const auto & ap, std::index_sequence<Is...>) 
{
    // 参数数量检查
    if (ap.size() != sizeof...(Is)) {
        return Err(rpc::EntryAccessError::ArgsCountNotMatch);
    }

    // 开始递归转换
    return ConvertHelper<Tuple>::apply(ap, std::index_sequence<Is...>{});
}

#if 0
[[maybe_unused]] static void static_test(){
    {
        constexpr std::array ap = {"1", "2"}; 

        constexpr auto result = convert_params_to_tuple<std::tuple<int, int>>(ap, std::make_index_sequence<2>{});    
        static_assert(result.is_ok());
        static_assert(std::get<0>(result.unwrap()) == 1);
        static_assert(std::get<1>(result.unwrap()) == 2);
    }

    {
        constexpr std::array ap = {"1"}; 

        constexpr auto result = convert_params_to_tuple<std::tuple<int>>(ap, std::make_index_sequence<1>{});    
        static_assert(result.is_ok());
        static_assert(std::get<0>(result.unwrap()) == 1);
    }
}
#endif

template<typename Ret, typename ... Args>
struct MethodByLambda final{
public:
    static constexpr size_t N = sizeof...(Args);
    
    using Tup = std::tuple<Args...>;
    using Callback = std::function<Ret(Args...)>;

    constexpr explicit MethodByLambda(
        const StringView name, 
        const Callback && callback
    )
        : name_(name), callback_(callback) {}

    [[nodiscard]] constexpr StringView name() const {
        return name_;
    }

    constexpr Ret invoke(const Tup & tup) const {
        return std::apply(callback_, tup);
    }
private:
    StringView name_;
    Callback callback_;
};


template<typename Obj, typename Ret, typename ... Args>
struct MethodByMemFunc final{

    static constexpr size_t N = sizeof...(Args);

    using Callback = Ret (Obj::*)(Args...);

    constexpr MethodByMemFunc(
        const StringView name, 
        Obj * obj, 
        Callback callback
    ) : name_(name), 
        obj_(obj),
        callback_(callback) {}

    constexpr StringView name() const{return name_;}

    // 新增的 invoke 方法
    template<typename... InvokeArgs>
    constexpr auto invoke(InvokeArgs&&... args) const {
        static_assert(sizeof...(InvokeArgs) == sizeof...(Args), "Argument count mismatch");
        return std::invoke(callback_, obj_, std::forward<InvokeArgs>(args)...);
    }
private:
    StringView name_;
    Obj * obj_;
    Callback callback_;
};


template<typename... Entries>
struct List final{
    explicit List(const StringView name):
        name_(name){;}

    template<typename... Args>
    explicit List(const StringView name, Args&&... entries) :
        name_(name),
        entries_(std::forward<Args>(entries)...) {
            const auto check_res = check_hash_collision(entries...);
            if(check_res.is_err()) 
                // PANIC("Hash collision detected", check_res.unwrap_err());
                while(true);
        } 


    constexpr const auto & entries() const { return entries_; } 

    constexpr StringView name() const { return name_; }
private:
    StringView name_;
    std::tuple<Entries...> entries_;

    [[nodiscard]] constexpr Result<void, std::tuple<size_t, size_t>> 
    check_hash_collision(auto && ... entries) {
        const std::array hashes = { entries.name().hash()... };
        for (size_t i = 0; i < hashes.size(); ++i) {
            for (size_t j = i + 1; j < hashes.size(); ++j) {
                if (hashes[i] == hashes[j]) {
                    return Err(std::make_tuple(i, j));
                }
            }
        }
        return Ok();
    }
};




// 主模板定义（处理非模板类和默认情况）
template <typename T, typename = void>
struct EntryVisitor {
    static IResult<> visit(
        T& self, 
        auto & ar,
        auto && ap
    ) {
        static_assert(sizeof(T) == 0, "No visitor specialization found for this type");
        return Err(EntryAccessError::NotImplemented);
    }
};


// Property<T> 非const特化
template <typename T>
struct EntryVisitor<Property<T>, std::enable_if_t<!std::is_const_v<T>>> {
    static IResult<> visit(
        const Property<T> & self, 
        auto & ar,
        auto && ap
    ) {
        if (ap.size() != 1) return Err(EntryAccessError::NoArgForSetter);
        self.deref() = ap[0].template defmt_to<std::decay_t<T>>();
        ar << self.deref() << ar.endl();
        return Ok();
    }
};

// Property<T> const特化
template <typename T>
struct EntryVisitor<const Property<T>, void> {
    static IResult<> visit(
        const Property<T> & self, 
        AccessReponserIntf & ar,     
        const AccessProviderIntf & ap
    ) {
        if (ap.size()) 
            return Err(EntryAccessError::CantModifyReadOnly);
        ar << self.deref() << ar.endl();
        return Ok();
    }
};

// Property<T> 非const特化
template <typename T>
struct EntryVisitor<PropertyWithLimit<T>, std::enable_if_t<!std::is_const_v<T>>> {
    static IResult<> visit(
        const PropertyWithLimit<T> & self, 
        auto & ar,
        auto && ap
    ) {
        if (ap.size() != 1) return Err(EntryAccessError::NoArgForSetter);
        const auto val = ({
            const auto res = ap[0].template defmt_to<std::decay_t<T>>();
            if(unlikely(res.is_err())) return Err(res.unwrap_err());
            res.unwrap();
        });
        if(unlikely(val < self.min()))
            return Err(EntryInteractError::ValueIsLessThanLimit);
        if(unlikely(val > self.max()))
            return Err(EntryInteractError::ValueIsGreatThanLimit);
        self.get() = val;
        ar << self.get() << ar.endl();
        return Ok();
    }
};


template <typename Ret, typename... Args>
struct EntryVisitor<MethodByLambda<Ret, Args...>> final{
    using Self = MethodByLambda<Ret, Args...>;
    using Tup = std::tuple<Args...>;

    static IResult<> visit(const Self & self, 
        auto & ar,
        auto && ap
    ) {
        if (ap.size() != sizeof...(Args)) {
            return Err(EntryAccessError::ArgsCountNotMatch);
        }

        const Tup tuple_params = ({
            const auto res = dump(ap);
            if(res.is_err()) return Err(res.unwrap_err());
            res.unwrap();
        });

        if constexpr(std::is_void_v<Ret>){
            self.invoke(tuple_params);
        } else {
            ar << self.invoke(tuple_params);
        }
        return Ok();
    }
private:
    static IResult<Tup> dump( 
        auto & ap
    ){
        if constexpr (std::is_same_v<Tup, std::tuple<>>)
            return Ok(Tup{});
        else return ({
            const auto res = convert_params_to_tuple<Tup>(
                ap, std::index_sequence_for<Args...>{});
            if(res.is_err()) return Err(res.unwrap_err());
            Ok(res.unwrap());
        });
    }
};

// MethodByMemFunc 非const特化
template <typename Obj, typename Ret, typename ... Args>
struct EntryVisitor<MethodByMemFunc<Obj, Ret, Args...>> final {
    using Self = MethodByMemFunc<Obj, Ret, Args...>;
    using Tup = std::tuple<Args...>;


    static IResult<> visit(const Self & self, 
        auto & ar,
        auto && ap
    ) {
        if (ap.size() != sizeof...(Args)) {
            return Err(EntryAccessError::ArgsCountNotMatch);
        }

        const Tup tuple_params = ({
            const auto res = dump(ap);
            if(res.is_err()) return Err(res.unwrap_err());
            res.unwrap();
        });


        if constexpr(std::is_void_v<Ret>){
            std::apply([&self](Args... args) { self.invoke(args...); }, tuple_params);
        } else {
            ar << std::apply([&self](Args... args) -> Ret { return self.invoke(args...); }, tuple_params);
        }
        return Ok();
    }

private:
    static IResult<Tup> dump( 
        auto & ap
    ){
        if constexpr (std::is_same_v<Tup, std::tuple<>>)
            return Ok(Tup{});
        else return ({
            const auto res = convert_params_to_tuple<Tup>(
                ap, std::index_sequence_for<Args...>{});
            if(res.is_err()) return Err(res.unwrap_err());
            Ok(res.unwrap());
        });
    }
};

template<typename... Entries>
struct EntryVisitor<List<Entries...>> final{
    using Self = List<Entries...>;
    static IResult<> visit(const Self & self, 
        auto & ar,
        auto && ap
    ) {
        if(ap.size() == 0) 
            return Err(EntryAccessError::EmptyArgs);

        const auto head_hash = ap[0].str().hash();
        // Modify the first block for "ls" command
        if (head_hash == "ls"_ha) {
            std::apply([&ar](auto&&... entry) { ar.println(entry.name()...); }, self.entries());
            return Ok();
        }
        return std::apply([&](auto&&... entry) -> IResult<> {
            IResult<> res = Err(EntryAccessError::NoCallableFounded);
            ( [&]() -> void {
                    auto ent_hash = entry.name().hash();
                    if (head_hash == ent_hash) {
                        res = EntryVisitor<std::decay_t<decltype(entry)>>::visit(
                            entry, ar, make_sub_provider(ap, 1));
                    }
                }(), ...
            );
            return res; 
        }, self.entries());
    }
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
requires (not std::is_const_v<T>)
auto make_property_with_limit(const StringView name, T * val, auto min, auto max){
    return PropertyWithLimit<T>(
        name, 
        val,
        std::make_pair(static_cast<T>(min), static_cast<T>(max))
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
    return List<Args...>(
        name, 
        // std::forward<Args>(entries)...
        entries...
    );
}


// 统一访问接口
template <typename T>
IResult<> visit(T&& self, AccessReponserIntf & ar, const AccessProviderIntf & ap) {
    return EntryVisitor<std::remove_cvref_t<T>>::visit(
        std::forward<T>(self), ar, ap);
}

}

#define DEF_RPC_MEMFUNC(func)\
    rpc::make_memfunc(#func, this, &std::decay_t<decltype(*this)>::func)

