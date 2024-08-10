#include "teach.hpp"
#include "sys/kernel/stream.hpp"

OutputStream & operator<<(OutputStream & os, const TrajectoryItem item){
    return os << '[' << item.x << ',' << item.y << ',' << item.z << ',' << item.nz << ']'; 
}