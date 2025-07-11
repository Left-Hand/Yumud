#include "can_filter.hpp"

#include "core/platform.hpp"
#include "core/sdk.hpp"
#include "core/string/string_view.hpp"

using namespace ymd;
using namespace ymd::hal;

using ID16 = CanStdIdMask;
using ID32 = CanExtIdMask;
struct bits_queue{
protected:
    uint32_t code_ = 0;
    size_t cnt_ = 0;
public: 
    void push_back(bool bit){
        if(bit )code_ |= (1 << cnt_);
        cnt_++;
    }

    uint32_t code() const{
        return code_;
    }

    size_t size() const{
        return cnt_;
    }

};


enum class remote_selection:uint8_t {
    any,
    data,
    remote
};

struct code_parser{
public:
    struct parse_result{
        #pragma pack(push,1)
        uint32_t id;
        uint32_t mask;
        size_t size;
        bool is_ext;
        remote_selection rm_sel;
        bool valid;
        #pragma pack(pop)
    };
protected:
    bits_queue id_queue;
    bits_queue mask_queue;
    bool is_ext = false;
    remote_selection rm_sel = remote_selection::any;


    void push_bit(bool bit){
        id_queue.push_back(bit);
        mask_queue.push_back(1);
    }

    void push_x(){
        id_queue.push_back(0);
        mask_queue.push_back(0);
    }

public: 
    void push(const char chr){
        auto & self = *this;
        switch(tolower(chr)){
            case 'r':
                self.rm_sel = remote_selection::remote;
                break;
            case 'e':
                self.is_ext = true;
                break;
            case 'd':
                self.rm_sel = remote_selection::data;
            case '_':
                break;
            case '0':
                self.push_bit(0);
                break;
            case '1':   
                self.push_bit(1);
                break;
            case 'x':
                self.push_x();
                break;
            default:
                break;
        }
    }

    uint32_t code() const{
        return id_queue.code();
    }

    uint32_t mask() const{
        return mask_queue.code();
    }

    size_t size() const{
        return id_queue.size();
    }

    auto result() const{
        auto & self = *this;
        const auto id = self.id_queue.code();
        const auto mask = self.mask_queue.code();
        const auto size = self.id_queue.size();

        return parse_result{
            .id = id,
            .mask = mask,
            .size = size,
            .is_ext = self.is_ext,
            .rm_sel = self.rm_sel,

            .valid = (
                ((size == 11) and (is_ext == false)) or 
                ((size == 29) and (is_ext == true))
            )
        };
    }
};


template<typename T>
std::tuple<T, T> id_and_mask(const code_parser::parse_result & result){

    switch(result.rm_sel){
        default:
        case remote_selection::any:
            return {
                T(result.id, CanRemoteSpec::Any), 
                T(result.mask, CanRemoteSpec::Any)
            };
        case remote_selection::data:
            return {
                T(result.id, CanRemoteSpec::Data), 
                T(result.mask, CanRemoteSpec::Specified)
            };
        case remote_selection::remote:
            return {
                T(result.id, CanRemoteSpec::Remote), 
                T(result.mask, CanRemoteSpec::Specified)
            };
    }
};


static auto parse_str(const StringView & str){
    // parse_result result;
    code_parser parser;
    // sstl::vector<bool, 29> bits;

    for(char chr : str){
        parser.push(chr);
    }

    return parser.result();
}

void CanFilter::list(const std::initializer_list<ID16> & list){
    switch(list.size()){
        default:
            HALT;
            break;
        case 2:
            id16[0] =       std::next(list.begin(), 0) -> to_u16();
            id16[1] =       std::next(list.begin(), 1) -> to_u16();
            mask16[0] =     std::next(list.begin(), 2) -> to_u16();
            mask16[1] =     std::next(list.begin(), 3) -> to_u16();

            break;
    }
    is32_ = false;
    islist_ = true;

    apply();
}

void CanFilter::all(){
    mask(
        {ID16::ACCEPT_ALL(), ID16::ACCEPT_ALL()},
        {ID16::ACCEPT_ALL(), ID16::ACCEPT_ALL()});
}

void CanFilter::list(const std::initializer_list<ID32> & list){
    switch(list.size()){
        default:
            HALT;
            [[fallthrough]];

        case 2:
            id32 =      list.begin() -> to_u32();
            mask32 =    std::next(list.begin()) -> to_u32();
            break;
    }
    is32_ = true;
    islist_ = true;

    apply();
}

void CanFilter::mask(const StdIdMaskPair & pair1, const StdIdMaskPair & pair2){
    id16[0] = pair1.id.to_u16();
    id16[1] = pair2.id.to_u16();
    mask16[0] = pair1.mask.to_u16();
    mask16[1] = pair2.mask.to_u16();

    is32_ = false;
    islist_ = false;

    apply();
}

void CanFilter::mask(const ID32 & id, const ID32 & mask){
    id32 = id.to_u32();
    mask32 = mask.to_u32();

    is32_ = true;
    islist_ = false;

    apply();
}


void CanFilter::apply(){

    CAN_FilterInitTypeDef CAN_FilterInitSturcture = {
        .CAN_FilterIdHigh = id16[1],
        .CAN_FilterIdLow = id16[0],
        .CAN_FilterMaskIdHigh = mask16[1],
        .CAN_FilterMaskIdLow = mask16[0],
        .CAN_FilterFIFOAssignment = CAN_FIFO0,
        .CAN_FilterNumber = idx_,
        .CAN_FilterMode = islist_ ? CAN_FilterMode_IdList : CAN_FilterMode_IdMask,
        .CAN_FilterScale = is32_ ? CAN_FilterScale_32bit : CAN_FilterScale_16bit,
        .CAN_FilterActivation = ENABLE,
    };

    CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    CAN_FilterInit(&CAN_FilterInitSturcture);

    CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_Filter_FIFO1;
    CAN_FilterInit(&CAN_FilterInitSturcture);

}

void CanFilter::deinit(){
    // HALT;
    // TODO();
}



bool CanFilter::bystr(const StringView & str){
    auto result = parse_str(str);

    if(result.valid == false){
        return false;
    }

    auto & self = *this;

    switch(result.size){
        case 11:{
                auto && [id, mask] = id_and_mask<ID16>(result);
                self.mask({id, mask});
            }
            break;
        case 29:{
                auto && [id, mask] = id_and_mask<ID32>(result);
                self.mask({id, mask});
            }
            break;
        default:
            return false;
    }
    return true;
}