#pragma once

#include "uavcan_primitive.hpp"

namespace ymd::uavcan::msgs{


struct ParamCfg final{
    // uint8 esc_index
    // uint32 esc_uuid
    // uint16 esc_id_set
    // uint16 esc_ov_threshold
    // uint16 esc_oc_threshold
    // uint16 esc_ot_threshold
    // uint16 esc_acc_threshold
    // uint16 esc_dacc_threshold
    // int16 esc_rotate_dir
    // uint8 esc_timing
    // uint8 esc_signal_priority
    // uint16 esc_led_mode
    // uint8 esc_can_rate
    // uint16 esc_fdb_rate
    // uint8 esc_save_option
};
}