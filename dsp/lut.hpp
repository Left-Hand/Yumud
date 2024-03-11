#ifndef __LUT_HPP__

#define __LUT_HPP__

template <typename Real, int N>
class LookUpTable {
protected:
    std::array<Real, N> values;
public:
    LookUpTable(std::array<Real, N> initializedValues) : values(initializedValues) {}
    virtual Real operator[](const uint32_t & i) const = 0;
    virtual Real operator[](const Real & x) const = 0;
};

template <typename Real, int N>
class SinTable : public LookUpTable<Real, N> {
public:
    constexpr SinTable() : LookUpTable<Real, N>([]{
        std::array<Real, N> arr {};
        for (int i = 0; i < N; i++) {
            arr[i] = static_cast<Real>(std::sin(i * TAU / N));
        }
        return arr;
    }()) {}


    Real operator[](const uint32_t & i) const override {
        return this->values[i % N];
    }

    Real operator[](const Real & x) const override {
        return this->values[static_cast<int>(x * N) % N];
    }
};

#endif