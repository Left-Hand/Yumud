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

void TwoPass::twoPassConnectComponent(ImageWithData<Grayscale, Grayscale> & out,const ImageReadable<Binary> &src){
	auto [w,h] = src.size();

	int label = 1;

	//init lable image
	for (auto y = 0u; y < h; y++)
	{
		for (auto x = 0u; x < w; x++)
		{
			if (src[{x,y}] != 0)
			{
				//left and up labels
				Grayscale left = (x - 1 < 0) ? Grayscale(0) : out.at(y, x - 1);
				Grayscale up = (y - 1 < 0) ? Grayscale(0) : out.at(y - 1, x);

				//at least one label is marked
				if (left != 0 || up != 0){
					//two labels are marked
					if (left != 0 && up != 0){
						//adopt smaller label
						out.at(y, x) = std::min(left, up);
						if (left <= up)
							Union(up, left);
						else if (up<left)
							Union(left, up);
					}
					else{
						//adopt larger label
						out.at(y, x) = std::max(left, up);
					}
				}
				else
				{
					//non of two labels is markded, so add a new label
					out.at(y, x) = ++label;
				}
			}
		}
	}

	//second pass 
	for (auto y = 0u; y < h; y++)
	{
		for (auto x = 0u; x < w; x++)
		{
			if (src.at(y, x) != 0)
				out.at(y, x) = Find(out.at<int>(y, x));
		}
	}
}

//find the max label value
int TwoPass::findMaxLabel(const ImageWithData<Grayscale, Grayscale> & out){
	const auto [imgH, imgW] = out.size();
	auto max = INT_FAST32_MIN;
	for (auto i = 0u; i < imgH; i++)
	{
		for (auto j = 0u; j < imgW; j++)
		{
			auto ele = uint8_t(out.at(i, j));
			if (ele > max) {
				max = ele;
			}
		}
	}
	return max;
}

ImageWithData<Grayscale, Grayscale> TwoPass::run(const ImageReadable<Binary> & src)
{
	//detect connected-region using two Pass method
	auto ret = ImageWithData<Grayscale, Grayscale>(src.size());
	twoPassConnectComponent(ret, src);
	return ret;
	//find the largest label
	// findMaxLabel(out, max);
}

}