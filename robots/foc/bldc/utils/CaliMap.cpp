#include "CaliMap.hpp"

using namespace ymd::foc;

CaliMap::CaliMap(){
    samples_.push_back({0, 0});
    samples_.push_back({1, 1});
}

void CaliMap::resize(const size_t size){
    samples_.resize(size);
}

void CaliMap::reserve(const size_t size){
    samples_.reserve(size);
}

void CaliMap::addSample(const Vector2<real_t> & vec){
    samples_.push_back(vec);
    std::sort(samples_.begin(), samples_.end(), Vector2<real_t>::sort_by_x);
}