#pragma once

namespace ymd::dsp{
class EdgeCounter{
public:
    void update(bool state){
        if(state != state_){
            count_ ++;
        }
        state_ = state;
    }

    auto count() const{
        return count_;
    }
private:
    unsigned int count_ = 0;
    bool state_ = false;
};

}