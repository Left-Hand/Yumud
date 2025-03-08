
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





// template<typename E, auto...args>
// struct __static_string_pool{
//     using Key = E;
//     using Value = const char *;

//     using Pair = std::pair<Key, Value>;

//     constexpr size_t N = sizeof...(args); 
//     static constexpr std::array<Pair> pool={static_cast<const char *>(args)...};
    
//     constexpr Value
//     operator [](const size_t idx) const{return pool[idx];}
// };


// template <typename E, size_t N = enum_count_v<E>>
// consteval auto make_string_pool(){

// }
// template <E V>
// using enum_type_t = typename E;

// template<typename T>
// struct _tuple_bytes{};

// template<typename First>
// struct _tuple_bytes<std::tuple<First>>{
//     static constexpr size_t value = sizeof(First);
// };

// template<typename First, typename ... Rest>
// struct _tuple_bytes<std::tuple<First, Rest...>>{
//     static constexpr size_t value = 
//         sizeof(First) + _tuple_bytes<std::tuple<Rest...>>::value;
// };

// template<typename ... Args>
// static constexpr size_t tuple_bytes_v = _tuple_bytes<std::tuple<Args...>>::value;
