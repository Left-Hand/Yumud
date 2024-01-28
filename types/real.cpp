#include "real.hpp"

namespace std {
    int mean(const int & a, const int & b){
        return ((a+b) >> 1);
    }

    float mean(const float & a, const float & b){
        return (a+b) / 2.0f;
    }
    
    double mean(const double & a, const double & b){
        return (a+b) / 2.0;
    }

    float frac(const float & fv){
        return (fv - float(int(fv)));
    }

    double frac(const double & dv){
        return (dv - double(int(dv)));
    }

    float round(const float & x)
    {
        return (int)(x+0.5f);
    }

    double round(const double & x)
    {
        return (int)(x+0.5);
    }


    bool is_equal_approx(const float & a, const float & b) {
        // Check for exact equality first, required to handle "infinity" values.
        if (a == b) {
            return true;
        }
        // Then check for approximate equality.
        float tolerance = CMP_EPSILON * std::abs(a);
        if (tolerance < CMP_EPSILON) {
            tolerance = CMP_EPSILON;
        }
        return std::abs(a - b) < tolerance;
    }

    bool is_equal_approx_ratio(const float & a, const float & b, float epsilon, float min_epsilon ) {
        float diff = std::abs(a - b);
        if (diff == 0.0 || diff < min_epsilon) {
            return true;
        }
        float avg_size = (std::abs(a) + std::abs(b)) / 2.0;
        diff /= avg_size;
        return diff < epsilon;
    }
    
    bool is_equal_approx(const double & a, const double & b) {
        // Check for exact equality first, required to handle "infinity" values.
        if (a == b) {
            return true;
        }
        // Then check for approximate equality.
        double tolerance = CMP_EPSILON * std::abs(a);
        if (tolerance < CMP_EPSILON) {
            tolerance = CMP_EPSILON;
        }
        return std::abs(a - b) < tolerance;
    }

    bool is_equal_approx_ratio(const double & a, const double & b, double epsilon, double min_epsilon ) {
        double diff = std::abs(a - b);
        if (diff == 0.0 || diff < min_epsilon) {
            return true;
        }
        double avg_size = (std::abs(a) + std::abs(b)) / 2.0;
        diff /= avg_size;
        return diff < epsilon;
    }
}