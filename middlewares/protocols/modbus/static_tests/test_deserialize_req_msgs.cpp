#include "../modbus_msgs.hpp"


using namespace ymd;
using namespace ymd::modbus;

namespace{

struct [[nodiscard]] Desrz final{
    using Error = Infallible;

    std::span<const uint8_t> bytes;
    size_t ind = 0;

    constexpr Result<void, Error> compatible_with_length(size_t n) const {
        return Ok();
    }

    constexpr const uint8_t * take_cursor_and_inc(const size_t n) {
        const size_t next_ind = ind + n;
        auto ptr = bytes.data() + ind;
        ind = next_ind;
        return ptr;
    }
};

static constexpr Desrz make_dsrz(std::span<const uint8_t> buf){
    return Desrz(buf);
}

#define MAKE_DSRZ(buf) make_dsrz(std::span(buf))

[[maybe_unused]] static void test_de0x01(){

    {
        static constexpr uint8_t buf[] = {
            0x00,
            0x00,
            0x00,
            0x02
        };

        static constexpr auto msg = []{
            auto dsrz = MAKE_DSRZ(buf);
            return req_msgs::ReadCoils::deserialize_from(dsrz).unwrap();
        }();

        static_assert(msg.base_address == 0x00);
        static_assert(msg.quantity == 0x02);
    }
}

}