#include "pdstepper_modbus_api_facade.hpp"

using namespace ymd;
using namespace ymd::robots::pdstepper;

namespace{



struct [[nodiscard]] Serializer final{
    using Error = Infallible;

    std::array<uint8_t, 20> bytes;
    size_t ind = 0;

    constexpr Result<void, Error> push_bytes(std::span<const uint8_t> input_bytes){
        for(size_t i = 0; i < input_bytes.size(); i++){
            bytes[ind + i] = input_bytes[i];
        }
        ind += input_bytes.size();
        return Ok();
    }

    constexpr std::span<const uint8_t> collected_bytes() const {
        return std::span(bytes.data(), ind);
    }

    constexpr Result<void, Error> compatible_with_length(size_t n) const {
        return Ok();
    }

    constexpr uint8_t * take_cursor_and_inc(const size_t n) {
        const size_t next_ind = ind + n;
        auto ptr = bytes.data() + ind;
        ind = next_ind;
        return ptr;
    }

    constexpr size_t length() const {return ind;}
};


[[maybe_unused]] static void test_modbus_serialize(){
    static constexpr auto factory = ClientApiFacade<ModbusPacketBackend>{.state{.node_id = 0x01}};
    {
        constexpr auto serializer = [&]{
            auto ret = Serializer{};
            factory.calibrate_encoder().serialize(ret).unwrap();
            return ret;
        }();
        static_assert(serializer.length() == 8);
        static_assert(serializer.bytes[0]  == 0x01);
        static_assert(serializer.bytes[1]  == 0x06); 
        static_assert(serializer.bytes[2]  == 0x00);
        static_assert(serializer.bytes[3]  == 0x01);
        static_assert(serializer.bytes[4]  == 0x00);
        static_assert(serializer.bytes[5]  == 0x01);
        static_assert(serializer.bytes[6]  == 0x19);
        static_assert(serializer.bytes[7]  == 0xca); 
    }

    {
        constexpr uint8_t nodeid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = modbus::req_msgs::ReadInputRegisters{
                .base_addr = 0x21,
                .quantity = 0x02,
            };
            modbus::serialize_rtu_msg(ret, msg, nodeid).unwrap();
            return ret;
        }();
        static_assert(serializer.length() == 8);
        static_assert(serializer.bytes[0]  == 0x01);
        static_assert(serializer.bytes[1]  == 0x04); 
        static_assert(serializer.bytes[2]  == 0x00);
        static_assert(serializer.bytes[3]  == 0x21);
        static_assert(serializer.bytes[4]  == 0x00);
        static_assert(serializer.bytes[5]  == 0x02);
        static_assert(serializer.bytes[6]  == 0x21);
        static_assert(serializer.bytes[7]  == 0xc1); 
    }
}


}