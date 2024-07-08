#include "mnist.hpp"

static uint8_t mdl_buf[MDL_BUF_LEN];

void Mnist::parse_output(tm_mat_t* outs){
    tm_mat_t out = outs[0];
    const float* data  = out.dataf;
    float maxp = 0;
    int maxi = -1;
    for(int i=0; i<N; i++){
        outputs[i] = data[i];
        if(data[i] > maxp) {
            maxi = i;
            maxp = data[i];
        }
    }

    output = {maxi, maxp};
    return;
}

void Mnist::load(){
    auto res = tm_load(&mdl, mdl_data, mdl_buf, layer_cb, &in);
    loaded = bool(res == TM_OK);

    if(res != TM_OK) {
        TM_PRINTF("tm model load err", int(res));
        return ;
    }
}

void Mnist::unload(){
    if(loaded) tm_unload(&mdl);   
}

Mnist::Result Mnist::update(const Image<Grayscale, Grayscale> & img, const Vector2i & pos){

    Image<Grayscale, Grayscale> img_view = img.clone({pos.x, pos.y, 28, 28});
    tm_mat_t in_uint8 = {3,IMG_L,IMG_L,IMG_CH, (mtype_t*)img_view.data.get()};
    auto err = tm_preprocess(&mdl, TMPP_UINT2INT, &in_uint8, &in); 

    err = tm_run(&mdl, &in, outs);

    if(err==TM_OK){
        parse_output(outs);        
    }

    return output;
}
