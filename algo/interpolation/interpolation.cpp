#include "Interpolation.hpp"



// real_t Interpolation::demo(){
//     auto turnCnt = millis() % 2667;
//     uint32_t turns = millis() / 2667;
    
//     scexpr real_t velPoints[7] = {
//         real_t(20)/360, real_t(20)/360, real_t(62.4)/360, real_t(62.4)/360, real_t(20.0)/360, real_t(20.0)/360, real_t(20.0)/360
//     };
    
//     scexpr real_t posPoints[7] = {
//         real_t(1.0f)/360,real_t(106.1f)/360,real_t(108.1f)/360, real_t(126.65f)/360, real_t(233.35f)/360,real_t(359.0f)/360,real_t(361.0f)/360
//     };

//     uint16_t tickPoints[7] = {
//         0, 300, 400, 500, 2210, 2567, 2667 
//     };

//     int8_t i = 6;

//     while((turnCnt < tickPoints[i]) && (i > -1))
//         i--;
    
//     turnSolver.ta = tickPoints[i];
//     turnSolver.tb = tickPoints[i + 1];
//     uint16_t dt = turnSolver.tb - turnSolver.ta;

//     turnSolver.va = velPoints[i];
//     turnSolver.vb = velPoints[i + 1];
    
//     turnSolver.pa = posPoints[i];
//     turnSolver.pb = posPoints[i + 1];
//     real_t dp = turnSolver.pb - turnSolver.pa;

//     real_t _t = ((real_t)(turnCnt  - turnSolver.ta) / (real_t)dt);
//     real_t temp = (real_t)dt / 1000 / dp; 

//     real_t yt = 0;

//     if((i == 0) || (i == 2) || (i == 4))
//         yt = cubicBezier(_t, Vec2{real_t(0.4f), 0.4f * turnSolver.va * temp}, Vec2(real_t(0.6f), 1.0f - 0.4f * turnSolver.vb * temp));
//     else
//         yt = _t;

//     real_t new_pos =  real_t(turns) + turnSolver.pa + dp * yt;

//     return new_pos;
// }

