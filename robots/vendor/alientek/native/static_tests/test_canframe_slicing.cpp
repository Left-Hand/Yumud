#include "../pdstepper_canframe_iterator.hpp"

using namespace ymd;
using namespace ymd::robots::pdstepper;


namespace{

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

static constexpr CanframeSlicingIterator make_iter(std::span<const uint8_t> bytes){
    return CanframeSlicingIterator(0x01, bytes);
};

[[maybe_unused]] static void test_canframe_slicing(){


    {
        static constexpr uint8_t buf[] = {
            0x01, 
            0x02, 
            0x03, 
            0x04, 
            0x05, 
            0x06, 
            0x07, 
        };

        static constexpr auto frame0 = get_nth_result(0, make_iter(buf));

        static_assert(frame0.length() == 7);
        static_assert(frame0.payload_bytes()[6] == 0x07);
    }


    {
        static constexpr uint8_t buf[] = {
            0x01, 
            0x02, 
            0x03, 
            0x04, 
            0x05, 
            0x06, 
            0x07,
            0x08 
        };

        static constexpr auto frame0 = get_nth_result(0, make_iter(buf));

        static_assert(frame0.length() == 8);
        static_assert(frame0.payload_bytes()[7] == 0x08);
    }

    {
        static constexpr uint8_t buf[] = {
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
            0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        };

        static constexpr auto frame1 = get_nth_result(1, make_iter(buf));

        static_assert(frame1.length() == 8);
        static_assert(frame1.payload_bytes()[7] == 0x18);
    }
}
}