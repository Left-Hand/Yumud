#pragma once


namespace ymd::dsp{

#if 0
template<typename T>
class ComplementaryFilter_t{
public:
    struct Config{
        T kq;
        T ko;
        uint fs;
    };
    
protected:
    T kq_;
    T ko_;
    T dt_;
    T rot_;
    T rot_unfiltered;
    T last_rot;
    T last_gyr;
    // T last_time;

    uint delta_t;
    
    bool inited;
public:
    ComplementaryFilter_t(const Config & config){
        reconf(config);
        reset();
    }


    void reconf(const Config & cfg){
        kq_ = cfg.kq;
        kq_ = cfg.kq;
        dt_ = T(1) / cfg.fs;
    }

    void update(const T rot, const T gyr){

        if(!inited){
            rot_ = rot;
            rot_unfiltered = rot;
            inited = true;
        }else{
            rot_unfiltered += gyr * delta_t;
            rot_unfiltered = kq_ * rot_ + (1-kq_) * rot;
            rot_ = ko_ * rot_ + (1-ko_) * rot_unfiltered;
        }
    
        last_rot = rot;
        last_gyr = gyr;
    }

    void reset(){
        rot_ = 0;
        rot_unfiltered = 0;
        last_rot = 0;
        last_gyr = 0;
        inited = false;
    }

    T result() const {
        return rot_;
    }
};

#endif
template<typename T>
struct ComplementaryFilter{
    struct Config{
        T kq;
        T ko;
        uint fs;
    };
    

    constexpr ComplementaryFilter(const Config & config){
        reconf(config);
        reset();
    }


    constexpr void reconf(const Config & cfg){
        kq_ = cfg.kq;
        kq_ = cfg.kq;
        dt_ = T(1) / cfg.fs;
    }

    constexpr T operator ()(const T rot, const T gyr){

        if(!is_inited_){
            rot_ = rot;
            rot_unfiltered_ = rot;
            is_inited_ = true;
        }else{
            rot_unfiltered_ += gyr * delta_t_;
            rot_unfiltered_ = kq_ * rot_ + (1-kq_) * rot;
            rot_ = ko_ * rot_ + (1-ko_) * rot_unfiltered_;
        }
    
        last_rot_ = rot;
        last_gyr_ = gyr;

        return rot_;
    }

    constexpr void reset(){
        rot_ = 0;
        rot_unfiltered_ = 0;
        last_rot_ = 0;
        last_gyr_ = 0;
        is_inited_ = false;
    }

    constexpr T get() const {
        return rot_;
    }

private:
    T kq_;
    T ko_;
    T dt_;
    T rot_;
    T rot_unfiltered_;
    T last_rot_;
    T last_gyr_;
    // T last_time;

    uint delta_t_;
    
    bool is_inited_;
};

}