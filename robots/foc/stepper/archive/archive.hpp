#pragma once

#include "robots/foc/motor_utils.hpp"
#include "../ctrls/ctrls.hpp"

namespace yumud::foc{

namespace MotorUtils{
    scexpr uint8_t build_version = 10;
    scexpr uint8_t drv_type = 42;
    scexpr char drv_branch = 'm';
    scexpr uint8_t year = YEAR;
    scexpr uint8_t month = MONTH;
    scexpr uint8_t day = DAY;
    scexpr uint8_t hour = HOUR;
    scexpr uint8_t minute = MINUTE;

    struct BoardInfo{
        uint8_t bver ;
        uint8_t dtype ;
        char dbranch ;
        uint8_t y ;
        uint8_t m ;
        uint8_t d;
        uint8_t h;
        uint8_t mi;

        void reset(){
            bver = build_version;
            dtype = drv_type;
            dbranch = drv_branch;
            y = year;
            m = month;
            d = day;
            h = hour;
            mi = minute;
        }

        bool is_invalid() const {
            return (y > 30) || (y < 24) || //no one will use this shit after 2030
            (m > 12) || (m == 0) || (d > 31) || (d == 0) || (h > 23) || (h == 0) || (mi > 59) || (mi == 0);
        }

        bool is_empty() const {
            uint8_t * ptr = (uint8_t *)this;
            for(size_t i = 0; i < sizeof(*this); i++){
                if(ptr[i]!= 0){
                    return false;
                }
            }
            return true;
        }

        bool is_latest() const {
            return (y == year && m == month && d == day && h == hour && mi == minute); 
        }

        bool errorless() const {
            return not(is_invalid() || is_empty());
        }
    };

    struct Switches{
        union{
            struct{
                struct{
                    uint8_t cali_done:1;
                    uint8_t cali_every_pwon:1;
                    uint8_t cali_when_update:1;
                };

                struct{
                    uint8_t skip_tone:1;


                };

                struct{
                    uint8_t cmd_mode:1;
                };

                struct{
                    uint8_t auto_shutdown_activation:1;
                    uint8_t shutdown_when_error_occurred:1;
                    uint8_t shutdown_when_warn_occurred:1;
                };
            };
            uint32_t data;
        };

        void reset(){

        }
    };

    struct alignas(256) Archive{
        struct alignas(128){
            std::array<int8_t, 100> cali_map;

            uint8_t node_id;
            uint32_t hashcode;
            BoardInfo board_info;
            Switches switches;
        };

        alignas(16) CurrentFilter::Config curr_config;
        alignas(16) SpeedCtrl::Config spd_config;
        alignas(16) PositionCtrl::Config pos_config;
        alignas(16) SpeedEstimator::Config spe_config;
        
        uint32_t hash() const {
            // sizeof(Archive);
            return hash_impl(cali_map);
        }

        auto & map() {return cali_map;}
        const auto & map() const {return cali_map;}
    };
}

}


namespace yumud{
OutputStream & operator<<(OutputStream & os, const foc::MotorUtils::BoardInfo & bi);
}