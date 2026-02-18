#pragma once

namespace ymd::dsp::motor_ctl { 
// struct StallDetector{
//     iq16 max_position_error;
//     iq16 stall_speed_threshold;
//     uint32_t entry_time_ms;
//     uint32_t timeout_ms;

//     static constexpr uint32_t no_stall_time_ms = 0;

//     bool update(const iq16 target_position, const iq16 measured_position, const iq16 measured_speed){
//         auto current_time_ms = millis();
//         if(abs(measured_speed) < stall_speed_threshold){ // stall means low speed
//             if(entry_time_ms == no_stall_time_ms){
//                 if(abs(target_position - measured_position) > max_position_error){ //
//                     entry_time_ms = current_time_ms;
//                 }
//             }else{
//                 return (current_time_ms - entry_time_ms> timeout_ms);
//             }
//         }else{
//             entry_time_ms = no_stall_time_ms;
//         }
//         return false;
//     }
// };

}