#pragma once

// #include "drivers/"

namespace gxm{
void initDisplayer(){
    {
        tftDisplayer.init();

        tftDisplayer.setFlipX(false);
        tftDisplayer.setFlipY(true);
        if(true){
            tftDisplayer.setSwapXY(true);
            tftDisplayer.setDisplayOffset({40, 52}); 
        }else{
            tftDisplayer.setSwapXY(false);
            tftDisplayer.setDisplayOffset({52, 40}); 
        }
        tftDisplayer.setFormatRGB(true);
        tftDisplayer.setFlushDirH(false);
        tftDisplayer.setFlushDirV(false);
        tftDisplayer.setInversion(true);

        tftDisplayer.fill(ColorEnum::BLACK);
    }
}

}