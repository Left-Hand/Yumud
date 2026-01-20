#pragma once

#include "core/utils/Option.hpp"
#include "core/utils/handle_status.hpp"
#include "primitive/can/bxcan_frame.hpp"

namespace ymd::robots{


class CanFrameHandlerIntf{ 
public:
    virtual HandleStatus handle(const hal::BxCanFrame & frame) = 0;
};


class CanFrameHandlerChainlink final: public CanFrameHandlerIntf{ 
public:
    CanFrameHandlerChainlink(
        Some<CanFrameHandlerIntf *> self, 
        Option<CanFrameHandlerIntf &> next
    ):
        self_handler_(*self.get()),
        next_handler_(next){;}

    HandleStatus handle(const hal::BxCanFrame & frame){ 
        HandleStatus res = self_handler_.handle(frame);
        if(next_handler_.is_none()) return res;
        return next_handler_.unwrap().handle(frame);
    }
private:
    CanFrameHandlerIntf & self_handler_;
    Option<CanFrameHandlerIntf &> next_handler_ = None;
};



class CanHandlerTerminator final: 
public CanFrameHandlerIntf{ 
    HandleStatus handle(const hal::BxCanFrame & frame){ 
        return HandleStatus::from_handled();
    }
};

}