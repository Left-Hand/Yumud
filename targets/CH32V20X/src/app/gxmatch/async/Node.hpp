#pragma once

#include "core/buffer/ringbuf/Fifo_t.hpp"
#include <vector>
#include <functional>

namespace gxm{

template<typename T>
class Topic_t;


template<typename T>
class Publisher_t{
private:
    Topic_t<T>& topic_;

public:
    Publisher_t(Topic_t<T>& topic) : topic_(topic) {}

    void publish(const T& message);
};

template<typename T>
class Subscriber_t{
private:
    using Callback = Topic_t<T>::Callback;
    Callback callback_;

public:
    Subscriber_t(const Callback callback)
        :callback_(callback) {}

    Callback getCallback() const {
        return callback_;
    }

    void onMessage(const T& message) {
        callback_(message);
    }
};

template<typename T>
class Topic_t{
public:
    using Callback = std::function<void(const T&)>;
    using Publisher = Publisher_t<T>;
    using Subscriber = Subscriber_t<T>;
protected:
    Fifo_t<T, 32> fifo_;

    std::vector<Callback> subscribers_;

    void notifySubscribers() {
        while (!fifo_.empty()) {
            auto msg = fifo_.pop();

            for (auto& subscriber : subscribers_) {
                subscriber(msg);
            }
        }
    }
public:
    void publish(const T& msg){
        fifo_.push(msg);
        notifySubscribers();
    }

    void subscribe(const Callback callback) {
        subscribers_.push_back(callback);
    }

    auto createPublisher(){
        return Publisher_t<T>(*this);
    }
    auto createSubscriber(const Callback callback) {
        subscribe(callback);
        return Subscriber_t<T>(callback);
    }
};


template<typename T>
void Publisher_t<T>::publish(const T& message) {
    topic_.publish(message);
}

}