#pragma once

#include "sys/core/platform.h"
#include "thirdparty/sstl/include/sstl/vector.h"


namespace yumud{
struct TrajectoryItem{
    uint32_t x:10;
    uint32_t y:10;
    uint32_t z:8;
    uint32_t nz:1;
};


class Trajectory{
public:
    scexpr uint record_fps = 50;
    scexpr uint record_dur = 1000 / record_fps;
    scexpr uint record_seconds = 60;
    scexpr uint record_len = (record_fps * record_seconds);

    using E = TrajectoryItem;
    using Container = sstl::vector<E, record_len>;
    
    Container data;

    class TrajectoryIterator{
        protected:

            const Trajectory& trajectory;
            uint32_t index;

        public:
            TrajectoryIterator(const Trajectory& _trajectory, uint32_t _index):trajectory(_trajectory), index(_index){}
            TrajectoryIterator& operator++(){
                index++;
                return *this;
            }

            TrajectoryIterator& operator--(){
                index--;
                return *this;
            }
    };

    // TrajectoryIterator begin() const{
    //     return TrajectoryIterator(*this, 0);
    // }

    // TrajectoryIterator end() const{
    //     return TrajectoryIterator(*this, data.size());
    // }
    E * begin(){
        return &data[0];
    }

    E * end(){
        return &data[data.size()];
    }

    auto size() const {
        return data.size();
    }

    bool is_full() const {
        return data.size() == data.capacity();
    }

    void clear(){
        data.clear();
    }

    auto capacity() const {
        return data.capacity();
    }
    void push(const uint x, const uint y, const uint z, const bool nz){
        TrajectoryItem item;
        item.x = x;
        item.y = y;
        item.z = z;
        item.nz = nz;
        if(data.size() < record_len) data.push_back(item);
    }

    const E & operator[](uint index) const {
        return data[MIN(index, record_len - 1)];
    }
};

struct OutputStream;

OutputStream & operator<<(OutputStream & os, const TrajectoryItem item);

}