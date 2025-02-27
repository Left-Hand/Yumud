#pragma once

// General template for IIR filter
template<arithmetic T, size_t N>
class IIR {
public:
    // Constructor to initialize coefficients
    IIR(const std::array<T, N+1>& feedforward_coeffs, const std::array<T, N>& feedback_coeffs) 
        : b(feedforward_coeffs), a(feedback_coeffs) {
        x.fill(0.0); // Initialize input history to zero
        y.fill(0.0); // Initialize output history to zero
    }

    // Filter function
    T filter(T input) {
        // Shift input history
        for (size_t i = N; i > 0; --i) {
            x[i] = x[i-1];
        }
        x[0] = input;

        // Calculate output using difference equation
        T output = 0.0;
        for (size_t i = 0; i <= N; ++i) {
            output += b[i] * x[i];
        }
        for (size_t i = 1; i <= N; ++i) {
            output -= a[i-1] * y[i];
        }

        // Shift output history
        for (size_t i = N-1; i > 0; --i) {
            y[i] = y[i-1];
        }
        y[0] = output;

        return output;
    }

private:
    std::array<T, N+1> b; // Feedforward coefficients
    std::array<T, N> a;   // Feedback coefficients
    std::array<T, N+1> x; // Input history
    std::array<T, N> y;   // Output history
};