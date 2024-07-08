#ifndef __STEPPER_ARCHIVE_HPP__

#define __STEPPER_ARCHIVE_HPP__

#include "constants.hpp"
#include "sys/kernel/stream.hpp"

//at24c02 can max contain 256 bytes

#define YEAR (((__DATE__[9]-'0')) * 10 + (__DATE__[10]-'0'))
#define MONTH (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n' ? 1 : \
            __DATE__[0] == 'F' ? 2 : \
            __DATE__[0] == 'M' && __DATE__[2] == 'r' ? 3 : \
            __DATE__[0] == 'A' && __DATE__[1] == 'p' ? 4 : \
            __DATE__[0] == 'M' ?  5 : \
            __DATE__[0] == 'J' && __DATE__[1] == 'u' ? 6 : \
            __DATE__[0] == 'J' ? 7 : \
            __DATE__[0] == 'A' ? 8 : \
            __DATE__[0] == 'S' ? 9 : \
            __DATE__[0] == 'O' ? 10 : \
            11)

#define DAY ((__DATE__[4] == ' ' ? 0 : __DATE__[4]-'0') * 10 + (__DATE__[5]-'0'))
#define HOUR ((__TIME__[0]-'0') * 10 + __TIME__[1]-'0')
#define MINUTE ((__TIME__[3]-'0') * 10 + __TIME__[4]-'0')

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

        void construct(){
            bver = build_version;
            dtype = drv_type;
            dbranch = drv_branch;
            y = year;
            m = month;
            d = day;
            h = hour;
            mi = minute;
        }

        bool broken() const {
            return (y > 30) || (y < 24) || //no one will use this shit after 2030
            (m > 12) || (m == 0) || (d > 31) || (d == 0) || (h > 23) || (h == 0) || (mi > 59) || (mi == 0);
        }

        bool empty() const {
            uint8_t * ptr = (uint8_t *)this;
            for(size_t i = 0; i < sizeof(*this); i++){
                if(ptr[i]!= 0){
                    return false;
                }
            }
            return true;
        }

        bool match() const {
            return (y == year && m == month && d == day && h == hour && mi == minute); 
        }

        bool errorless() const {
            return not(broken() || empty());
        }

        void printout(IOStream & logger) const {
            logger << "build version:\t\t" << this->bver << "\r\n";
            logger << "build time:\t\t20" << 
                    this->y << '/' << this->m << '/' << 
                    this->d << '\t' << this->h << ':' << this->mi << "\r\n";

            logger << "driver type:\t\t" << this->dtype << "\r\n";
            logger << "driver branch:\t\t" << this->dbranch << "\r\n";
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

        void construct(){

        }
    };

    struct Archive{

        union{
            struct{
                struct alignas(128){
                    uint32_t hashcode;
                    BoardInfo board_info;
                    Switches switches;
                
                };

                struct alignas(128){
                    // int16_t cali_map[50];
                    std::array<int16_t, 50> cali_map;
                };
            };

            uint8_t data[256];
        };


        uint32_t hash() const {
            return hash_djb(cali_map);
        }

        auto & map() {return cali_map;}
        const auto & map() const {return cali_map;}
        
        void clear(){
            memset(this, 0, sizeof(*this));
        }
    };
}


#endif