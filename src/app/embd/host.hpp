#pragma once

#include "stepper/constants.hpp"
#include "stepper/cli.hpp"


class RemoteStepper{
public:
    // void loadArchive() = 0;
    // virtual void saveArchive() = 0;
    // virtual void removeArchive() = 0;
    // virtual bool autoload() = 0;

    // virtual void setTargetCurrent(const real_t current) = 0;
    // virtual void setTargetSpeed(const real_t speed) = 0;
    // virtual void setTargetPosition(const real_t pos) = 0;
    // virtual void setTagretTrapezoid(const real_t pos) = 0;
    // virtual void setOpenLoopCurrent(const real_t current) = 0;
    // virtual void setTargetVector(const real_t pos) = 0;
    // virtual void setCurrentClamp(const real_t max_current) = 0;
    // virtual void locateRelatively(const real_t pos = 0) = 0;

    // virtual bool isActive() const = 0;
    // virtual const volatile RunStatus & status() = 0;

    // virtual real_t getSpeed() const = 0;
    // virtual real_t getPosition() const = 0;
    // virtual real_t getCurrent() const = 0;

    // virtual void setTargetPositionClamp(const Range & clamp) = 0;
    // virtual void enable(const bool en = true) = 0;
    // virtual void setNodeId(const uint8_t _id) = 0;
    // virtual void setSpeedClamp(const real_t max_spd) = 0;
    // virtual void setAccelClamp(const real_t max_acc) = 0;

    // virtual void triggerCali() = 0;

    
};


class EmbdHost(){

};