#include "two_pass.hpp"

namespace ymd::nvcv2::shape{

TwoPass::TwoPass(int size)
{
	M_maxsize = size;
	parent = std::make_unique<uint16_t[]>(size);
}


// find the root of label x
int TwoPass::Find(int x)
{
	//assert(x < maxSize);
	int i = x;
	while (0 != parent[i])
		i = parent[i];
	return i;
}

// set label x and label y to the same connected-region
void TwoPass::Union(int x, int y)
{
	//assert(x < maxSize && y < maxSize);
	int i = x;
	int j = y;
	while (0 != parent[i])
		i = parent[i];
	while (0 != parent[j])
		j = parent[j];
	if (i != j)
		parent[i] = j;
}

void TwoPass::twoPassConnectComponent(Image<Gray> & out,const Image<Binary> &src){
	auto [w,h] = src.size();

	int label = 1;

	//init lable image
	for (uint16_t y = 0u; y < h; y++){
		for (uint16_t x = 0u; x < w; x++){
			if (src[{x,y}].is_black()) continue;
			//left and up labels
			Gray left = (x - 1 < 0) ? Gray::black() : out(static_cast<uint16_t>(x - 1), y);
			Gray up = (y - 1 < 0) ? Gray::black() : out(x, static_cast<uint16_t>(y-1));

			if (left.is_black() and up.is_black()){
				//non of two labels is markded, so add a new label
				out[{x,y}] = Gray::from_u8(++label);
				continue;
			}
			
			// least one label is marked
			//two labels are marked
			if (left.is_black() or up.is_black()){
				//adopt larger label
				out[{x,y}] = std::max(left, up);
				continue;
			}

			//adopt smaller label
			out[{x,y}] = std::min(left, up);
			if (left <= up)
				Union(up.to_u8(), left.to_u8());
			else if (up<left)
				Union(left.to_u8(), up.to_u8());
		}

	}


	//second pass 
	for (uint16_t y = 0u; y < h; y++){
		for (uint16_t x = 0u; x < w; x++){
			if (src[{x,y}] != Binary::black())
				out[{x,y}] = Gray::from_u8(Find(out[{x,y}].to_u8()));
		}
	}
}

//find the max label value
int TwoPass::findMaxLabel(const Image<Gray> & out){
	const auto [imgH, imgW] = out.size();
	int32_t max = INT_FAST32_MIN;
	for (uint16_t i = 0u; i < imgH; i++)
	{
		for (uint16_t j = 0u; j < imgW; j++)
		{
			auto ele = out(i, j).to_u8();
			if (ele > max) {
				max = ele;
			}
		}
	}
	return max;
}

Image<Gray> TwoPass::run(const Image<Binary> & src)
{
	//detect connected-region using two Pass method
	auto ret = Image<Gray>(src.size());
	twoPassConnectComponent(ret, src);
	return ret;
	//find the largest label
	// findMaxLabel(out, max);
}

}