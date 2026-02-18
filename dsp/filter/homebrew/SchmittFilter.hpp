#pragma once

#include "core/math/iq16.hpp"

template<arithmetic T>
class SchmittFilter{
	SchmittFilter(const T _lowThr,const T _highThr):lowThr(_lowThr),highThr(_highThr){;}

	bool update(const T & input){
        if(input < lowThr and 
				output == true){
            output = false;
        }else if(input > highThr and 
				output == false){
            output = true;
        return output;
	}
private:
	T lowThr = 0;
	T highThr = 0;
	bool output = false;

};
}

