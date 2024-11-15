
#pragma once
#include "nvcv2.hpp"


namespace ymd::nvcv2::Shape{
class TwoPass{
public:
	TwoPass(int size);
	~TwoPass();

private:
	int M_maxsize;
	uint16_t *parent;

private:
	int Find(int x);
	void Union(int x, int y);

	void findMaxLabel(const ImageWithData<Grayscale, Grayscale> & out, int &max);

	void twoPassConnectComponent(ImageWithData<Grayscale, Grayscale> & out,const ImageReadable<Binary> &src);
public:
	// void run(ImageWithData<Grayscale, Grayscale> & out, const ImageReadable<Binary> & src, int &max);
	ImageWithData<Grayscale, Grayscale> run(const ImageReadable<Binary> & src);
};
}