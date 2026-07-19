



// iq16 Interpolation::demo(){
//     auto turnCnt = millis() % 2667;
//     uint32_t turns = millis() / 2667;
    
//     static constexpr iq16 velPoints[7] = {
//         iq16(20)/360, iq16(20)/360, iq16(62.4)/360, iq16(62.4)/360, iq16(20.0)/360, iq16(20.0)/360, iq16(20.0)/360
//     };
    
//     static constexpr iq16 posPoints[7] = {
//         iq16(1.0f)/360,iq16(106.1f)/360,iq16(108.1f)/360, iq16(126.65f)/360, iq16(233.35f)/360,iq16(359.0f)/360,iq16(361.0f)/360
//     };

//     uint16_t tick_points[7] = {
//         0, 300, 400, 500, 2210, 2567, 2667 
//     };

//     int8_t i = 6;

//     while((turnCnt < tick_points[i]) && (i > -1))
//         i--;
    
//     turnSolver.ta = tick_points[i];
//     turnSolver.tb = tick_points[i + 1];
//     uint16_t dt = turnSolver.tb - turnSolver.ta;

//     turnSolver.va = velPoints[i];
//     turnSolver.vb = velPoints[i + 1];
    
//     turnSolver.pa = posPoints[i];
//     turnSolver.pb = posPoints[i + 1];
//     iq16 dp = turnSolver.pb - turnSolver.pa;

//     iq16 _t = ((iq16)(turnCnt  - turnSolver.ta) / (iq16)dt);
//     iq16 temp = (iq16)dt / 1000 / dp; 

//     iq16 yt = 0;

//     if((i == 0) || (i == 2) || (i == 4))
//         yt = cubicBezier(_t, Vec2{iq16(0.4f), 0.4f * turnSolver.va * temp}, Vec2(iq16(0.6f), 1.0f - 0.4f * turnSolver.vb * temp));
//     else
//         yt = _t;

//     iq16 new_pos =  iq16(turns) + turnSolver.pa + dp * yt;

//     return new_pos;
// }

