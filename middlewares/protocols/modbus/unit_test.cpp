#include "modbus_msgs.hpp"
#include "core/utils/Result.hpp"


using namespace ymd;
using namespace ymd::modbus;

namespace{

struct Receiver{
    using Error = Infallible;

    std::array<uint8_t, 20> bytes;
    size_t ind = 0;

    constexpr Result<void, Error> push_bytes(std::span<const uint8_t> input_bytes){
        // std::copy_n(input_bytes.begin(), input_bytes.end(), bytes.begin());
        for(size_t i = 0; i < input_bytes.size(); i++){
            bytes[ind + i] = input_bytes[i];
        }
        ind += input_bytes.size();
        return Ok();
    }
};

[[maybe_unused]] void test_ser(){
    {
        constexpr auto receiver = []{
            auto ret = Receiver{};
            req_msg::ReadHoldingRegisters{
                .base_addr = 0x1234,
                .quantity = 0x5678,
            }.serialize_context(ret).unwrap();
            return ret;
        }();
        static_assert(receiver.bytes[0] == 0x12);
        static_assert(receiver.bytes[1] == 0x34);
        static_assert(receiver.bytes[2] == 0x56);
        static_assert(receiver.bytes[3] == 0x78);
    }
}
}