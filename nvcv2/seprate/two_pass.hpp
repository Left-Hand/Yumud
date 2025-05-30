
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

	int findMaxLabel(const Image<Grayscale> & out);

	void twoPassConnectComponent(Image<Grayscale> & out,const Image<Binary> &src);
public:
	// void run(Image<Grayscale> & out, const Image<Binary> & src, int &max);
	Image<Grayscale> run(const Image<Binary> & src);
};
}