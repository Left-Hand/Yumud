#include "../cobs.hpp"

using namespace ymd;


namespace{

struct TestReceiver{
    using Error = Infallible;
    std::array<uint8_t, 20> bytes;
    size_t ind = 0;

    constexpr Result<void, Infallible> push_bytes(std::span<const uint8_t> input_bytes){
        // std::copy_n(input_bytes.begin(), input_bytes.end(), bytes.begin());
        for(size_t i = 0; i < input_bytes.size(); i++){
            bytes[ind + i] = input_bytes[i];
        }
        ind += input_bytes.size();
        return Ok();
    }
};


[[maybe_unused]] void test_cobs_ser(){
    {
        static constexpr std::array<uint8_t, 6> input = {1,2,7,3, 5, 6};
        cobs::CobsEncoder encoder;
        static constexpr TestReceiver receiver = [&]{
            TestReceiver _receiver;
            if(const auto res = encoder.encode(_receiver, input);
                res.is_err()) __builtin_trap();
            return _receiver;
        }();

        static_assert(receiver.ind == 7);
        static_assert(receiver.bytes[0] == 7);
        static_assert(receiver.bytes[1] == 1);
        static_assert(receiver.bytes[2] == 2);
        static_assert(receiver.bytes[3] == 7);
        static_assert(receiver.bytes[4] == 3);
        static_assert(receiver.bytes[5] == 5);
        static_assert(receiver.bytes[6] == 6);
    }
}


// 添加解码测试
[[maybe_unused]] void test_cobs_deser() {
    // 测试解码原始的 {1,2,0,3,5,6}
    static constexpr std::array<uint8_t, 7> encoded_input = {7, 1, 2, 7, 3, 5, 6};
    

    static constexpr TestReceiver receiver = [&]{
        TestReceiver _receiver;
        if(const auto res = cobs::cobs_decode(_receiver, encoded_input);
            res.is_err()) __builtin_trap();
        return _receiver;
    }();
    
    static_assert(receiver.ind == 6);  // 原始数据长度
    static_assert(receiver.bytes[0] == 1);
    static_assert(receiver.bytes[1] == 2);
    static_assert(receiver.bytes[2] == 7);
    static_assert(receiver.bytes[3] == 3);
    static_assert(receiver.bytes[4] == 5);
    static_assert(receiver.bytes[5] == 6);
}


}