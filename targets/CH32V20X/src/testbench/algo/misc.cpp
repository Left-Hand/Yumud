
// // Helper to extract bitfield values
// template <typename T, typename... Members>
// constexpr auto extractBitFields(const T& obj, Members... members) {
//     return std::make_tuple((obj.*members)...);
// }

// // Helper to combine bitfield values into an unsigned integer
// template <typename Tuple, std::size_t... Is>
// constexpr auto combineBitFieldsImpl(const Tuple& tuple, std::index_sequence<Is...>) {
//     return (static_cast<std::make_unsigned_t<decltype(std::get<Is>(tuple))>>(std::get<Is>(tuple)) << (Is * 8)) | ...;
// }

// template <typename... Members>
// constexpr auto combineBitFields(const Members&... members) {
//     return combineBitFieldsImpl(extractBitFields(members...), std::index_sequence_for<Members...>{});
// }

// CRTP base class for bitfield extraction
// struct Dummy{

// };

// template <typename T>

// struct RegCRTP {
//     static constexpr auto maskof(const T obj){
//         // static_assert(sizeof(T) == Bytes);
//         // static_assert(std::has_unique_object_representations_v<T>);
//         using IntType = typename size_to_uint<sizeof(T)>::type;
//         // return reinterpret_cast<IntType*>(&obj);
//         return std::bit_cast<IntType>(obj);
//     }
// };

// Example struct with bitfields
// // struct MyReg : public RegCRTP<MyReg>, public Dummy{
// struct MyReg : public RegCRTP<MyReg>{
//     uint16_t a : 1;
//     uint16_t b : 1;
//     // uint16_t : 6;  // Padding bits
//     uint16_t c : 1;
//     uint16_t __resv__:13;
// };

// constexpr auto size = sizeof(MyReg);

// enum class Mask:uint16_t{
//     A = MyReg::maskof({.a = 0, .b = 0, .c = 1}),
//     B = 1
// };

// #define scinit static constinit;