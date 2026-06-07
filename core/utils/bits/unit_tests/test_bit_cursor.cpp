#include "../bit_cursor.hpp"
#include "core/utils/Result.hpp"

using namespace ymd;


namespace{

#define RUN_TEST()



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




// [[maybe_unused]] static void test_bit_cursor(){
//     {

//         uint8_t expected[] = {0x00, 0xF0, 0x00};
//         {
//             std::array<uint8_t, 3> raw = {0xF0};
//             // bit_cursor_store_bits(raw.data(), 8, )
//         }
//     }
// }


}