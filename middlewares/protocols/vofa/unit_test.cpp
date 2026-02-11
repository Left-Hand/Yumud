#include "vofa_just_float.hpp"
#include <algorithm>

using namespace ymd::vofa;

namespace{

struct Receiver{
    std::array<uint8_t, 20> bytes;
    size_t ind = 0;

    constexpr void push_bytes(std::span<const uint8_t> input_bytes){
        // std::copy_n(input_bytes.begin(), input_bytes.end(), bytes.begin());
        for(size_t i = 0; i < input_bytes.size(); i++){
            bytes[ind + i] = input_bytes[i];
        }
        ind += input_bytes.size();
    }
};

[[maybe_unused]] void test_ser(){
    {
        constexpr auto receiver = []{
            auto ret = Receiver{};
            ser_just_float(ret, 1.0, 2.0, 3, 4.0);
            return ret;
        }();
        static_assert(receiver.bytes[0] == std::bit_cast<std::array<uint8_t, 4>>(1.0)[0]);
        static_assert(receiver.bytes[1] == std::bit_cast<std::array<uint8_t, 4>>(1.0)[1]);
        static_assert(receiver.bytes[2] == std::bit_cast<std::array<uint8_t, 4>>(1.0)[2]);
        static_assert(receiver.bytes[3] == std::bit_cast<std::array<uint8_t, 4>>(1.0)[3]);

        static_assert(receiver.bytes[8 + 0] == std::bit_cast<std::array<uint8_t, 4>>(3.0)[0]);
        static_assert(receiver.bytes[8 + 1] == std::bit_cast<std::array<uint8_t, 4>>(3.0)[1]);
        static_assert(receiver.bytes[8 + 2] == std::bit_cast<std::array<uint8_t, 4>>(3.0)[2]);
        static_assert(receiver.bytes[8 + 3] == std::bit_cast<std::array<uint8_t, 4>>(3.0)[3]);

        static_assert(receiver.bytes[16 + 0] == 0x00);
        static_assert(receiver.bytes[16 + 1] == 0x00);
        static_assert(receiver.bytes[16 + 2] == 0x80);
        static_assert(receiver.bytes[16 + 3] == 0x7f);
    }
}


}