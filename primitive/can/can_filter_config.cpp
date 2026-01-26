#include "can_filter_config.hpp"    

#include "core/stream/ostream.hpp"

namespace ymd{

OutputStream & operator << (OutputStream & os, const hal::CanFilterConfig & self){
    if(self.is_list_mode_){
        if(self.is_32bit_){
            return os << os.field("ext_list")(std::span(&self.id32, 2));
        }else{
            return os << os.field("std_list")(std::span(self.id16.data(), 4));
        }
    }else{
        if(self.is_32bit_){
            return os << os.field("ext_gated")(std::array{self.id32, self.mask32});
        }else{
            return os << os.field("std_gated")(
                std::array{self.id16[0], self.mask16[0]},
                os.splitter(),
                std::array{self.id16[1], self.mask16[2]}
            );
        }
    }
}

}