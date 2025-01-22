// #pragma once

// #include "sys/math/real.hpp"

// namespace ymd{

// class PRCtrl{
//     struct Config{
//         iq_t Kp;
//         iq_t Kr;
//         iq_t wc;
//         iq_t wo;
//         size_t freq;
//     };

//     iq_t vo, vo_1, vo_2, vi_1, vi_2;
//     iq_t B0, B1, B2, A1, A2;

//     PRCtrl(const Config& config){
//         iq_t temp = 0;
//         temp = 4 * config.freq * config.freq + 4 * config.wc * config.freq + config.wo * config.wo;

//         B0 = (4 * Kp * config.freq * config.freq + 4 * wc * (Kp + Kr) * config.freq
//                 + Kp * wo * wo) / temp;
//         B1 = (-8 * Kp * config.freq * config.freq + 2 * Kp * wo * wo) / temp;
//         B2 = (4 * Kp * config.freq * config.freq - 4 * wc * config.freq * (Kp + Kr)
//                 + Kp * wo * wo) / temp;
//         A1 = (-8 * config.freq * config.freq + 2 * wo * wo) / temp;
//         A2 = (4 * config.freq * config.freq - 4 * wc * config.freq + wo * wo) / temp;
//     }

//     void update(const iq_t vi){
//         vo = -A1 * vo_1 - A2 * vo_2 + B0 * vi + B1 * vi_1
//                 + B2 * vi_2;

//         vo_2 = vo_1;
//         vo_1 = vo;
//         vi_2 = vi_1;
//         vi_1 = vi;
//     }
// };

// }
