#include "can_filter_config.hpp"    

#include "core/stream/ostream.hpp"

namespace ymd{

OutputStream & operator << (OutputStream & os, const hal::CanFilterConfig & self){
    if(self.is_list_mode()){
        if(self.is_32bit()){
            return os << os.field("ext_list")(std::span(&self.id32, 2));
        }else{
            return os << os.field("std_list")(std::span(self.id16x2.data(), 4));
        }
    }else{
        if(self.is_32bit()){
            return os << os.field("ext_gated")(std::array{self.id32, self.mask32});
        }else{
            return os << os.field("std_gated")(
                std::array{self.id16x2[0], self.mask16x2[0]},
                os.splitter(),
                std::array{self.id16x2[1], self.mask16x2[2]}
            );
        }
    }
}

}