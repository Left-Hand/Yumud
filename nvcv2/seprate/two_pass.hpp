
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

	int findMaxLabel(const ImageWithData<Grayscale, Grayscale> & out);

	void twoPassConnectComponent(ImageWithData<Grayscale, Grayscale> & out,const ImageReadable<Binary> &src);
public:
	// void run(ImageWithData<Grayscale, Grayscale> & out, const ImageReadable<Binary> & src, int &max);
	ImageWithData<Grayscale, Grayscale> run(const ImageReadable<Binary> & src);
};
}