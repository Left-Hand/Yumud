#include "trajectory.hpp"
#include "core/stream/ostream.hpp"


namespace ymd{
OutputStream & operator<<(OutputStream & os, const PackedTrajectoryPoint point){
    return os << os.brackets<'['>() 
            << point.x << os.splitter() 
            << point.y << os.splitter() 
            << point.z << os.splitter() 
            << point.nz << os.splitter() 
        << os.brackets<']'>();

}


}