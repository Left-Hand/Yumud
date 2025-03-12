#include "mahony.hpp"
#include "sys/debug/debug.hpp"

using namespace ymd;

// #pragma GCC optimize("Ofast,unroll-loops")


void Mahony::update(const Vector3 & gyr,const Vector3 & acc) {
	const auto v = q.xform_top();
	const auto e = acc.normalized().cross(v);

	inte_ += e * inv_fs_;
	Quat delta_q = q.xform((gyr + kp_ * e + ki_ * inte_) * (inv_fs_));
	q = (q * delta_q).normalized();
}