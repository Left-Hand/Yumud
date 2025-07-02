#pragma once

#include <cmath>
#include <numbers>


namespace dsp {

template<typename T>
class OnePoleFilter {
public:
    enum class Type {
        kLowpass,
        kHighpass
    };

    void  Init(T sampleRate){
        sampleRate_ = sampleRate;
    }
    void  SetCutoffLPF(T freq){
        freq_ = freq;
        freq = std::min(freq, GetMaxLowpassFreq());
        T omega = 2 * std::numbers::pi_v<T> * freq / sampleRate_;
        auto k = std::tan(omega / 2);
        b0_ = k / (1 + k);
        b1_ = b0_;
        a1_ = (k - 1) / (k + 1);
        type_ = Type::kLowpass;
    }
    void  SetCutoffHPF(T freq){
        freq_ = freq;
        T omega = 2 * std::numbers::pi_v<T> * freq / sampleRate_;
        auto k = std::tan(omega / 2);
        b0_ = 1 / (1 + k);
        b1_ = -b0_;
        a1_ = (k - 1) / (k + 1);
        type_ = Type::kHighpass;
    }
    T GetMagPowerResponce(T omega){
        auto cosv = std::cos(omega);
        auto up = b1_ * b1_ + b0_ * b0_ + 2 * b0_ * b1_ * cosv;
        auto down = 1 + a1_ * a1_ + 2 * a1_ * cosv;
        return up / down;
    }
    T GetMaxLowpassFreq() const;
    T GetFreq() const { return freq_; }
    T GetPhaseDelay(T freq) const{
        auto omega = 2.0f * std::numbers::pi_v<T> * freq / sampleRate_;
        auto z = std::polar(1.0f, omega);
        auto up = b0_ * z + b1_;
        auto down = z + a1_;
        auto response = up / down;
        auto phase = std::arg(response);
        // first order filter always in -pi ~pi, no need unwarp
        return -phase / omega;
    }
    void  CopyCoeff(const OnePoleFilter& other){
        freq_ = other.freq_;
        b0_ = other.b0_;
        b1_ = other.b1_;
        a1_ = other.a1_;
        type_ = other.type_;
    }

    void ClearInteral() {
        latch1_ = 0;
    }

    T Process(T in) {
        auto t = in - a1_ * latch1_;
        auto y = t * b0_ + b1_ * latch1_;
        latch1_ = t;
        return y;
    }
private:
    T sampleRate_ = 0;
    T freq_ = 0;
    T b0_ = 0;
    T b1_ = 0;
    T a1_ = 0;
    T latch1_ = 0;
    Type type_ = Type::kLowpass;
};


}