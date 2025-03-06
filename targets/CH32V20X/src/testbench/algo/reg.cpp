#include "../tb.h"

#define pub public:
#define prv private:

template <size_t Size>
struct bytes_to_uint;

template <>
struct bytes_to_uint<1> {
    using type = uint8_t;
};

template <>
struct bytes_to_uint<2> {
    using type = uint16_t;
};

template <>
struct bytes_to_uint<4> {
    using type = uint32_t;
};

template<size_t Bytes>
using bytes_to_uint_t = typename bytes_to_uint<Bytes>::type;

template<size_t Bits>
using bits_to_uint_t = typename bytes_to_uint<(Bits + 7) / 8>::type;

template<typename T>
static constexpr size_t type_to_bits_v = sizeof(T) * 8;

template<typename T>
static constexpr size_t type_to_bytes_v = sizeof(T) * 8;

template<typename T>
struct reg_data_type{};

// template<typename T>
// struct reg_data_type<Reg>{using type = T;};

template<typename T>
__inline static constexpr 
T lower_mask_of(const size_t b){
    return (1 << b) - 1;
}

template<typename T>
__inline static constexpr 
T mask_of(const size_t b_bits, const size_t e_bits){
    const T lower_mask = lower_mask_of<T>(b_bits);
    const T upper_mask = lower_mask_of<T>(e_bits);
    return upper_mask & ~lower_mask;
}

template<typename D>
__inline static constexpr 
bool is_under_bits(const size_t bits, const D && val){
    using T = std::decay_t<D>;

    const T lower_mask = lower_mask_of<T>(bits);
    return val <= lower_mask;
}

template<typename T>
struct BitFieldCrtp{
    __inline constexpr
    T as_mask() const{
        return static_cast<T *>(this)->as_mask();
    }

    __inline constexpr 
    T as_unshifted() const{
        return static_cast<T *>(this)->as_unshifted();
    }
    __inline constexpr 
    T as_val() const{
        return static_cast<T *>(this)->as_unshifted();
    }
};

// template<typename TDummy>
// struct BitField{};

template<
	typename D,
	size_t b_bits, size_t e_bits
>
struct BitField{
private:    
    using T = std::decay_t<D>;
    D & value_;

    static constexpr T lower_mask = lower_mask_of<T>(b_bits);
	static constexpr T mask = mask_of<T>(b_bits, e_bits);

    pub static constexpr size_t bits_len = e_bits - b_bits;

    template<typename I_raw>
    static constexpr void apply_unshift_to_data(D & data, I_raw && in){
        data = (in << b_bits) | (data & ~mask);
    }
public:
    constexpr 
    explicit BitField(D & value):value_(value){;}

    template<typename I_raw>
    __inline constexpr 
    auto & operator =(I_raw && unshift_in){
        // using I = std::decay_t<I_raw>;
        // static_assert(type_to_bits_v<T> <= bits_len, "input type is longer than bitfield can present");
        apply_unshift_to_data(value_, unshift_in);
        return *this;
    }

    __inline constexpr 
    T as_val() const{
        return (value_ & mask) >> (b_bits);
    }
};

template<typename D>

struct BitFieldDyn{
private:    
    using T = std::decay_t<D>;
    D & data_;
    const size_t b_bits_;
    const T mask_;

public:
    constexpr 
    explicit BitFieldDyn(D & data, const size_t b_bits, const size_t e_bits):
        data_(data), b_bits_(b_bits), mask_(mask_of<T>(b_bits, e_bits)){;}

    template<typename I_raw>
    __inline constexpr 
    auto & operator =(I_raw && unshift_in){
        // apply_unshift_to_data(value_, unshift_in);
        data_ = (unshift_in << b_bits_) | (data_ & ~mask_);
        return *this;
    }

    __inline constexpr 
    T as_val() const{
        return (data_ & mask_) >> (b_bits_);
    }
};




template<auto ... Args>
using BitField_u16 = BitField<const uint16_t, Args...>;

template<auto ... Args>
using BitField_mu16 = BitField<uint16_t, Args...>;


struct RegBase{};


struct Reg16:public RegBase{
    using data_type = uint16_t;
};

template<typename T>
struct _reg_data_type{};

template<typename T>
requires std::is_integral_v<T>
struct _reg_data_type<T>{using type = T;};

template<typename T>
requires std::is_base_of_v<RegBase, T>
struct _reg_data_type<T>{using type = T::data_type;};

template<typename T>
using reg_data_type_t = typename _reg_data_type<T>::type;

static_assert(std::is_same_v<uint16_t, reg_data_type_t<uint16_t>>, "reg_data_type_t is not uint16_t");

