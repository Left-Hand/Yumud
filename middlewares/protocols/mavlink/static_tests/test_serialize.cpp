#include "../mavlink_serialize.hpp"

using namespace ymd;
using namespace ymd::mavlink;


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

    constexpr size_t length() const {return ind;}
};



// https://blog.csdn.net/qq_51566076/article/details/149910138


[[maybe_unused]] static void test_serialze_tailer(){

    {
        //header v1

        static constexpr uint8_t buf[] = {
            0x09, 0x00, 0x00, 0x38, 0x01,
            0x01, 
            0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x51, 0x03, 0x03,
        };

        static constexpr uint8_t crc_extra = 50;

        constexpr auto srz = []{
            auto ret = Serializer{};

            serialize_tailer(ret, std::span(buf), crc_extra).unwrap();
            return ret;
        }();


        static_assert(srz.length() == 2);
        static_assert(srz.bytes[0]  == 0xaf);
        static_assert(srz.bytes[1]  == 0x28);

    }
}


}