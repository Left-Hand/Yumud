#pragma once

namespace ymd{
template<typename ... Args>
struct PhantomMarker {
    // Default constructor
    constexpr PhantomMarker() noexcept = default;

    // Copy/move constructors and assignments (defaulted)
    constexpr PhantomMarker(const PhantomMarker&) noexcept = default;
    constexpr PhantomMarker(PhantomMarker&&) noexcept = default;
    PhantomMarker& operator=(const PhantomMarker&) noexcept = default;
    PhantomMarker& operator=(PhantomMarker&&) noexcept = default;

    // No members - empty class
};
}