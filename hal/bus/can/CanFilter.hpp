#pragma once

#include <cstdint>
#include <initializer_list>

namespace yumud{
class CanFilter{
private:
    struct ID16{
        union{
            uint16_t raw;
            struct{
                uint16_t:3;
                const bool ide:1 = 0;
                bool rtr:1;
                uint16_t id:11;
            };
        };
    };

    struct ID32{
        union{
            uint32_t raw;
            struct{
                uint32_t:1;
                const bool ide:1 = 1;
                bool rtr:1;
                uint32_t id:29;
            };
        };
    };


    void list16(const uint16_t id0, const uint16_t id1, const uint16_t id2, const uint16_t id3);
    void list32(const uint32_t id0, const uint32_t id1);
    void gate16();
    void gate32();
protected:
    union{
        uint16_t id16[2] = {0};
        uint32_t id32;
    };

    union{
        uint16_t mask16[2] = {0};
        uint32_t mask32;
    };

public:
    CanFilter() = default;
    CanFilter(const uint16_t _id): id16{_id, 0}, mask16{_id, 0}{;}
    CanFilter(const uint16_t _id, const uint16_t _mask) : id16{_id, 0}, mask16{_mask, 0}{;}
    CanFilter(const std::initializer_list<uint16_t> & list);

    void init();
    static void init(const CanFilter & filter);
};

}