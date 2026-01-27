#pragma once

namespace ymd{
template<typename T, typename Fn>
struct ForeachIter{
    T iter_;
    Fn fn_;

    constexpr bool has_next() const{
        return iter_.has_next();
    }

    constexpr auto next(){
        return fn_(iter_.next());
    }
};

template<typename Fn>
struct ForeachIterProsoma{
    using Self = ForeachIterProsoma<Fn>;

    Fn fn_;

    template<typename T>
    friend constexpr auto operator | (T && iter, Self && self){
        return ForeachIter<std::decay_t<T>, std::decay_t<Fn>>{
            std::forward<T>(iter), 
            std::forward<Fn>(self.fn_)
        };
    }
};

template<typename Fn>
constexpr auto foreach(Fn && fn){ 
    return ForeachIterProsoma<std::decay_t<Fn>>{
        std::forward<Fn>(fn)
    };
}

}