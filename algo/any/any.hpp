#ifndef __ANY_HPP__

#include <memory>
#include <functional>
#include <type_traits>
#include <utility>

namespace base{

class Any
{
    class AnyConcept
    {
    public:
        virtual ~AnyConcept() {};
        virtual void run() = 0;

    protected:
        AnyConcept() = default;
        AnyConcept(AnyConcept const &) = default;
        AnyConcept(AnyConcept &&) noexcept = default;
        AnyConcept &operator=(AnyConcept &&) noexcept = default;
        AnyConcept &operator=(AnyConcept &) = default;
    };

    template <typename ConcreteType>
    class AnyModel final : public AnyConcept
    {
    public:

        using CallBackFunc = std::function<void(ConcreteType &)>;
    public:
        explicit AnyModel(ConcreteType &&concrete)
            // Copy-construct or move-construct the concrete type.
            : object_{std::forward<ConcreteType>(concrete)}
        {
            this->callback_ = [this](ConcreteType & obj)
            { this->object_.run(); };
        }

        AnyModel(ConcreteType&& concrete, CallBackFunc callback)
        : object_{std::forward<ConcreteType>(concrete)}
        , callback_{std::move(callback)}
        {
        }

        ~AnyModel() = default;

        void run() override
        {
            if(this->callback_)
                this->callback_(this->object_);
        }

    private:
        ConcreteType object_;
        CallBackFunc callback_;
    };

public:
    
    template <typename ConcreteType>
    explicit Any(ConcreteType &&concrete)
        // Strip the reference, so that either a copy or a move occurs, but not reference binding.
        : pimpl_{std::make_unique<AnyModel<std::remove_reference_t<ConcreteType>>>(
              std::forward<std::remove_reference_t<ConcreteType>>(concrete))}
    {
    }

    template <typename ConcreteType, typename Func>
    Any(ConcreteType &&concrete, Func callback)
        : pimpl_{std::make_unique<AnyModel<std::remove_reference_t<ConcreteType>>>(
              std::forward<std::remove_reference_t<ConcreteType>>(concrete), std::move(callback))}
    {
    }

    void run()
    {
        pimpl_->run();
    }

    Any(Any const &other) = delete;
    Any &operator=(Any const &rhs) = delete;

    // unique_ptr is movable by default.
    Any(Any &&rhs) noexcept
    {
        if(&rhs == this)
            return;
        pimpl_ = std::move(rhs.pimpl_);
    }
    Any &operator=(Any && rhs) noexcept
    {
        if(&rhs == this)
            return *this;
        pimpl_ = std::move(rhs.pimpl_);
        return *this;
    }

    ~Any() = default;

private:
    // Pointer to a ShapeModel, which has a member object of the concrete type.
    std::unique_ptr<AnyConcept> pimpl_;
};


}


#endif