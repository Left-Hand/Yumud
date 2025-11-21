#pragma once

#include "core/utils/Option.hpp"
#include "core/utils/handle_status.hpp"
#include "primitive/can/bxcan_frame.hpp"

namespace ymd::robots{


class CanFrameHandlerIntf{ 
public:
    virtual HandleStatus handle(const hal::CanClassicFrame & frame) = 0;
};


class CanFrameHandlerChainlink final: public CanFrameHandlerIntf{ 
public:
    CanFrameHandlerChainlink(
        Some<CanFrameHandlerIntf *> curr, 
        Option<CanFrameHandlerIntf &> next
    ):
        curr_handler_(*curr.get()),
        next_handler_(next){;}

    HandleStatus handle(const hal::CanClassicFrame & frame){ 
        HandleStatus res = curr_handler_.handle(msg);
        if(next_handler_.is_none()) return res;
        return next_handler_.unwrap().handle(msg);
    }
private:
    CanFrameHandlerIntf & curr_handler_;
    Option<CanFrameHandlerIntf &> next_handler_ = None;
};



class CanHandlerTerminator final: 
public CanFrameHandlerIntf{ 
    HandleStatus handle(const hal::CanClassicFrame & frame){ 
        return HandleStatus::from_handled();
    }
};

}