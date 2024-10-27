#pragma once

#include "motion_module.hpp"
#include "wheel/wheel.hpp"

namespace gxm{
class ChassisModule:public MotionModule{
public:
    struct Config{
        Mecanum4Solver::Config solver_config;
        Wheel::Config wheel_config;
    };
    

    class Wheels{
    protected:
        std::array<Wheel *, 4> instances;
    public:
        Wheels(Wheel & w1, Wheel & w2, Wheel & w3, Wheel & w4):
            instances{&w1, &w2, &w3, &w4}{;}
    
        void setSpeed(const std::tuple<real_t, real_t, real_t, real_t> & spds){
            #define SET_SPD(n) instances[n]->setSpeed(std::get<n>(spds))
            SET_SPD(0);
            SET_SPD(1);
            SET_SPD(2);
            SET_SPD(3);
        }

        void setPosition(const std::tuple<real_t, real_t, real_t, real_t> & pos){
            #define SET_POS(n) instances[n]->setPosition(std::get<n>(pos))
            SET_POS(0);
            SET_POS(1);
            SET_POS(2);
            SET_POS(3);
        }

        void forwardPosition(const std::tuple<real_t, real_t, real_t, real_t> & pos){
            #define FWD_POS(n) instances[n]->forwardPosition(std::get<n>(pos))
            FWD_POS(0);
            FWD_POS(1);
            FWD_POS(2);
            FWD_POS(3);
        }
    
        std::tuple<real_t, real_t, real_t, real_t> getPosition(){
            return std::make_tuple(
                instances[0]->getPosition(),
                instances[1]->getPosition(),
                instances[2]->getPosition(),
                instances[3]->getPosition()
            );
        }

        std::tuple<real_t, real_t, real_t, real_t> getSpeed(){
            return std::make_tuple(
                instances[0]->getSpeed(),
                instances[1]->getSpeed(),
                instances[2]->getSpeed(),
                instances[3]->getSpeed()
            );
        }
    
        Wheel & operator [](const size_t idx){
            return *instances[idx];
        }
    };

    const Config & config_;
    Wheels wheels_;

    Mecanum4Solver solver_{config_.solver_config};
    
public:
    ChassisModule(const Config & config, const Wheels & wheels):config_(config), wheels_(wheels) {}

    void positionTrim(const Vector2 & trim);
    void rotationTrim(const real_t raderr);
    void forwardMove(const Vector2 & vel, const real_t spinrate);

    void calibratePosition(const Vector2 & pos);
    void calibrateRotation(const real_t rad);

    void tick();

    const auto & config()const {return config_;}
};

}