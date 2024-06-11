#ifndef __ODOMETER_HPP__

#define __ODOMETER_HPP__

#include "types/real.hpp"
#include "Encoder.hpp"
#include <array>

// #include "OdometerLines.hpp"

namespace MotorUntils{

};

// class Odometer;
// class OdometerLines;
// class OdometerPoles;


class Odometer{
protected:
// public:
    real_t rawLapPosition = real_t(0);

    real_t lapPosition = real_t(0);
    real_t lapPositionLast = real_t(0);
    real_t deltaLapPosition = real_t(0);

    real_t accPosition = real_t(0);
    real_t accPositionLast = real_t(0);

    Encoder & encoder;

    bool rsv = false;

    void locate(const real_t & pos){
        update();
        lapPosition = getLapPosition();
        lapPositionLast = lapPosition;
        accPosition = pos;
    }

    virtual real_t correctPosition(const real_t rawPosition){
        return rawPosition;
    }

public:
    Odometer(Encoder & _encoder):encoder(_encoder){;}

    real_t getRawLapPosition(){
        return rawLapPosition;
    }

    real_t getLapPosition(){
        return lapPosition;
    }



    void init(){
        encoder.init();
    }

    void locateRelatively(const real_t & offset = real_t(0)){
        locate(offset);
    }

    void locateAbsolutely(const real_t & offset = real_t(0)){
        locate(getRawLapPosition() + offset);
    }

    virtual void reset(){
        rawLapPosition = real_t(0);
        lapPosition = real_t(0);
        lapPositionLast = real_t(0);

        accPosition = real_t(0);
        accPositionLast = real_t(0);
    }

    void inverse(const bool en = true){
        rsv = en;
    }

    void update(){
        {
            real_t undiredRawLapPostion = encoder.getLapPosition();
            if (rsv) rawLapPosition = real_t(1) - undiredRawLapPostion;
            else rawLapPosition = undiredRawLapPostion;
        }

        lapPosition = correctPosition(rawLapPosition);
        deltaLapPosition = lapPosition - lapPositionLast;

        if(deltaLapPosition > real_t(0.5f)){
            deltaLapPosition -= real_t(1);
        }else if (deltaLapPosition < real_t(-0.5f)){
            deltaLapPosition += real_t(1);
        }

        lapPositionLast = lapPosition;
        accPosition += deltaLapPosition;
    }

    virtual real_t getPosition(){
        return accPosition;
    }
};


class OdometerScaled:public Odometer{
protected:
    real_t scale;
public:
    OdometerScaled(Encoder & _encoder, const real_t & _scale):
        Odometer(_encoder),scale(_scale){;}
};

// template<int poles>
class OdometerPoles:public Odometer{
// protected:
public:
    static constexpr int poles = 50;
    real_t elecRad = real_t(0);
    real_t elecRadOffset = real_t(0);

    std::array<real_t, poles>cali_map;
    // std::array<int, poles>cali_map_scores;

    real_t position2rad(const real_t & position){
        real_t frac1 = poles * frac(position);
        return TAU * (frac(frac1));
    }

    int position2pole(const iq_t & position){
        real_t pole = frac(position) * real_t(poles);
        return MIN(int(pole), poles - 1);
    }

    real_t pole2position(const int & pole){
        return real_t(pole) / 50;
    }

    real_t correctPosition(const real_t rawPosition) override{
        // uint8_t pole_a = position2pole(rawPosition);
        // uint8_t pole_b = warp_mod(pole_a, 50);

        // real_t p_a = cali_map[pole_a];
        // real_t p_b = cali_map[pole_b];
        // real_t ratio = (rawPosition - pole2position(pole_a)) * 50;
        // return LERP(ratio, p_a, p_b) + rawPosition;
        return (rawPosition + cali_map[position2pole(rawPosition)]);
    }

public:
    OdometerPoles(Encoder & _encoder):Odometer(_encoder){;}

    void reset() override{
        Odometer::reset();
        elecRad = real_t(0);
        elecRadOffset = real_t(0);
        cali_map.fill(real_t(0));
        // cali_map_scores.fill(0);
    }

    real_t getElecRad(){
        return position2rad(getLapPosition()) - elecRadOffset;
    }

    real_t getElecRadFixed(){
        return position2rad(getLapPosition() + deltaLapPosition);
    }

    real_t getElecRadOffset(){
        return elecRadOffset;
    }

    // real_t getElecRadOffset(){
    //     return = position2rad(getLapPosition());
    // }

    void setElecRadOffset(const real_t & new_offset){
        elecRadOffset = new_offset;
    }

    void addPostDynamicFixPosition(const real_t & err){
        elecRadOffset += err * 50 * TAU;
    }

    int getRawPole(){
        return position2pole(getRawLapPosition());
    }

    int warp_mod(const int & x, const int & y){
        int ret = x % y;
        if(ret < 0) ret += y;
        return ret;
    }

    real_t warp_around_zero(const real_t & x){
        // return warp_mod(x.value, iq_t(1).value);
        // real_t ret = x;
        // while(ret > 1){
        //     ret -= 1;
        //     return ret;
        // }
        // while(ret < -1){
        //     ret += 1;
        //     return ret;
        // }


        return fmod(frac(x - 0.5) - 0.5, real_t(0.02));
    }


    // void addCaliPoint(const real_t & correct_position, const int pole){
    //     int index = warp_mod(pole, poles);

    //     cali_map[index] += warp_around_zero(frac(correct_position) - rawLapPosition);
    //     cali_map_scores[index] ++;
    //     // cali_map[position2pole(getRawLapPosition())] += (correct_position - rawLapPosition) * percent;
    // }

    // void runCaliAnalysis(){
    //     for(int i = 0; i < poles; i++){
    //         if(cali_map_scores[i]){
    //             cali_map[i] /= real_t(cali_map_scores[i]);
    //             cali_map[i] = warp_around_zero(cali_map[i]);
    //         }
    //     }

        // for(int i = 0; i < poles; i++){
        //     if(!cali_map_scores[i]){
        //         cali_map[i] = mean(cali_map[warp_mod(i - 1, poles)], cali_map[warp_mod(i + 1, poles)]);
        //     }
        // }
    // }
};

#endif