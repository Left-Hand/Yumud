#include "cybergear.hpp"

namespace ymd::robots::cybergear{

struct [[nodiscard]] CyberGearRx{

    struct [[nodiscard]] Feedback final{
        iq16 radians;
        iq16 omega;
        iq16 torque;
        iq16 celsius;
    };

    StatusBitFields fault_ = {};
    Option<uint64_t> device_mcu_id_ = None;


    Feedback feedback_ = {};

    [[nodiscard]] Option<uint64_t> get_device_mcu_id() const noexcept {return device_mcu_id_;}

    IResult<> on_mcu_id_feed_back(const uint32_t id, const uint64_t data, const uint8_t dlc);
    IResult<> on_ctrl2_feed_back(const uint32_t id, const uint64_t data, const uint8_t dlc);
    IResult<> on_read_para_feed_back(const uint32_t id, const uint64_t data, const uint8_t dlc);

    IResult<> on_receive(const hal::ClassicCanFrame & frame);
};



}