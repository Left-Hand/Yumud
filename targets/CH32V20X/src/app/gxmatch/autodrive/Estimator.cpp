// #include "Estimator.hpp"

// using namespace ymd::drivers;
// using namespace gxm;

// void Estimator::reset(){

// }



// real_t Estimator::calculate_raw_dir(const Vector3 & mag) const {
//     return -atan2(mag.y, mag.x);
// }

// void Estimator::update(const real_t time){
//     // acc_gyr_sensor_.update();
//     // mag_sensor_.update();

//     // acc3_ = bias_.acc.xform(Vector3(acc_gyr_sensor_.getAcc()));
//     // gyr3_ = Vector3(acc_gyr_sensor_.getGyr()) - bias_.gyr;
//     // mag3_ = bias_.mag.xform(Vector3(mag_sensor_.getMagnet()));

//     // quat_ = mahony_.update9(gyr3_, acc3_, mag3_);
//     // euler_ = Basis(quat_).get_euler_xyz();

//     acc_gyr_sensor_.update();
//     mag_sensor_.update();

//     acc3_raw = Vector3{acc_gyr_sensor_.getAcc()};
//     gyr3_raw = Vector3{acc_gyr_sensor_.getGyr()};
//     mag3_raw = Vector3{mag_sensor_.getMagnet()};
//     // mag3_raw = Vector3{};
//     const auto rot_raw = calculate_raw_dir(mag3_raw);
//     gyr_ = gyr3_raw.z;
//     rot_ = rot_obs_.update(rot_raw, gyr_, time) - bias_.rot;



//     // acc3_ = bias_.acc.xform(acc3_raw);
//     // gyr3_ = Vector3(gyr3_raw - bias_.gyr);
//     // mag3_ = bias_.mag.xform(mag3_raw);


//     // DEBUG_PRINTLN(mag3_raw);
//     // DEBUG_PRINTLN(mag3_raw.x, mag3_raw.y, mag3_raw.z);



//     // flow_sensor_.update(rot_ + real_t(PI/2));

//     // const auto new_pos = Vector2(flow_sensor_.getPosition());
//     // const auto delta_pos = new_pos - pos_;
//     // pos_ = flow_sensor_.getPosition();

//     // const auto delta_t = time - last_time;
//     // last_time = time;

//     // const auto new_spd = delta_pos / delta_t;
//     // spd_ = spd_lpf_.update(new_spd);

//     // const auto [x,y] = pos_;
//     // DEBUG_PRINTLN(rot_, gyr_, x, y);
// }

// void Estimator::recalibrate(const Vector2 & _pos, const real_t _rot){
//     TODO();
//     pos_ = _pos;
//     rot_ = _rot;
// }

// Quat Estimator::calculateAccBias(){
//     Vector3 acc_bias_sum = Vector3::ZERO;
    
//     for(size_t i = 0; i < config_.calibrate_times; i++){
//         acc_gyr_sensor_.update();
//         acc_bias_sum += Vector3(acc_gyr_sensor_.getAcc());
//     }

//     const auto acc_vec3 = acc_bias_sum / config_.calibrate_times;
//     return Quat(Vector3(0,0,-1), acc_vec3.normalized()).inverse();
// }

// Vector3 Estimator::calculateGyrBias(){
//     Vector3 gyr_bias_sum = Vector3::ZERO;
    
//     for(size_t i = 0; i < config_.calibrate_times; i++){
//         acc_gyr_sensor_.update();
//         gyr_bias_sum += Vector3(acc_gyr_sensor_.getGyr());
//     }

//     return gyr_bias_sum / config_.calibrate_times;
// }


// Quat Estimator::calculateMagBias(){
//     Vector3 mag_bias_sum = Vector3::ZERO;
    
//     for(size_t i = 0; i < config_.calibrate_times; i++){
//         mag_sensor_.update();
//         mag_bias_sum += Vector3(mag_sensor_.getMagnet());
//     }

//     const auto mag_vec3 =  mag_bias_sum / config_.calibrate_times;
//     return Quat(Vector3(0,0,-1), mag_vec3.normalized()).inverse();
// }


// void Estimator::calibrate(){
//     // bias_.acc = calculateAccBias();
//     bias_.acc = Vector3();
//     // bias_.gyr = calculateGyrBias();
//     bias_.gyr = Vector3();
//     // bias_.mag = calculateMagBias();
//     bias_.rot = calculate_raw_dir(Vector3{mag_sensor_.getMagnet()});
//     // DEBUG_PRINTLN(bias_.acc, bias_.gyr, bias_.mag)
// }


// void Estimator::init()
// {
//     if(config_.force_calibrate){
//         calibrate();
//     }else{
//         bias_ = config_.pre_bias;
//     }
// }