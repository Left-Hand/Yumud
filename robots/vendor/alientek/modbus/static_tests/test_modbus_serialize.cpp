#include "../pdstepper_modbus_api_facade.hpp"
#include "../pdstepper_modbus_backend.hpp"

using namespace ymd;
using namespace ymd::robots::pdstepper;

namespace{



struct [[nodiscard]] Serializer final{
    using Error = Infallible;

    std::array<uint8_t, 32> bytes;
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
    using ModbusPacketFactory = ClientApiFacade<ModbusPacketFactoryBackend>;
    static constexpr auto factory = ModbusPacketFactory{.state{.node_id = 0x01}};


    {
        constexpr auto srz = [&]{
            auto ret = Serializer{};
            factory.calibrate_encoder().serialize(ret).unwrap();
            return ret;
        }();
        static_assert(srz.length() == 8);
        static_assert(srz.bytes[0]  == 0x01);
        static_assert(srz.bytes[1]  == 0x06); 
        static_assert(srz.bytes[2]  == 0x00);
        static_assert(srz.bytes[3]  == 0x01);
        static_assert(srz.bytes[4]  == 0x00);
        static_assert(srz.bytes[5]  == 0x01);
        static_assert(srz.bytes[6]  == 0x19);
        static_assert(srz.bytes[7]  == 0xca); 
    }

    {
        constexpr auto srz = [&]{
            auto ret = Serializer{};
            factory.get_flux().serialize(ret).unwrap();
            return ret;
        }();
        static_assert(srz.length() == 8);
        static_assert(srz.bytes[0]  == 0x01);
        static_assert(srz.bytes[1]  == 0x04); 
        static_assert(srz.bytes[2]  == 0x00);
        static_assert(srz.bytes[3]  == 0x21);
        static_assert(srz.bytes[4]  == 0x00);
        static_assert(srz.bytes[5]  == 0x02);
        static_assert(srz.bytes[6]  == 0x21);
        static_assert(srz.bytes[7]  == 0xc1); 
    }

    {
        constexpr auto srz = [&]{
            auto ret = Serializer{};
            const auto packet = factory.set_position_pid_paraments({300, 30, 380});
            packet.serialize(ret).unwrap();
            return ret;
        }();

        // 01 10 00 63  00 06 0C 00     00 01 2C 00     00 00 1E 00     00 01 7C 57 79
        static_assert(srz.length() == 21);
        static_assert(srz.bytes[0]  == 0x01);
        static_assert(srz.bytes[1]  == 0x10); 
        static_assert(srz.bytes[2]  == 0x00);
        static_assert(srz.bytes[3]  == 0x63);

        static_assert(srz.bytes[4]  == 0x00);
        static_assert(srz.bytes[5]  == 0x06);
        static_assert(srz.bytes[6]  == 0x0c);
        static_assert(srz.bytes[7]  == 0x00); 

        static_assert(srz.bytes[8]  == 0x00);
        static_assert(srz.bytes[9]  == 0x01);
        static_assert(srz.bytes[10]  == 0x2c);
        static_assert(srz.bytes[11]  == 0x00); 

        static_assert(srz.bytes[12]  == 0x00);
        static_assert(srz.bytes[13]  == 0x00);
        static_assert(srz.bytes[14]  == 0x1e);
        static_assert(srz.bytes[15]  == 0x00); 

        static_assert(srz.bytes[16]  == 0x00);
        static_assert(srz.bytes[17]  == 0x01);
        static_assert(srz.bytes[18]  == 0x7c);
        static_assert(srz.bytes[19]  == 0x57); 
        static_assert(srz.bytes[20]  == 0x79); 
    }


}


}