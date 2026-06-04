#include "zdt_stepper_msg.hpp"

using namespace ymd;
using namespace ymd::robots::zdtmotor;


namespace {
template<typename Iter>
static constexpr size_t count_iter(Iter && iter){
    size_t count = 0;
    while(iter.has_next()){
        (void)iter.next();
        ++count;
    }
    return count;
}

template<typename Iter>
static constexpr auto get_nth_result(const size_t i, Iter && iter){
    size_t count = 0;
    while(iter.has_next()){
        auto result = iter.next();
        if(count == i){
            return static_cast<decltype(result)>(result);
        }
        ++count;
    }
    // 如果索引超出范围，可以返回默认值或触发错误
    __builtin_trap();
}

}

namespace{

[[maybe_unused]] static void test_slice(){

}

[[maybe_unused]] static void test_ser_msg(){

}



}

