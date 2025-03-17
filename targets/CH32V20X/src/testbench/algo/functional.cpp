

template<typename A, typename B>
class Either {
public:
    Either(A a) : a(a), b(nullptr) {}
    Either(B b) : a(nullptr), b(b) {}
    A getA() { return a; }
    B getB() { return b; }
private:
    A a;
    B b;
};