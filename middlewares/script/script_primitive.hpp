#pragma once

#include <span>
#include <utility>
#include <functional>
#include <utility>
#include <type_traits>

#include "core/tmp/functor.hpp"
#include "core/tmp/reflect/enum.hpp"
#include "core/utils/Result.hpp"
#include "core/string/view/string_view.hpp"
#include "core/string/conv/strconv2.hpp"
#include "core/stream/ostream.hpp"


namespace ymd::script{

enum class [[nodiscard]] EntryAccessError: uint8_t{
    NoArgForSetter,
    NotImplemented,
    NoCallableFounded,
    EmptyArgs,
    ArgsCountMismatch,
    CantModifyImmutable
};


DEF_DERIVE_DEBUG(EntryAccessError)


enum class [[nodiscard]] EntryInteractError: uint8_t{
    ValueIsGreatThanLimit,
    ValueIsLessThanLimit
};

DEF_DERIVE_DEBUG(EntryInteractError)

DEF_ERROR_WITH_KINDS(Error, EntryAccessError, EntryInteractError, strconv2::DestringError)

template<typename T = void>
using IResult = Result<T, Error>;



class AccessProvider_ByStringViews final{
public: 
    AccessProvider_ByStringViews(const std::span<const StringView> views):
        views_(views){;}

    size_t size() const{
        return views_.size();
    }

    StringView operator [](const size_t idx) const {
        return views_[idx];
    }

    AccessProvider_ByStringViews subspan(const size_t idx) const {
        return AccessProvider_ByStringViews(views_.subspan(1));
    }
private:    
    std::span<const StringView> views_;
};


template<typename T>
struct alignas(4) [[nodiscard]] Property{
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
struct alignas(4) [[nodiscard]] PropertyWithLimit final:public Property<T>{
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
        
    union{
        alignas(4) Tuple tuple;
    }ret;

    // 递归情况：处理至少一个索引
    template <size_t I, size_t... Js>
    constexpr Result<void, Error>
    apply(const auto& ap, std::index_sequence<I, Js...>) {
        // 转换当前参数
        using Element = std::tuple_element_t<I, Tuple>;
        static_assert(is_result_v<Element> == false);
        const auto res = exact_one(std::get<I>(ret.tuple), ap[I]);
        if (res.is_err()) {
            return Err(res.unwrap_err());  // 遇到错误立即返回
        }
        // 递归处理剩余参数
        return apply(ap, std::index_sequence<Js...>{});
    }

