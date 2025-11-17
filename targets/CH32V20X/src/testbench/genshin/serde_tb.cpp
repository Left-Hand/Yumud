#include "core/utils/serde.hpp"
#include "core/utils/stdrange.hpp"
#include "core/utils/data_iter.hpp"


using namespace ymd;


[[maybe_unused]] static void static_test(){
    auto count_iter_size = [](auto iter) -> size_t{
        size_t size = 0;
        while(iter.has_next()){
            (void)iter.next();
            size++;
        }
        return size;
    };

    {
        static constexpr std::array buf = {uint8_t(0x1234), uint8_t(0x5678)};
        static constexpr auto iter = serde::make_serialize_generator<serde::RawLeBytes>(std::span(buf));
        static constexpr auto iter_size = count_iter_size(iter);
        static_assert(iter_size == 2);
    }

    {
        static constexpr std::array buf = {uint32_t(0x1234), uint32_t(0x5678), uint32_t(0x5678)};
        static constexpr auto iter = serde::make_serialize_generator<serde::RawLeBytes>(std::span(buf));
        static constexpr auto iter_size = count_iter_size(iter);
        static_assert(iter_size == 12);
    }

    {
        static constexpr auto buf = {uint32_t(0x1234), uint32_t(0x5678), uint32_t(0x5678)};
        static constexpr auto iter = serde::make_serialize_generator<serde::RawLeBytes>(std::span(buf));
        static constexpr auto iter_size = count_iter_size(iter);
        static_assert(iter_size == 12);
    }

    {
        static constexpr auto iter = RepeatIter<uint8_t, 4>(0, 4);
        static constexpr auto iter_size = count_iter_size(iter);
        static_assert(iter_size == 4);
    }
}
