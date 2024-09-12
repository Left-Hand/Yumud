#ifndef __STEPPER_ARCHIVE_HPP__

#define __STEPPER_ARCHIVE_HPP__

#include "../constants.hpp"

//at24c02 can max contain 256 bytes

namespace StepperUtils{
    static constexpr uint8_t build_version = 10;
    static constexpr uint8_t drv_type = 42;
    static constexpr char drv_branch = 'm';
    static constexpr uint8_t year = YEAR;
    static constexpr uint8_t month = MONTH;
    static constexpr uint8_t day = DAY;
    static constexpr uint8_t hour = HOUR;
    static constexpr uint8_t minute = MINUTE;

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

    struct Archive{

        union{
            struct{
                struct alignas(128){
                    struct alignas(16){
                        uint32_t hashcode;
                        BoardInfo board_info;
                        Switches switches;
                    };

                    struct alignas(16){
                        uint8_t node_id;
                    };
                };

                struct alignas(128){
                    std::array<int16_t, 50> cali_map;
                };
            };

            uint8_t data[256];
        };


        uint32_t hash() const {
            return hash_impl(cali_map);
        }

        auto & map() {return cali_map;}
        const auto & map() const {return cali_map;}
        
        void clear(){
            memset(this, 0, sizeof(*this));
        }
    };
}

OutputStream & operator<<(OutputStream & os, const StepperUtils::BoardInfo & bi);

#endif