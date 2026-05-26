



class MotorLesoFloat {
public:
    using State = SecondOrderState<float>;
    struct Coeffs {
        float b0;
        float dt;
        float g1;
        float g2;
    };

    struct Config {
        float fs;
        float fc;
        float b0;

        Result<Coeffs, const char*> try_into_coeffs() const noexcept {
            if (fs <= 0.0f) return Err("fs must be positive");
            if (fc <= 0.0f) return Err("fc must be positive");
            if (2.0f * fc >= fs) return Err("fc too large");

            const float dt = 1.0f / fs;
            const float g1 = 2.0f * fc;
            const float g2 = fc * fc;

            return Ok(Coeffs{.b0 = b0, .dt = dt, .g1 = g1, .g2 = g2});
        }
    };
    explicit MotorLesoFloat(const Coeffs& coeffs) : coeffs_(coeffs) {}

    State iterate(const State& state, float y, float u) const noexcept {
        const float e = y - state.x1;
        const float delta_x1 = (state.x2 + u * coeffs_.b0) * coeffs_.dt + e * coeffs_.g1;
        const float delta_x2 = e * coeffs_.g2;

        return State{
            state.x1 + delta_x1,
            state.x2 + delta_x2
        };
    }

private:
    Coeffs coeffs_;
};