    // 终止条件：所有索引处理完成
    template <typename... Args>
    constexpr Result<void, Error>
    apply(const auto& /*ap*/, std::index_sequence<>) {
        // 使用完美转发构造目标元组
        return Ok();
    }
private:
    template<typename T>
    static constexpr Result<void, Error> exact_one(T && element, const StringView str){
        const auto res = strconv2::defmt_from_str<std::decay_t<T>>(str);
        if(res.is_err()) return Err(Error(res.unwrap_err())); 
        else{
            element = res.unwrap();
            return Ok();
        }
    }

};


template<typename Tuple>
static constexpr Result<Tuple, Error>
convert_params_to_tuple(const auto & ap) {
    constexpr size_t N = std::tuple_size_v<Tuple>;
    // 参数数量检查
    if (ap.size() != N) {
        return Err(script::EntryAccessError::ArgsCountMismatch);
    }

    // 开始递归转换
    auto helper = ConvertHelper<Tuple>{};
    const auto res = helper.apply(ap, std::make_index_sequence<N>{});
    if(res.is_err()) return Err(res.unwrap_err());
    else{
        return Ok(std::move(helper).ret.tuple);
    }
}



template<typename Ret, typename ... Args>
struct alignas(4) [[nodiscard]] MethodByLambda final{
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
struct alignas(4) [[nodiscard]] MethodByMemFunc final{

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
struct alignas(4) [[nodiscard]] List final{
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
auto make_function(const StringView func_name, Lambda && lambda) {
    using DecayedLambda = std::decay_t<Lambda>;

    using Ret = tmp::functor_ret_t<DecayedLambda>;
    using ArgsTuple = tmp::functor_args_tuple_t<DecayedLambda>;

    return details::make_method_by_lambda_impl<Ret, ArgsTuple, MethodByLambda, Lambda>::make(
        func_name,
        std::forward<Lambda>(lambda)
    );
}

template<typename Ret, typename ... Args, typename TObj>
auto make_memfunc(
    const StringView func_name, 
    TObj * pobj, 
    Ret(TObj::*member_func_ptr)(Args...)
) {
    return MethodByMemFunc<TObj, Ret, Args...>(
        func_name,
        pobj,
        member_func_ptr
    );
}

template<typename T>
auto make_mut_property(const StringView prop_name, T * val){
    static_assert(not std::is_const_v<T>);
    return Property<T>(
        prop_name, 
        val
    );
}

template<typename T>
requires (not std::is_const_v<T>)
auto make_mut_property_with_limit(const StringView prop_name, T * val, auto min, auto max){
    return PropertyWithLimit<T>(
        prop_name, 
        val,
        std::make_pair(static_cast<T>(min), static_cast<T>(max))
    );
}

template<typename T>
auto make_property(const StringView prop_name, const T * val){
    return Property<const T>(
        prop_name, 
        val
    );
}


template<typename ... Args>
auto make_list(const StringView list_name, Args && ... entries){
    return List<Args...>(
        list_name, 
        std::forward<Args>(entries)...
        // entries...
    );
}


// 主模板定义（处理非模板类和默认情况）
template <typename T>
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
requires (!std::is_const_v<T>)
struct EntryVisitor<Property<T>> {
    static IResult<> visit(
        const Property<T> & self, 
        auto & ar,
        auto && ap
    ) {
        if (ap.size() != 1) return Err(EntryAccessError::NoArgForSetter);
        const auto val = ({
            const auto res = strconv2::defmt_from_str<std::decay_t<T>>(ap[0]);
            if(res.is_err()) return Err(res.unwrap_err());
            res.unwrap();
        });
        self.get() = val;
        ar << self.get() << OutputStream::endl();
        return Ok();
    }
};

// Property<T> const特化
template <typename T>
requires (std::is_const_v<T>)
struct EntryVisitor<const Property<T>> {
    static IResult<> visit(
        const Property<T> & self, 
        auto & ar,     
        auto && ap
    ) {
        if (ap.size()) 
            return Err(EntryAccessError::CantModifyImmutable);
        ar << self.get() << OutputStream::endl();
        return Ok();
    }
};

// Property<T> 非const特化
template <typename T>
requires (!std::is_const_v<T>)
struct EntryVisitor<PropertyWithLimit<T>> {
    static IResult<> visit(
        const PropertyWithLimit<T> & self, 
        auto & ar,
        auto && ap
    ) {
        if (ap.size() != 1) return Err(EntryAccessError::NoArgForSetter);
        const auto val = ({
            const auto res = strconv2::defmt_from_str<std::decay_t<T>>(ap[0]);
            if(res.is_err()) return Err(res.unwrap_err());
            res.unwrap();
        });
        if(unlikely(val < self.min()))
            return Err(EntryInteractError::ValueIsLessThanLimit);
        if(unlikely(val > self.max()))
            return Err(EntryInteractError::ValueIsGreatThanLimit);
        self.get() = val;
        ar << self.get() << OutputStream::Endl{};
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
            return Err(EntryAccessError::ArgsCountMismatch);
        }

        const Tup tuple_params = ({
            const auto res = convert_params_to_tuple<Tup>(ap);
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
            return Err(EntryAccessError::ArgsCountMismatch);
        }

        const Tup tuple_params = ({
            const auto res = convert_params_to_tuple<Tup>(ap);
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

        const auto head_hash = ap[0].hash();
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
                            entry, ar, ap.subspan(1));
                    }
                }(), ...
            );
            return res; 
        }, self.entries());
    }
};

// 统一访问接口
template <typename T>
IResult<> visit(T&& self, auto & ar, auto && ap) {
    return EntryVisitor<std::decay_t<T>>::visit(
        std::forward<T>(self), ar, ap);
}

}

#define DEF_CALLABLE_MEMFUNC(func)\
    script::make_memfunc(#func, this, &std::decay_t<decltype(*this)>::func)

