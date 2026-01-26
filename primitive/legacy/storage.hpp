// class BlockDeviceOperation{
// public:
//     struct Write{
//         Address addr;
//         AddressDiff length;
//         const uint8_t * pbuf;
//     };

//     struct Read{
//         Address addr;
//         AddressDiff length;
//         uint8_t * pbuf;
//     };

//     struct Erase{
//         Address addr;
//         AddressDiff length;
//     };

//     enum class Kind{
//         Write,
//         Read,
//         Erase
//     };

//     // using enum Kind;

//     // template<typename T>
//     BlockDeviceOperation(Write oper):count_(oper){;}
//     BlockDeviceOperation(Read oper):count_(oper){;}
//     BlockDeviceOperation(Erase oper):count_(oper){;}


//     template<typename T>
//     T as(){
//         if(std::holds_alternative<T>(count_))
//             return std::get<T>(count_);
//         else __builtin_unreachable();
//     }

//     Kind kind() const{
//         if(std::holds_alternative<Write>(count_))
//             return Kind::Write;
//         else if(std::holds_alternative<Read>(count_))
//             return Kind::Read;
//         else if(std::holds_alternative<Erase>(count_))
//             return Kind::Erase;
//         else __builtin_unreachable();
//     }
// private:
//     std::variant<Write, Read, Erase> count_;
// };

// class BlockDeviceAsyncTask;

// class BlockDeviceIntf{
// public:
//     enum class Error_Kind{
//         IsOperatingByOther,
//         PayloadOverlength
//     };

//     DEF_ERROR_SUMWITH_HALERROR(Error,Error_Kind)

//     using Operation = BlockDeviceOperation;

    
//     template<typename T = void>
//     using IResult = Result<T,Error>;
// public:

//     virtual ~BlockDeviceIntf() = 0;
    
//     // virtual bool is_busy() = 0;

//     virtual bool is_available() = 0;

//     virtual IResult<uint32_t> resume() = 0;

// protected:
//     virtual IResult<> push_operation(const Operation op);
//     friend class BlockDeviceAsyncTask;
// };

// class BlockDeviceAsyncTask final{
// public:
//     BlockDeviceAsyncTask(
//         BlockDeviceIntf & device, 
//         BlockDeviceOperation op
//     ) : device_(device), op_(op){;}

//     auto & device() { return device_; }
//     const auto & operation() const { return op_; }

//     using DeviceError = BlockDeviceIntf::Error_Kind;
//     Result<uint32_t, BlockDeviceIntf::Error> resume(){
//         switch(progress_){
//             case Progress::Pending:{
//                 if(device_.is_available() == false) break;
//                 const auto res = device_.push_operation(op_);
//                 if(res.is_err()){
//                     progress_ = Progress::Failed;
//                     return Err(res.unwrap_err());
//                 }else{
//                     progress_ = Progress::InProgress;
//                     return device_.resume();
//                 }
//             }break;
//             case Progress::InProgress:{
//                 const auto res = device_.resume();
//                 if(res.is_err()) {
//                     progress_ = Progress::Failed;
//                     err_ = res.err();
//                     return Err(res.unwrap_err());
//                 }
//                 if(const auto remain = res.unwrap(); remain == 0)
//                     progress_ = Progress::Completed;
//             }break;
//             case Progress::Completed: { 
//                 return Ok(0u);
//             }break;
//             case Progress::Failed: { 
//                 return Err(err_.unwrap());
//             }break;
//             default: __builtin_unreachable();
//         }
//     }
// private:
//     BlockDeviceIntf & device_;
//     BlockDeviceOperation op_;
//     // Result<Capacity, BlockDeviceIntf::Error> result_;
//     Option<BlockDeviceIntf::Error> err_ = None;
//     enum class Progress{
//         Pending,
//         InProgress,
//         Completed,
//         Failed
//     };

//     Progress progress_ = Progress::Pending;
// };



// class StorageIntf:
//     public BlockDeviceIntf{
// public:
//     virtual AddressDiff capacity() = 0;
//     // virtual void store_bytes(const Address loc, const std::span<const uint8_t> pbuf) final{
//     //     entry_oper(Operation::Store);
//     //     store_bytes_impl(loc, pbuf);
//     //     exit_oper(Operation::Store);
//     // }

//     // virtual void load_bytes(const Address loc, const std::span<uint8_t> pbuf) final{
//     //     entry_oper(Operation::Load);
//     //     load_bytes_impl(loc, pbuf);
//     //     exit_oper(Operation::Load);
//     // }

//     // virtual void erase_bytes(const Address loc, const AddressDiff len) final{
//     //     entry_oper(Operation::Erase);
//     //     erase_bytes_impl(loc, len);
//     //     exit_oper(Operation::Erase);
//     // };
// protected:
//     // virtual push_operation(const Operation op);
//     // virtual void entry_oper(const Operation op) = 0;
//     // virtual void exit_oper(const Operation op) = 0;

//     // virtual void store_bytes_impl(const Address loc, const std::span<const uint8_t> pbuf) = 0;

//     // virtual void load_bytes_impl(const Address loc, const std::span<uint8_t> pbuf) = 0;

//     // virtual void erase_bytes_impl(const Address loc, const AddressDiff length) = 0;
// };
