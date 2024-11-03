#include "two_pass.hpp"

using std::min;
using std::max;

namespace yumud::nvcv2::Shape{

TwoPass::TwoPass(int size)
{
	M_maxsize = size;
	parent = (uint16_t*)malloc(M_maxsize * sizeof(uint16_t));
	memset(parent, 0, M_maxsize * sizeof(uint16_t));
}

TwoPass::~TwoPass()
{
	free(parent);
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
	auto [w,h] = src.get_size();

	int label = 1;

	//init lable image
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
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
						out.at(y, x) = min(left, up);
						if (left <= up)
							Union(up, left);
						else if (up<left)
							Union(left, up);
					}
					else{
						//adopt larger label
						out.at(y, x) = max(left, up);
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
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			if (src.at(y, x) != 0)
				out.at(y, x) = Find(out.at<int>(y, x));
		}
	}
}

//find the max label value
void TwoPass::findMaxLabel(const ImageWithData<Grayscale, Grayscale> & out, int &max)
{
	auto [imgH, imgW] = out.get_size();

	for (int i = 0; i < imgH; i++)
	{
		for (int j = 0; j < imgW; j++)
		{
			int idx = i*imgW + j;
			if (out[idx] > max) max = out[idx];
		}
	}
}

ImageWithData<Grayscale, Grayscale> TwoPass::run(const ImageReadable<Binary> & src)
{
	//detect connected-region using two Pass method
	auto ret = ImageWithData<Grayscale, Grayscale>(src.get_size());
	twoPassConnectComponent(ret, src);
	return ret;
	//find the largest label
	// findMaxLabel(out, max);
}

}