#include "../tb.h"
#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

using namespace ymd;
using namespace ymd::hal;

struct Node{
	size_t idx;
};

struct Edge{
	size_t a;
	size_t b;
	real_t cost;
};

template<typename T, size_t X_N, size_t Y_N>
struct Matrix{
	using Row = std::array<T, X_N>;
	using Mat = std::array<Row, Y_N>;
	
	static constexpr size_t w = X_N;
	static constexpr size_t h = Y_N;
	
	Mat _mat;
	
	T & operator [](const size_t x, const size_t y){
		auto & row =  (_mat[y]);
		return row[x];
	}

	const T & operator [](const size_t x, const size_t y)const{
		auto & row =  (_mat[y]);
		return row[x];
	}
	
	T & element(const size_t x, const size_t y){
		auto & self = *this;
		return self[x-1, y-1];
	}

	const T & element(const size_t x, const size_t y) const {
		auto & self = *this;
		return self[x-1, y-1];
	}
	
	void fill(const T val){
		auto & self = *this;
		for(size_t y = 0; y < h; y++){
			for(size_t x = 0; x < w; x++){
				self[x,y] = val;
			}
		}
	}
	
	void cross(const T val){
		auto & self = *this;
		for(size_t y = 0; y < h; y++){
			self[y,y] = val;
		}
	}
	
	void plot() const{
		auto & self = *this;
		for(size_t y = 0; y < h; y++){
			DEBUGGER << '[';
			for(size_t x = 0; x < w; x++){
				auto v = self[x,y];
				if(v == UINT32_MAX)DEBUGGER << "inf";
				else DEBUGGER << v;
				DEBUGGER << ",\t";
			}
			DEBUGGER << "],\r\n";
		}
		// DEBUGGER << std::endl;
	}
};

#define scexpr static constexpr

scexpr auto W = 7u;
scexpr auto H = 7u;

using MatrixR = Matrix<size_t, W, H>;
using MatrixS = Matrix<real_t, W, H>;
using Path = std::vector<size_t>;

struct Topology{
	static constexpr size_t INF = UINT32_MAX;
	
	std::vector<Edge> _edges;

	void addEdge(const Edge & edge){
		_edges.push_back(edge);
	}

	void emplaceEdge(const size_t a, const size_t b, const real_t cost){
		_edges.push_back({std::min(a, b), std::max(a,b),cost});
	}
	
	bool connects(size_t a, size_t b) const {
		if(b < a) std::swap(a,b);
		for(const auto & [_a, _b, cost]: _edges){
			if(_a == a and _b == b) return true;
		}
		return false;
	}
	
	void initR(MatrixR & mat) const {
		mat.fill(INF);
		mat.cross(0);
		for(const auto & [a, b, cost]: _edges){
			mat[a-1,b-1] = a;
			mat[b-1,a-1] = b;
		}
	}
	
	void initS(MatrixS & mat) const {
		mat.fill(INF);
		mat.cross(0);
		for(const auto & [a, b, cost]: _edges){
			mat[a-1,b-1] = cost;
			mat[b-1,a-1] = cost;
		}
	}
	
	void forward(MatrixR & r, MatrixS & s, const size_t prog) const{
		for(size_t y = 0; y < H; y++){
			if(y == prog) continue;
			for(size_t x = 0; x < W; x++){
				if(x == prog) continue;
				auto org = s[x,y];
//				if(org == INF) continue;
				
				auto ca = s[prog, y];
				auto cb = s[x, prog];
				auto sum = ca + cb;
				if(ca + cb <= org){
					s[x,y] = sum;
					r[x,y] = prog + 1;
				}
			}
		}
	}
	
	real_t bestCost(MatrixS & s, const size_t from, const size_t to){
		return s[to - 1, from - 1];
	}
	

	
	void bestPath_impl(Path & path, const MatrixR & r, const size_t from, const size_t to) const{
		auto hop = r[to - 1, from - 1];
		
		if(to == hop){
			path.push_back(from);
			return;
		}else{
			path.push_back(hop);
			bestPath_impl(path, r, from, hop);
		}
	}

	void bestPath(Path & path, const MatrixR & r, const size_t from, const size_t to) const{
		path.push_back(to);

		bestPath_impl(path, r, from, to);
	}
};

void plotPath(Path & path){
	DEBUGGER << '[';
	for(auto it = path.rbegin(); it != path.rend(); it++){
		DEBUGGER << *it;
		DEBUGGER << "->";
	}
	DEBUGGER << "]";	
}

// template<typename T>
// void test(const std::span<std::remove_cvref_t<T>> arr) {
//     for (auto it = arr.begin(); it != arr.end(); it++) {
//         DEBUG_PRINTLN(*it);
//     }
// }

// template<typename T>
// void test(const std::span<auto> arr) {
//     for (const auto &item : arr) {
//         DEBUG_PRINTLN(item);
//     }
// }


template <typename T, typename U>
concept SpanConvertible = requires(T t) {
    { std::span<U>{t} } -> std::same_as<std::span<U>>;
};

template <typename T>
// requires SpanConvertible<T, std::remove_cvref_t<typename T::value_type>>
// void test(const std::span<typename T> arr) {
void test(const std::span<T> arr) {
    for (const auto &item : arr) {
        DEBUG_PRINTLN(item);
    }
}

void floyd_main(){
	Topology tp;
	

	tp.emplaceEdge(1, 2, 0.5_r);
	tp.emplaceEdge(1, 3, 2.0_r);
	tp.emplaceEdge(1, 4, 1.5_r);
	
	tp.emplaceEdge(2, 5, 1.2_r);
	tp.emplaceEdge(2, 6, 9.2_r);

	tp.emplaceEdge(3, 5, 5.0_r);
	tp.emplaceEdge(3, 7, 3.1_r);

	tp.emplaceEdge(4, 7, 4.0_r);
	
	tp.emplaceEdge(5, 6, 6.7_r);

	tp.emplaceEdge(6, 7, 15.6_r);
	
	MatrixR r;
	MatrixS s;
	tp.initR(r);
	tp.initS(s);
	
	for(size_t i = 0; i < 7; i++){
		tp.forward(r, s, i);	
	}
	
	Path path;

	r.plot();
	s.plot();
	
	auto from = 3;
	auto to = 6;
	auto cost = tp.bestCost(s, from, to);
	tp.bestPath(path, r, from, to);
	// std::endl
	DEBUGGER << cost << std::endl;

    int arr[] = {1,2,3};
    // __builtin_ass
    // DEBUGGER << std::span<int>(arr);
    // test(std::span<int>(arr));
    test(std::span<const int>(arr));
    // test(arr);
	plotPath(path);
}