// template<size_t b_bits, size_t e_bits, typename T>
// __inline static constexpr
// auto make_mut_bitfield(T * obj){
//     // using T = std::remove_pointer_t<decltype(obj)>;
//     using D = typename std::remove_const_t<reg_data_type_t<T>>;
    
//     return BitField<D, b_bits, e_bits>(*reinterpret_cast<D *>(obj));
// }

template<size_t b_bits, size_t e_bits, typename D>
__inline static constexpr
auto make_bitfield(D & data){
    // using T = std::remove_pointer_t<decltype(obj)>;
    // using D = typename std::add_const_t<reg_data_type_t<T>>;
    
    return BitField<D, b_bits, e_bits>(data);
}



template<typename T>
struct BitFielfArray{};

template<typename T, size_t b_bits, size_t e_bits, size_t cnt>
struct BitFieldArray{
private:    
    // using T = std::decay_t<D>;
    static constexpr size_t data_len = type_to_bits_v<T>;
    static constexpr size_t len = e_bits - b_bits;
    static constexpr size_t per_len = (len / cnt);
    // static constexpr size_t len = ()

    static_assert(b_bits + len <= data_len, "bitfield array is longer than reg data");
    static_assert(per_len * cnt == len, "bitfield array is not aligned with reg data");

    T & data_;
public:
    constexpr 
    explicit BitFieldArray(T & data):data_(data){;}

    __inline constexpr 
    BitFieldDyn<T> operator [](const size_t idx) const {
        return BitFieldDyn<T>(data_, b_bits + per_len * idx, b_bits + per_len * (idx + 1));
    }

    template<size_t idx>
    __inline constexpr 
    auto get() const {
        static_assert(idx < cnt, "index out of range");
        return BitField<T, b_bits + per_len * idx, b_bits + per_len * (idx + 1)>(data_);
    }
};



template<size_t b_bits, size_t per_len, size_t cnt, typename D>
__inline static constexpr
BitFieldArray<D, b_bits, per_len, cnt> make_bfarray(D & data){
    return BitFieldArray<D, b_bits, per_len, cnt>(data);
}

// template<size_t b_bits, size_t e_bits>
// __inline static constexpr
// auto make_bitfield(auto * obj){
//     using T = std::remove_pointer_t<decltype(obj)>;
//     using reg_data_type = std::conditional_t<
//         std::is_const_v<T>,
//         const reg_data_type_t<T>,
//         reg_data_type_t<T>
//     >;
    
//     return BitField<reg_data_type, b_bits, e_bits>(reinterpret_cast<reg_data_type &>(*obj));
// }

struct R16_Temp:public Reg16{
    
    uint16_t data;
    
    constexpr R16_Temp(uint16_t val) : data(val) {} 
    __inline constexpr
    auto data1() const { return make_bitfield<8, 10>(data); };

    __inline constexpr
    auto arr1() const { return make_bfarray<0,16,4>(data); };
};

void test() {

    static_assert(mask_of<uint16_t>(8, 16) == 0xFF00);

    {
        static constexpr uint16_t temp = 0xFF00;
        static constexpr BitField_u16<8, 10> bf1(temp);

        static_assert(bf1.bits_len == 2, "bitfield length error");
        static_assert(bf1.as_val() == 0b11, "bitfield data error");

    }
    
    {
        static constexpr R16_Temp temp = R16_Temp{0x1234};
        static constexpr auto bf1 = temp.data1();

        static_assert(bf1.bits_len == 2, "bitfield length error");
        // static_assert(bf1.as_val() == 0b01, "bitfield data error");
        static_assert(temp.arr1().get<0>().as_val() == 0x04, "bitfield data error");
        static_assert(temp.arr1().get<1>().as_val() == 0x03, "bitfield data error");
        static_assert(temp.arr1().get<2>().as_val() == 0x02, "bitfield data error");
        static_assert(temp.arr1().get<3>().as_val() == 0x01, "bitfield data error");

        static_assert(temp.arr1()[0].as_val() == 0x04, "bitfield data error");
        static_assert(temp.arr1()[1].as_val() == 0x03, "bitfield data error");
        static_assert(temp.arr1()[2].as_val() == 0x02, "bitfield data error");
        static_assert(temp.arr1()[3].as_val() == 0x01, "bitfield data error");
    }

    {
        R16_Temp temp = R16_Temp{0x1234};
        [[maybe_unused]]
        auto bf1 = temp.data1();

        // static_assert(bf1.bits_len == 2, "bitfield length error");
        // static_assert(bf1.as_val() == 0b01, "bitfield data error");
    }
}

