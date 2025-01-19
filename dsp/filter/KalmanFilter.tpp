template<arithmetic T>
const T KalmanFilter_t<T>::update(const arithmetic auto _x) {
    T x = static_cast<T>(_x);
    if (!inited) {
        x_last = x;
        p_last = T(0);
        inited = true;
    }else {
        x_last = predict(x);
        p_last = (1-kg)*p_mid;
    }
    
    return x_last;
}

template<arithmetic T>
const T KalmanFilter_t<T>::predict(const arithmetic auto _x) {
    T x = static_cast<T>(_x);
    p_mid = p_last + q;
    kg = p_mid / (p_mid + r);
    return (x_last + kg * (x - x_last));
}