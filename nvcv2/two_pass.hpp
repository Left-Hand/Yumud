
#pragma once
#include "nvcv2.hpp"


namespace NVCV2::Shape{
class TwoPass{
public:
	TwoPass(int size);
	~TwoPass();

private:
	int maxSize;
	uint16_t *parent;

private:
	int Find(int x);
	void Union(int x, int y);

	void findMaxLabel(const Image<Grayscale, Grayscale> & out, int &max);

	void twoPassConnectComponent(Image<Grayscale, Grayscale> & out,const ImageReadable<Binary> &src);
public:
	// void run(Image<Grayscale, Grayscale> & out, const ImageReadable<Binary> & src, int &max);
	Image<Grayscale, Grayscale> run(const ImageReadable<Binary> & src);
};
}