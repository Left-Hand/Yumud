#pragma once

namespace ymd{

template<typename Fn>
class ScopeGuard{
public:
    constexpr ScopeGuard(Fn && fn):
        fn_(std::move(fn)){}
    constexpr ~ScopeGuard(){
        fn_();
    }
private:
    Fn fn_;
};


template<typename Fn>
static constexpr auto make_scope_guard(Fn && fn){
    return ScopeGuard<std::remove_reference_t<Fn>>(std::forward<Fn>(fn));
}

}