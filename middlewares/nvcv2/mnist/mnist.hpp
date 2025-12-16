#pragma once


#include "middlewares/nvcv2/mnist/tinymaix.h"
#include "middlewares/nvcv2/nvcv2.hpp"

class Classify{

};


class Mnist{
public:
    
    static constexpr size_t N = 10; 
    static constexpr Vec2u IMAGE_SIZE = {28, 28};
    static constexpr size_t IMAGE_CHANNELS = 1;

    using Choices = std::array<real_t, N>;
    
    struct Result{
        int token;
        real_t confidence;
    };

    Choices outputs = {0};
    Result output = {0, 0};
protected:
    tm_mdl_t mdl;
    bool loaded = false;
    tm_mat_t in = {3,IMAGE_SIZE.x,IMAGE_SIZE.y,IMAGE_CHANNELS, NULL};
    tm_mat_t outs[1];

    static tm_err_t layer_cb(tm_mdl_t* mdl, tml_head_t* lh){
        return TM_OK;
    }

    void parse_output();

    void load();
    void unload();
public:
    Mnist(){load();}
    ~Mnist(){unload();}
    Result update(const Image<Gray> & img, const Vec2u & pos);
    Result update(const Image<Gray> & img);
};