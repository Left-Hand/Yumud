#pragma once

#include <cstdint>

namespace ymd::dsp{
template<typename T>
class BurrFilter{
	BurrFilter(const T _certainty_tolerance = T(0.8),
		const T _error_tolerance = T(0.3),
		const uint8_t _max_misjudge_cnt = 2):
			certainty_tolerance(_certainty_tolerance), error_tolerance(_error_tolerance), max_misjudge_cnt(_max_misjudge_cnt){;}

	T update(const T x){
		if(certainty(x) < certainty_tolerance && abs(error(x)) > error_tolerance){
			misjudge_cnt++;
			if(misjudge_cnt > max_misjudge_cnt){
				misjudge_cnt = 0;
				goto follow_output;
			}else{
				T ret = predict(current);
				last = current;

				return ret;
			}
		}else{
			misjudge_cnt = 0;
			goto follow_output;
		}

		follow_output:
		last = x;
		current = x;
		return x;
	}

	T predict(const T x)const{
		return x * 2 - last;
	}

private:
	T last = T(0);
	T current = T(0);

	const T certainty_tolerance;
	const T error_tolerance;

	const uint8_t max_misjudge_cnt;
	uint8_t misjudge_cnt = 0;
	bool inited = false;

	T certainty(const T x)const{
		T ma = abs(x - current);
		T mi = abs(current - last);
		if(ma > mi){
			if(ma) return mi / ma;
			else return (mi ? T(0) : T(1));
		}
		else{
			if(mi) return ma / mi;
			else return (ma ? T(0) : T(1));
		}
	}
	T error(const T x)const{
		return x - predict(x);
	}
};

}