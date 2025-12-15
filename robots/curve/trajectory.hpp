#pragma once

#include "core/platform.hpp"
#include "core/clock/clock.hpp"
#include "core/container/heapless_vector.hpp"


namespace ymd{
struct PackedTrajectoryPoint{
    uint32_t x:10;
    uint32_t y:10;
    uint32_t z:8;
    uint32_t nz:1;
};


class Trajectory{
public:
    static constexpr uint RECORD_FPS = 50;
    static constexpr Milliseconds RECORD_DURATION = 1000ms / RECORD_FPS;
    static constexpr uint32_t RECORD_SECONDS = 60;
    static constexpr uint RECORD_LENGTH = (RECORD_FPS * RECORD_SECONDS);

    using E = PackedTrajectoryPoint;
    class TrajectoryIterator{
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
    private:

        const Trajectory& trajectory;
        uint32_t index;
    };

    // TrajectoryIterator begin() const{
    //     return TrajectoryIterator(*this, 0);
    // }

    // TrajectoryIterator end() const{
    //     return TrajectoryIterator(*this, buf.size());
    // }
    E * begin(){
        return &buf[0];
    }

    E * end(){
        return &buf[buf.size()];
    }

    auto size() const {
        return buf.size();
    }

    bool is_full() const {
        return buf.size() == buf.capacity();
    }

    void clear(){
        buf.clear();
    }

    auto capacity() const {
        return buf.capacity();
    }
    void push(const uint x, const uint y, const uint z, const bool nz){
        PackedTrajectoryPoint item;
        item.x = x;
        item.y = y;
        item.z = z;
        item.nz = nz;
        if(buf.size() < RECORD_LENGTH) buf.push_back(item);
    }

    const E & operator[](uint index) const {
        return buf[MIN(index, RECORD_LENGTH - 1)];
    }
private:

    using Container = HeaplessVector<E, RECORD_LENGTH>;
    
    Container buf;

};

struct OutputStream;

OutputStream & operator<<(OutputStream & os, const PackedTrajectoryPoint item);

}