#pragma once

#include "sr2631z3_prelude.hpp"

namespace ymd::drivers::sr2631z3{


namespace msgs{


struct [[nodiscard]] GGA final{
    using Self = GGA;
    using Error = DeMsgErrorKind;


    Option<UtcTime> utc_time;
    Option<Lat> lat;
    Option<uLat> ulat;
    Option<Lon> lon;
    Option<uLon> ulon;


    static constexpr Self from_uninitialized(){
        return Self{
            .utc_time = None,
            .lat = None,
            .ulat = None,
            .lon = None,
            .ulon = None
        };
    }

};


}


}