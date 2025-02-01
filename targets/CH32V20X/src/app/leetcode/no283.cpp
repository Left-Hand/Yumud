#include "tb.h"

#include "sys/debug/debug_inc.h"

class Solution283{
public:
	void moveZeroes(std::vector<int>& nums) {
		auto idxs = std::vector<int>();
		idxs.reserve(nums.size());
		
		for(size_t i = 0;i < nums.size(); i++){
			if(nums[i] != 0) idxs.push_back(i);
		}
		
		bool scannoff = false;
		for(size_t i = 0; i < nums.size(); i++){
			
			if(i >= idxs.size()) scannoff = true;
			
			if(scannoff){
				nums[i] = 0;
			}else{
				const auto targ_idx = i;
				const auto org_idx = idxs[i];	
				nums[targ_idx] = nums[org_idx];

			}
			
		}
	}
};

class Solution27{
public:
    int removeElement(std::vector<int>& nums, int val) {
		auto idxs = std::vector<size_t>();
		idxs.reserve(nums.size());
		
		for(size_t i = 0;i < nums.size(); i++){
			if(nums[i] == val) idxs.push_back(i);
		}
		
		// bool scannoff = false;
        size_t match_idx = 0;
        size_t targ_idx = 0;
        // size_t org_idx = 0;
        
		for(size_t i = 0; i < nums.size(); i++){
            if(i != idxs[match_idx] or match_idx >= idxs.size()){//未匹配
                nums[targ_idx] = nums[i];
                targ_idx++;
            }else{//匹配
                match_idx++;
            }
			
		}

        return nums.size() - idxs.size();
    }
};

class Solution11 {
public:
    int maxArea(std::vector<int>& height) {
        auto i = height.begin();
        auto j = std::prev(height.end());

        auto max_area = 0;
        while(i != j){
            auto area = std::min(*i, *j) * (j - i);
            if(area > max_area){
                max_area = area;
            }
            if(*i < *j){
                i++;
            }else{
                j--;
            }

        }

        return max_area;
    }
};

class Solution46{
public:
    vector<vector<int>> permute(vector<int>& nums) {
        
    }
};

void leetcode_no283(){
	Solution283 solu;
	
	std::vector nums = {0,1,0,3,12};
	solu.moveZeroes(nums);
	
    DEBUG_PRINTLN(nums);
}

void leetcode_no27(){
	Solution27 solu;
	
	// std::vector nums = {0,1,2,2,3,0,4,2};
	std::vector nums = {3,2,2,3};
	auto n = solu.removeElement(nums, 2);
	
    DEBUG_PRINTLN(nums, n);
}

void leetcode_no11(){
	Solution11 solu;
	
	// std::vector nums = {0,1,2,2,3,0,4,2};
	std::vector nums = {1,8,6,2,5,4,8,3,7};
	auto n = solu.maxArea(nums);
	
    DEBUG_PRINTLN(n);
}

void leetcode_main(){
    uart2.init(576000);
    DEBUGGER.change(uart2);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");

    auto m = micros();

    // leetcode_no283();
    leetcode_no11();

    DEBUG_PRINTS("done, use", micros() - m, "us")
    while(true);
}