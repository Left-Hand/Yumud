#include "CanFilter.hpp"

#include "sys/core/platform.h"
#include "sys/string/StringView.hpp"

using namespace ymd;
using namespace ymd::hal;
using RemoteType = CanUtils::RemoteType; 

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
                T(result.id, RemoteType::Any), 
                T(result.mask, RemoteType::Any)
            };
        case remote_selection::data:
            return {
                T(result.id, RemoteType::Data), 
                T(result.mask, RemoteType::Specified)
            };
        case remote_selection::remote:
            return {
                T(result.id, RemoteType::Remote), 
                T(result.mask, RemoteType::Specified)
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

void CanFilter::list(const std::initializer_list<CanID16> & list){
    switch(list.size()){
        default:
            HALT;
        case 2:
            id16[0] =       *std::next(list.begin(), 0);
            id16[1] =       *std::next(list.begin(), 1);
            mask16[0] =     *std::next(list.begin(), 2);
            mask16[1] =     *std::next(list.begin(), 3);

            break;
    }
    is32 = false;
    islist = true;

    apply();
}

void CanFilter::all(){
    mask(
        CanID16::ACCEPT_ALL(), CanID16::ACCEPT_ALL(),
        CanID16::ACCEPT_ALL(), CanID16::ACCEPT_ALL());
}

void CanFilter::list(const std::initializer_list<CanID32> & list){
    switch(list.size()){
        default:
            HALT;
        case 2:
            id32 =      *list.begin();
            mask32 =    *std::next(list.begin());
            break;
    }
    is32 = true;
    islist = true;

    apply();
}

void CanFilter::mask(const CanID16 & id1, const CanID16 & mask1, const CanID16 & id2, const CanID16 & mask2){
    id16[0] = id1;
    id16[1] = id2;
    mask16[0] = mask1;
    mask16[1] = mask2;

    is32 = false;
    islist = false;

    apply();
}

void CanFilter::mask(const CanID32 & id, const CanID32 & mask){
    id32 = id;
    mask32 = mask;

    is32 = true;
    islist = false;

    apply();
}


void CanFilter::apply(){

    CAN_FilterInitTypeDef CAN_FilterInitSturcture = {
        .CAN_FilterIdHigh = id16[1],
        .CAN_FilterIdLow = id16[0],
        .CAN_FilterMaskIdHigh = mask16[1],
        .CAN_FilterMaskIdLow = mask16[0],
        .CAN_FilterFIFOAssignment = CAN_FIFO0,
        .CAN_FilterNumber = idx,
        .CAN_FilterMode = islist ? CAN_FilterMode_IdList : CAN_FilterMode_IdMask,
        .CAN_FilterScale = is32 ? CAN_FilterScale_32bit : CAN_FilterScale_16bit,
        .CAN_FilterActivation = ENABLE,
    };

    CAN_FilterInit(&CAN_FilterInitSturcture);

    CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_Filter_FIFO1;

    //register fifo1
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
                auto && [id, mask] = id_and_mask<CanID16>(result);
                self.mask(id, mask);
            }
            break;
        case 29:{
                auto && [id, mask] = id_and_mask<CanID32>(result);
                self.mask(id, mask);
            }
            break;
        default:
            return false;
    }
    return true;
}