#ifndef __BURR_FILTER_HPP__

#define __BURR_FILTER_HPP__

template<typename real>
class BurrFilter_t{
	real last = real(0);
	real current = real(0);

	const real certainty_tolerance;
	const real error_tolerance;

	const uint8_t max_misjudge_cnt;
	uint8_t misjudge_cnt = 0;
	bool inited = false;

	real certainty(const real x)const{
		real ma = abs(x - current);
		real mi = abs(current - last);
		if(ma > mi){
			if(ma) return mi / ma;
			else return (mi ? real(0) : real(1));
		}
		else{
			if(mi) return ma / mi;
			else return (ma ? real(0) : real(1));
		}
	}
	real error(const real x)const{
		return x - predict(x);
	}
public:

	BurrFilter_t(const real _certainty_tolerance = real(0.8),
		const real _error_tolerance = real(0.3),
		const uint8_t _max_misjudge_cnt = 2):
			certainty_tolerance(_certainty_tolerance), error_tolerance(_error_tolerance), max_misjudge_cnt(_max_misjudge_cnt){;}

	real update(const real x){
		if(certainty(x) < certainty_tolerance && abs(error(x)) > error_tolerance){
			misjudge_cnt++;
			if(misjudge_cnt > max_misjudge_cnt){
				misjudge_cnt = 0;
				goto follow_output;
			}else{
				real ret = predict(current);
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

	real predict(const real & x)const{
		return x * 2 - last;
	}
};

#endif