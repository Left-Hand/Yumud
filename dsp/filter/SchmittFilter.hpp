#ifndef __SCHMITT_FILTER_H__
#define __SCHMITT_FILTER_H__

#include "sys/math/real.hpp"

template<typename real>
class SchmittFilter{
protected:
	real lowThr = 0;
	real highThr = 0;
	bool output = false;

public:

	SchmittFilter(const real _lowThr,const real _highThr):lowThr(_lowThr),highThr(_highThr){;}

	bool update(const real & input){
        if(input < lowThr and 
				output == true){
            output = false;
        }else if(input > highThr and 
				output == false){
            output = true;
        return output;
	}
};
}

#endif
