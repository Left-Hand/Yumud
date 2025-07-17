#pragma once

#include "core/container/ringbuf/Fifo.hpp"
#include <vector>
#include <functional>

namespace gxm{

template<typename T>
class Topic;


template<typename T>
class Publisher{
private:
    Topic<T>& topic_;

public:
    Publisher(Topic<T>& topic) : topic_(topic) {}

    void publish(const T& message);
};

template<typename T>
class Subscriber{
private:
    using Callback = Topic<T>::Callback;
    Callback callback_;

public:
    Subscriber(const Callback callback)
        :callback_(callback) {}

    Callback getCallback() const {
        return callback_;
    }

    void onMessage(const T& message) {
        callback_(message);
    }
};

template<typename T>
class Topic{
public:
    using Callback = std::function<void(const T&)>;
    using Publisher = Publisher<T>;
    using Subscriber = Subscriber<T>;
protected:
    static constexpr size_t N = 16;
    Fifo<T, N> fifo_;

    std::vector<Callback> subscribers_;

    void notify_subscribers() {
        while (!fifo_.empty()) {
            auto msg = fifo_.pop();

            for (auto& subscriber : subscribers_) {
                subscriber(msg);
            }
        }
    }
public:
    void publish(const T & msg){
        fifo_.push(msg);
        notifySubscribers();
    }

    void subscribe(const Callback callback) {
        subscribers_.push_back(callback);
    }

    auto create_publisher(){
        return Publisher<T>(*this);
    }
    auto create_subscriber(const Callback callback) {
        subscribe(callback);
        return Subscriber<T>(callback);
    }
};


template<typename T>
void Publisher<T>::publish(const T& message) {
    topic_.publish(message);
}

}