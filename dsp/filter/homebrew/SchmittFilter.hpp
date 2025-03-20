#pragma once

#include "core/math/real_t.hpp"

template<arithmetic T>
class SchmittFilter_t{
protected:
	T lowThr = 0;
	T highThr = 0;
	bool output = false;

public:

	SchmittFilter_t(const T _lowThr,const T _highThr):lowThr(_lowThr),highThr(_highThr){;}

	bool update(const T & input){
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

