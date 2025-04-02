// https://blog.csdn.net/fengbingchun/article/details/72874623
template<typename _Tp>
int pinv(const std::vector<std::vector<_Tp>>& src, std::vector<std::vector<_Tp>>& dst, _Tp tolerance)
{
	std::vector<std::vector<_Tp>> D, U, Vt;
	if (svd(src, D, U, Vt) != 0) {
		fprintf(stderr, "singular value decomposition fail\n");
		return -1;
	}
 
	int m = src.size();
	int n = src[0].size();
 
	std::vector<std::vector<_Tp>> Drecip, DrecipT, Ut, V;
 
	transpose(Vt, V);
	transpose(U, Ut);
 
	if (m < n)
		std::swap(m, n);
 
	Drecip.resize(n);
	for (int i = 0; i < n; ++i) {
		Drecip[i].resize(m, (_Tp)0);
 
		if (D[i][0] > tolerance)
			Drecip[i][i] = 1.0f / D[i][0];
	}
 
	if (src.size() < src[0].size())
		transpose(Drecip, DrecipT);
	else
		DrecipT = Drecip;
 
	std::vector<std::vector<_Tp>> tmp = matrix_mul(V, DrecipT);
	dst = matrix_mul(tmp, Ut);
 
	return 0;
}