#pragma once

#include "core/platform.hpp"
#include <initializer_list>


namespace ymd::hal{
class Crc{
protected:
    Crc(){;}
public:
    static Crc & getInstance(){
        static auto crc_instance = Crc();
        return crc_instance;
    }
    void init();

    void clear();

    uint32_t update(const uint32_t * begin, const uint32_t * end);
    uint32_t update(const std::initializer_list<uint32_t> & list){return update(list.begin(), list.end());}
};

extern Crc & crc;

}