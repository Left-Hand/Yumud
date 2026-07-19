#pragma once

#include "damiao_api_facade.hpp"

namespace ymd::robots::damiao{

struct [[nodiscard]] FrameFactoryBackend final{
    struct State{
        NodeId motor_id;
    };

    static constexpr hal::ClassicCanFrame convert(const hal::ClassicCanFrame && frame){
        return frame.clone();
    }
};


using FrameFactory = ClientApiFacade<FrameFactoryBackend>;

}