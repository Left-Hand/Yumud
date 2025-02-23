#include "PdoSession.hpp"
#include "PdoProtocol.hpp"


using namespace ymd::canopen;

struct Mapping{
    uint8_t bits;
    uint8_t subindex;
    uint8_t index;

    Mapping(const int map):
        bits(map & 0xFF),
        subindex((map >> 8) & 0xFF),
        index((map >> 16) & 0xFF){
    }
};


//从当前pdo的参数构造一个报文
CanMsg PdoTxSession::buildMessage() const {
    //mapping条目0项对应映射项数目
    //mapping条目s剩余子项按照Packet映射

    std::array<uint8_t, 8> data;

    const int cnt = int(mapping_[0].value());
    const CobId cobId = int(params_[1].value());

    int bits_cnt = 0;

    for (int i = 1; i <= cnt; i++) {

        //获取mapping条目i的子项 得到映射
        const int map = int(mapping_[i].value());

        //拆分为条目的比特数 索引
        const auto [bits, subindex, index] = Mapping(map);

        //获取映射项的值
        const auto val = int(pdo_.getSubEntry(index, subindex).value());

        //将获得的值复制到data中
        memcpy(data.begin(), &val, bits/8);
        
        bits_cnt += bits;

        if (bits_cnt > 64) {
            break;
        }
    }

    int numBytes = bits_cnt / 8;

    if ((bits_cnt % 8) != 0) {
        numBytes++;
    }

    numBytes = std::min(numBytes, 8);

    return CanMsg(cobId, data.data(), numBytes);
}

//将收到的pdo报文写入字典
bool PdoRxSession::processMessage(const CanMsg& msg){
    const CobId cobId = int(params_[1].value());

    if (msg.id() != cobId) {
        return false;
    }

    int cnt = int(mapping_[0].value());
    int bits_cnt = 0;

    for (int i = 1; i <= cnt; i++) {

        const int map = int(mapping_[i].value());

        //拆分为条目的比特数 索引
        const auto [bits, subindex, index] = Mapping(map);
    
        //获取映射项
        SubEntry& se = pdo_.getSubEntry(index, subindex).value();

        se.put(msg);

        bits_cnt += bits;

        if (bits_cnt > 64) {
            break;
        }
    }
    return true;
}

//TODO transfer type的模式匹配问题

bool PdoTxSession::onSyncEvent() {
    TransferType transType = TransferType(int(params_[2].value()));
    if (int(transType) >= int(TransferType::SyncMin) and 
        int(transType) <= int(TransferType::SyncMax)) {
        //周期性同步传输 每transType次发生同步事件才发送数据
        transSyncCount++;
        if (transSyncCount >= int(transType)){
            transSyncCount = 0;
            pdo_.sendMessage(buildMessage());
        }
    } else if (transType == TransferType::RtrSync) {
    } else if (transType == TransferType::SyncAcyclic) {
        pdo_.sendMessage(buildMessage());
    }
    return true;
}