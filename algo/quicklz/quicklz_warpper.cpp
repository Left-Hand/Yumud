#include "quicklz_warpper.hpp"
#include "quicklz.h"

#include <string.h>

std::vector<uint8_t> quicklz_compress(const std::vector<uint8_t> & src)
{
	qlz_state_compress state;
	// memset(&state, 0, sizeof(qlz_state_compress));
	std::vector<uint8_t> dst;

	scexpr size_t n = 128;
	char buffer[n + n/4];
	for(size_t pos = 0;pos<src.size();pos+=n) {
		size_t len = MIN(src.size() - pos, n);
		len = qlz_compress(src.data() + pos, buffer, len, &state);
		for(size_t i = 0;i < len; i++)dst.push_back((uint8_t)buffer[i]);
	}
	return dst;
}


std::vector<uint8_t> quicklz_decompress(const std::vector<uint8_t>& qlzdata)
{
	qlz_state_decompress state;
	memset(&state, 0, sizeof(qlz_state_decompress));
	std::vector<uint8_t> dst;
	for(size_t pos = 0;pos < qlzdata.size(); ){
		// 获取压缩数据段大小
		size_t co_size = qlz_size_compressed((char *)(qlzdata.data()) + pos);
		// 获取该压缩段解压后的大小
		size_t de_size = qlz_size_decompressed((char *)qlzdata.data() + pos);
		std::vector<uint8_t> buffer(de_size,0);
		qlz_decompress((char *)qlzdata.data()+pos, (char*)buffer.data(),&state);
		pos += co_size;
//		dst.push_back(buffer);
		for(const auto & item :buffer){
			dst.push_back(item);	
		}
	}
	return dst;
}