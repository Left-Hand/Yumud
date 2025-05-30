#include "GridCaliMap.hpp"

#include "types/regions/range2/range2.hpp"
#include "types/Line2/Line2.hpp"


using namespace ymd::foc;


void GridCaliMap::resize(const size_t size){
    samples_.resize(size);
}

void GridCaliMap::reserve(const size_t size){
    samples_.reserve(size);
}


GridCaliMap::ErrorCode GridCaliMap::loadSamples(const Samples & samples){
    auto calculate_y_average = [](const Sample * begin, const size_t len){
        real_t sum = 0;
        for(size_t i = 0; i < len; i++){
            sum += begin[i].y;
        }
        return sum / len;
    };

    if(samples.size() == 0) return ErrorCode::NO_SAMPLES;
    
    ok_ = false;
    samples_.clear();
    auto temp = samples;
    
    real_t x = 0;
    real_t x_delta = real_t(1) / samples_.size();
    
    size_t temp_index = 0;
    real_t last_y_out = 0;
    real_t y_out;
    for(size_t i = 0; i < samples_.size(); i++){
        x += x_delta;  

        // |  .|
        // | . |
        // |.  |
        // |   |
        //找出满足到下一次y的区间
        
        Range2i range = Range2i(temp_index, temp_index);

        for(; temp_index < temp.size(); temp_index++){
            range.to ++;
            if(temp[range.to].x > x) break;
        }

        //如果这个区间有满足条件的采样 则更新y平均值
        if(range.length() > 0){
            y_out = calculate_y_average(temp.data() + range.from, range.length());
        }else{
            return ErrorCode::NO_SAMPLE_FOUND_IN_RANGE;
        }

        if((y_out - last_y_out) * samples_.size() // 直线的斜率
             > real_t(2)){
            
        }

        
        //将这个区间计算出的平均点加入采样
        samples_.push_back({x, y_out});
        last_y_out = y_out;
    }

    ok_ = true;
    return ErrorCode::OK;
}