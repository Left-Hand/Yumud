#include "mp6540.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;

#ifdef MP6540_DEBUG
#undef MP6540_DEBUG
#define MP6540_DEBUG(...) DEBUG_LOG(...)
#else 
#define MP6540_DEBUG(...)
#endif

static void on_error(){
    PANIC("terminated!!!");
}


MP6540::MP6540(PWM3_WP && pwms, AIN3_WP && ains)
: pwms_{
    &pwms[0].get(),
    &pwms[1].get(),
    &pwms[2].get()}
,ains_{
    &ains[0].get(),
    &ains[1].get(),
    &ains[2].get()}
    
{

}



void MP6540::init(){
    for(size_t i = 0; i < 3; i++){
        const auto & item = pwms_[i];
        if(item == nullptr){
            MP6540_DEBUG("pwm[", i, "]: is null");
            on_error();
        }else{
            // item->init();
        } 
    }

    for(size_t i = 0; i < 3; i++){
        const auto & item = ains_[i]; 
        if(item == nullptr){
            MP6540_DEBUG("ain[", i, "]: is null");
            on_error();
        }else{

        }
    }
    
    return;
}

void MP6540::enable(const Enable en){
    bool has_en = false;
    for(size_t i = 0; i < 3; i++){
        const auto & item = ens_[i]; 
        if(item == nullptr){
        }else{
            item->write(en == EN ? HIGH : LOW);
        }
    }

    if(has_en == false){
        MP6540_DEBUG("No enable channel found!");
        on_error();
    }
}

void MP6540::set_so_res(const uint so_res_ohms){
    static constexpr real_t curr_mirror_ratio = real_t(9200.0);
    const auto volt_to_curr_ratio = curr_mirror_ratio / so_res_ohms;

    for(auto & ch : chs){
        ch.set_ratio(volt_to_curr_ratio);
    }
}

// void MP6540::setBias(const real_t b0, const real_t b1, const real_t b2){
//     chs[0].bias_ = b0;
//     chs[1].bias_ = b1;
//     chs[2].bias_ = b2;
// }

MP6540::MP6540CurrentChannel & MP6540::ch(const size_t index){
    if(index == 0 or index > 3){
        MP6540_DEBUG("Channel index out of range:", index);
        PANIC();
    }
    return chs[index - 1];
}

MP6540 & MP6540::operator= (const UVW_Duty & duty){
    std::tie(*pwms_[0], *pwms_[1], *pwms_[2]) = duty; 
    return *this;
}
