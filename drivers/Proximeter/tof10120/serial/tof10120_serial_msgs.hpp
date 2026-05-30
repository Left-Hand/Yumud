#include "../tof10120_prelude.hpp"
#include "core/container/heapless_vector.hpp"
namespace ymd::drivers::tof10120{

struct req_msgs{


};

static constexpr size_t MAX_REQ_CHARS = 8;

struct alignas(4) [[nodiscard]] FlatRequest final{
    using Self = FlatRequest;


    HeaplessVector<uint8_t, MAX_REQ_CHARS> bytes;

    std::span<const uint8_t> transmitable_bytes() const {
        return std::span(bytes.begin(), bytes.end());
    } 

    static constexpr Self from_bare_id(const uint8_t id){
        Self self;
        self.bytes.push_back('r');
        self.bytes.push_back(id + '0');
        self.bytes.push_back('#');
        return self;
    }

    static constexpr Self from_id_and_num(
        const uint8_t id, 
        const uint32_t num, 
        const size_t num_len
    ){
        Self self;
        self.bytes.push_back('r');
        self.bytes.push_back(id + '0');
        self.bytes.push_back('#');
        return self;
    }
};

struct Factory{

};


}