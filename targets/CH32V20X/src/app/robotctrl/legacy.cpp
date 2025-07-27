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
struct QueuePointIterator{
    struct Config{
        std::span<const Vector2<bf16>> points;
    };

    explicit constexpr QueuePointIterator(
        const Config & cfg
    ):
        points_(cfg.points){;}

    [[nodiscard]] constexpr Vector2<q24> next(const q24 step){

        const auto curr_index = index_;

        const auto p1 = Vector2{
            q24::from(float(points_[curr_index].x)), 
            q24::from(float(points_[curr_index].y))
        };


        current_position = vec_step_to(current_position, p1, step);

        if(current_position.is_equal_approx(p1)){
            index_ = (index_ + 1) % points_.size();
        }

        return current_position;
    }

    [[nodiscard]] constexpr size_t index() const {
        return index_;
    }
private:
    std::span<const Vector2<bf16>> points_;
    Vector2<q24> current_position = {};
    size_t index_ = 0;
};

#endif