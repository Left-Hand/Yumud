#if 0
struct AxisIterator{
    struct State{
        real_t position;
        real_t speed;
    };

private:
    State state_;
};

struct PolarRobotGestureIterator{
    struct Config{
        real_t max_joint_spd;
        real_t max_joint_acc;

        Range2<real_t> rho_range;
        Range2<real_t> theta_range;

        real_t r;
    };

    struct MachineStates{
        Vector2<real_t> position;
        Vector2<real_t> speed;
    };

    struct JointStates{
        real_t position;
        real_t speed;
    };

    struct State{
        MachineStates machine;
        JointStates radius_joint;
        JointStates theta_joint;
    };

    struct Solution{
        real_t rho_joint_rotation;
        real_t theta_joint_rotation;
    };

    Solution forward(const Vector2<real_t> target_pos, const real_t delta_t){
        State state = state_;
        return {
            .rho_joint_rotation = state.radius_joint.position, 
            .theta_joint_rotation = state.theta_joint.position
        };
    }

private:
    State state_;
};

class CanHandlerAdaptor_PolarRobotActuator final:
public CanMsgHandlerIntf{
    HandleStatus handle(const hal::CanMsg & msg){ 
        if(not msg.is_standard()) 
            return HandleStatus::from_unhandled();
        switch(msg.id_as_u32()){
            case 0:
                DEBUG_PRINTLN("o"); break;
            default:
                return HandleStatus::from_unhandled();
        }
        return HandleStatus::from_handled();
    }
};


#endif