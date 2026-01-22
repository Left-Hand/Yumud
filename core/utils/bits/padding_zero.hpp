
#if 0
template<size_t Extents>
struct PaddingZero{
    using Self = PaddingZero;
    [[nodiscard]] constexpr RepeatIter<uint8_t, Extents> to_bytes() const {
        return RepeatIter<uint8_t, Extents>(0, Extents);
    };

    [[nodiscard]] static constexpr Self from_bytes(std::span<const uint8_t, Extents>){;}
};
#endif