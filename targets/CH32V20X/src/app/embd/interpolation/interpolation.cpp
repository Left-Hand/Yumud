#include "interpolation.hpp"

static Interpolation::TurnSolver turnSolver;

real_t Interpolation::cubicBezier(const real_t x, const Vector2 & a,const Vector2 & b){

    real_t y0a = 0.0; // initial y
    real_t x0a = 0.0; // initial x 
    real_t y1a = a.y;    // 1st influence y   
    real_t x1a = a.x;    // 1st influence x 
    real_t y2a = b.y;    // 2nd influence y
    real_t x2a = b.x;    // 2nd influence x
    real_t y3a = 1.0; // final y 
    real_t x3a = 1.0; // final x 

    real_t A =   x3a - 3.0*x2a + 3.0*x1a - x0a;
    real_t B = 3.0*x2a - 6.0*x1a + 3.0*x0a;
    real_t C = 3.0*x1a - 3.0*x0a;   
    real_t D =   x0a;

    real_t E =   y3a - 3.0*y2a + 3.0*y1a - y0a;    
    real_t F = 3.0*y2a - 6.0*y1a + 3.0*y0a;             
    real_t G = 3.0*y1a - 3.0*y0a;             
    real_t H =   y0a;

    real_t currentt = x;
    for (int i=0; i < 5; i++){
        real_t currentx = xFromT (currentt, A,B,C,D); 
        real_t currentslope = slopeFromT (currentt, A,B,C);
        currentt -= (currentx - x)*(currentslope);
        currentt = CLAMP(currentt,real_t(0),real_t(1)); 
    } 

    real_t y = yFromT (currentt,  E,F,G,H);
    return y;
}

real_t Interpolation::ss(){
    auto turnCnt = millis() % 2667;
    uint32_t turns = millis() / 2667;
    
    real_t velPoints[7] = {
        20.0f/360, 20.0f/360, 62.4f/360, 62.4f/360, 20.0f/360, 20.0f/360, 20.0f/360
    };
    
    real_t posPoints[7] = {
        1.0f/360, 106.1f/360, 108.1f/360, 126.65f/360, 233.35f/360, 359.0f/360, 361.0f/360
    };

    uint16_t tickPoints[7] = {
        0, 300, 400, 500, 2210, 2567, 2667 
    };

    int8_t i = 6;

    while((turnCnt < tickPoints[i]) && (i > -1))
        i--;
    
    turnSolver.ta = tickPoints[i];
    turnSolver.tb = tickPoints[i + 1];
    uint16_t dt = turnSolver.tb - turnSolver.ta;

    turnSolver.va = velPoints[i];
    turnSolver.vb = velPoints[i + 1];
    
    turnSolver.pa = posPoints[i];
    turnSolver.pb = posPoints[i + 1];
    real_t dp = turnSolver.pb - turnSolver.pa;

    real_t _t = ((real_t)(turnCnt  - turnSolver.ta) / (real_t)dt);
    real_t temp = (real_t)dt / 1000 / dp; 

    real_t yt = 0.0f;

    if((i == 0) || (i == 2) || (i == 4))
        yt = cubicBezier(_t, Vector2{real_t(0.4f), 0.4f * turnSolver.va * temp}, Vector2(real_t(0.6f), 1.0f - 0.4f * turnSolver.vb * temp));
    else
        yt = _t;

    real_t new_pos =  real_t(turns) + turnSolver.pa + dp * yt;
    // turnSolver.pos = new_pos;

    // stp.setTargetPosition(new_pos);
    return new_pos;
    // stp.setTargetVector(new_pos*2);
}