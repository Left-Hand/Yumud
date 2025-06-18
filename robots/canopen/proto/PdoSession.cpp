#include "PdoSession.hpp"
#include "PdoProtocol.hpp"


using namespace ymd::canopen;




//从当前pdo的参数构造一个报文
CanMsg PdoTxSession::buildMessage() const {
    //mapping条目0项对应映射项数目
    //mapping条目s剩余子项按照Packet映射

    std::array<uint8_t, 8> data;

    const int cnt = int(mapping_[0].unwrap());
    const CobId cobId = CobId::from_u16(int(params_[1].unwrap()));

    int bits_cnt = 0;

    for (int i = 1; i <= cnt; i++) {

        //获取mapping条目i的子项 得到映射
        const int map = int(mapping_[i].unwrap());

        //拆分为条目的比特数 索引
        const auto [bits, subindex, index] = PdoMapping::from_u32(map);

        //获取映射项的值
        const auto val = int(pdo_.get_sub_entry(index, subindex).unwrap());

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

    return CanMsg::from_bytes(
        cobId.to_stdid(), 
        std::span(data)
    );
}

//将收到的pdo报文写入字典
bool PdoRxSession::processMessage(const CanMsg& msg){
    const CobId cobId = CobId::from_u16(int(params_[1].unwrap()));

    if (cobId.to_stdid() != msg.id()) {
        return false;
    }

    int cnt = int(mapping_[0].unwrap());
    int bits_cnt = 0;

    for (int i = 1; i <= cnt; i++) {

        const int map = int(mapping_[i].unwrap());

        //拆分为条目的比特数 索引
        const auto [bits, subindex, index] = PdoMapping::from_u32(map);
    
        //获取映射项
        SubEntry se = pdo_.get_sub_entry(index, subindex).unwrap();

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
    TransferType transType = TransferType(int(params_[2].unwrap()));
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