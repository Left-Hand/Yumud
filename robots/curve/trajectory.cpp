#include "trajectory.hpp"
#include "core/stream/ostream.hpp"


namespace ymd{
OutputStream & operator<<(OutputStream & os, const TrajectoryItem item){
    return os << '[' << item.x << ',' << item.y << ',' << item.z << ',' << item.nz << ']'; 
}


}