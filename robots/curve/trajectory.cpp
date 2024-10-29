#include "trajectory.hpp"
#include "sys/stream/ostream.hpp"

OutputStream & operator<<(OutputStream & os, const TrajectoryItem item){
    return os << '[' << item.x << ',' << item.y << ',' << item.z << ',' << item.nz << ']'; 
}