
#pragma once
#include "nvcv2/nvcv2.hpp"


namespace ymd::nvcv2::shape{
class TwoPass{
public:
	TwoPass(int size);

private:
	int M_maxsize;
	std::unique_ptr<uint16_t[]> parent;

private:
	int Find(int x);
	void Union(int x, int y);

	int findMaxLabel(const Image<Gray> & out);

	void twoPassConnectComponent(Image<Gray> & out,const Image<Binary> &src);
public:
	// void run(Image<Gray> & out, const Image<Binary> & src, int &max);
	Image<Gray> run(const Image<Binary> & src);
};
}