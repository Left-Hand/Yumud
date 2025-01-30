// #pragma once

// class Endpoint
// {
// public:
//     //const char* const name_;
//     virtual void handle(const uint8_t *input, size_t input_length, StreamSink *output) = 0;

//     virtual bool get_string(char *output, size_t length)
//     { return false; }

//     virtual bool set_string(char *buffer, size_t length)
//     { return false; }

//     virtual bool set_from_float(float value)
//     { return false; }
// };



// template<typename TObj, typename ... TInputsAndOutputs>
// class ProtocolFunction;

// template<typename TObj, typename ... TInputs, typename ... TOutputs>
// class ProtocolFunction<TObj, std::tuple<TInputs...>, std::tuple<TOutputs...>> : Endpoint
// {
// public:
//     // @brief The return type of the function as written by a C++ programmer
//     using TRet = typename return_type<TOutputs...>::type;

//     static constexpr size_t endpoint_count = 1 + MemberList<ProtocolProperty<TInputs>...>::endpoint_count +
//                                              MemberList<ProtocolProperty<TOutputs>...>::endpoint_count;

//     ProtocolFunction(const char *name, TObj &obj, TRet(TObj::*func_ptr)(TInputs...),
//                      std::array<const char *, sizeof...(TInputs)> input_names,
//                      std::array<const char *, sizeof...(TOutputs)> output_names) :
//             name_(name), obj_(&obj), func_ptr_(func_ptr),
//             input_names_{input_names}, output_names_{output_names},
//             input_properties_(PropertyListFactory<TInputs...>::template make_property_list<0>(input_names_, in_args_)),
//             output_properties_(
//                     PropertyListFactory<TOutputs...>::template make_property_list<0>(output_names_, out_args_))
//     {
//         LOG_FIBRE("my tuple is at %x and of size %u\r\n", (uintptr_t) &in_args_, sizeof(in_args_));
//     }

//     // The custom copy constructor is needed because otherwise the
//     // input_properties_ and output_properties_ would point to memory
//     // locations of the old object.
//     ProtocolFunction(const ProtocolFunction &other) :
//             name_(other.name_), obj_(other.obj_), func_ptr_(other.func_ptr_),
//             input_names_{other.input_names_}, output_names_{other.output_names_},
//             input_properties_(PropertyListFactory<TInputs...>::template make_property_list<0>(input_names_, in_args_)),
//             output_properties_(
//                     PropertyListFactory<TOutputs...>::template make_property_list<0>(output_names_, out_args_))
//     {
//         LOG_FIBRE("COPIED! my tuple is at %x and of size %u\r\n", (uintptr_t) &in_args_, sizeof(in_args_));
//     }

//     void write_json(size_t id, StreamSink *output)
//     {
//         // write name
//         write_string("{\"name\":\"", output);
//         write_string(name_, output);

//         // write endpoint ID
//         write_string("\",\"id\":", output);
//         char id_buf[10];
//         snprintf(id_buf, sizeof(id_buf), "%u", (unsigned) id); // TODO: get rid of printf
//         write_string(id_buf, output);

//         // write arguments
//         write_string(",\"type\":\"function\",\"inputs\":[", output);
//         input_properties_.write_json(id + 1, output),
//                 write_string("],\"outputs\":[", output);
//         output_properties_.write_json(id + 1 + decltype(input_properties_)::endpoint_count, output),
//                 write_string("]}", output);
//     }

//     // special-purpose function - to be moved
//     Endpoint *get_by_name(const char *name, size_t length)
//     {
//         return nullptr; // can't address functions by name
//     }

//     void register_endpoints(Endpoint **list, size_t id, size_t length)
//     {
//         if (id < length)
//             list[id] = this;
//         input_properties_.register_endpoints(list, id + 1, length);
//         output_properties_.register_endpoints(list, id + 1 + decltype(input_properties_)::endpoint_count, length);
//     }

//     template<size_t i = sizeof...(TOutputs)>
//     std::enable_if_t<i == 0>
//     handle_ex()
//     {
//         invoke_function_with_tuple(*obj_, func_ptr_, in_args_);
//     }

//     template<size_t i = sizeof...(TOutputs)>
//     std::enable_if_t<i == 1>
//     handle_ex()
//     {
//         std::get<0>(out_args_) = invoke_function_with_tuple(*obj_, func_ptr_, in_args_);
//     }

//     template<size_t i = sizeof...(TOutputs)>
//     std::enable_if_t<i >= 2>
//     handle_ex()
//     {
//         out_args_ = invoke_function_with_tuple(*obj_, func_ptr_, in_args_);
//     }

//     void handle(const uint8_t *input, size_t input_length, StreamSink *output) final
//     {
//         (void) input;
//         (void) input_length;
//         (void) output;
//         LOG_FIBRE("tuple still at %x and of size %u\r\n", (uintptr_t) &in_args_, sizeof(in_args_));
//         LOG_FIBRE("invoke function using %d and %.3f\r\n", std::get<0>(in_args_), std::get<1>(in_args_));
//         handle_ex();
//     }

//     const char *name_;
//     TObj *obj_;

//     TRet (TObj::*func_ptr_)(TInputs...);

//     std::array<const char *, sizeof...(TInputs)> input_names_; // TODO: remove
//     std::array<const char *, sizeof...(TOutputs)> output_names_; // TODO: remove
//     std::tuple<TInputs...> in_args_;
//     std::tuple<TOutputs...> out_args_;
//     MemberList<ProtocolProperty<TInputs>...> input_properties_;
//     MemberList<ProtocolProperty<TOutputs>...> output_properties_;
// };