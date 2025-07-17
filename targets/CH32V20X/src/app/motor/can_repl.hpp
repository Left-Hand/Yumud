#pragma once

namespace ymd::can_repl{

template<typename Obj, typename E, typename M>
struct MsgDispatcher {

};


template<typename E>
requires (std::is_enum_v<E> and std::is_same_v<std::underlying_type_t<E>, uint8_t>)
struct ReplDict{
    static constexpr size_t N = ymd::magic::enum_count_v<E>;
private:
    // InlinveVector<CanMsgDispatcher<E>, N> dict_;
    // EnumDict<E> dict_;
};


}