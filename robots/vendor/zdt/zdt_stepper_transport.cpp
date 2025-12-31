#include "zdt_stepper_transport.hpp"

#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"


using namespace ymd;
namespace ymd::robots::zdtmotor{


struct [[nodiscard]] Bytes2CanFrameIterator{
    static constexpr size_t MAX_PAYLOAD_LENGTH_PER_CAN_FRAME = 7;
    explicit constexpr Bytes2CanFrameIterator(
        const NodeId node_id, 
        const FuncCode func_code,
        const std::span<const uint8_t> tailed_context_bytes
    ):
        node_id_(node_id),
        func_code_(func_code),
        tailed_context_bytes_(tailed_context_bytes){;}

    explicit constexpr Bytes2CanFrameIterator(
        const FlatPacket & flat_packet
    ):
        Bytes2CanFrameIterator(
            flat_packet.node_id,
            flat_packet.func_code,
            flat_packet.tailed_context_bytes()
        ){;}

    [[nodiscard]] constexpr bool has_next() const {
        return tailed_context_bytes_.size() > offset_;
    }
    [[nodiscard]] constexpr hal::BxCanFrame next(){

        const auto frame_len = MIN(
            tailed_context_bytes_.size() - offset_, 
            MAX_PAYLOAD_LENGTH_PER_CAN_FRAME
        );

        const auto frame = make_canmsg(
            node_id_, func_code_, 
            offset_ / MAX_PAYLOAD_LENGTH_PER_CAN_FRAME,
            tailed_context_bytes_.subspan(offset_, frame_len)
        ); 

        offset_ += frame_len;

        return frame;
    }
private:
    static constexpr hal::BxCanFrame make_canmsg(
        const NodeId node_id,
        const FuncCode func_code,
        const uint8_t piece_cnt,
        const std::span<const uint8_t> bytes
    ){
        constexpr size_t CAN_MAX_PAYLOAD_SIZE = 8;
        auto buf = HeaplessVector<uint8_t, CAN_MAX_PAYLOAD_SIZE>{};
        buf.append_unchecked(std::bit_cast<uint8_t>(func_code));
        buf.append_unchecked(bytes);

        return hal::BxCanFrame(
            nodeid_and_piececnt_to_canid(node_id, piece_cnt),
            hal::BxCanPayload::from_bytes(buf.as_slice())
        );
    }

    //固定为拓展帧
    static constexpr hal::CanExtId nodeid_and_piececnt_to_canid(
        const NodeId node_id, 
        const uint8_t piece
    ){
        return hal::CanExtId::from_bits(
            uint32_t(node_id.to_u8() << 8) | 
            (piece)
        );
    }

private:
    NodeId node_id_;
    FuncCode func_code_;
    std::span<const uint8_t> tailed_context_bytes_;
    size_t offset_ = 0;
};


struct [[nodiscard]] CanFrame2BytesDumper{
    static constexpr IResult<FlatPacket> dump(
        std::span<const hal::BxCanFrame> frames
    ) {
        if(frames.size() == 0)
            return Err(Error::RxNoMsgToDump);
        
        FlatPacket flat_packet;

        flat_packet.node_id = ({
            if(frames[0].is_standard())
                return Err(Error::RxMsgIdTypeNotMatch);

            const auto id = frame_to_nodeid(frames[0]);

            for(size_t i = 1; i < frames.size(); i++){
                if(frames[i].is_standard())
                    return Err(Error::RxMsgIdTypeNotMatch);
                const auto next_id = frame_to_nodeid(frames[i]);
                if(next_id != id) return Err(Error::RxMsgNodeIdNotTheSame);
            }
            NodeId::from_u8(id);
        });

        for(size_t i = 0; i < frames.size(); i++){
            const auto piece_cnt = frame_to_piececnt(frames[i]);
            if(i != piece_cnt) 
                return Err(Error::RxMsgPieceIsNotSteady);
        }

        flat_packet.func_code = ({
            if(frames[0].length() == 0) return Err(Error::RxMsgNoPayload);
            const auto func_code0 = std::bit_cast<FuncCode>(frames[0].payload_bytes()[0]);
            for(size_t i = 0; i < frames.size(); i++){
                const auto & frame = frames[i];
                if(frame.length() == 0) return Err(Error::RxMsgNoPayload);
                const auto func_code = std::bit_cast<FuncCode>(frame.payload_bytes()[0]);
                if(func_code != func_code0)
                    return Err(Error::RxMsgFuncCodeNotTheSame);
            }
            func_code0;
        });

        BytesFiller filler(std::span(flat_packet.context));
        for(size_t i = 0; i < frames.size(); i++){
            const auto & frame = frames[i];
            filler.push_bytes(frame.payload_bytes().subspan(1));
        }
        flat_packet.payload_len = (filler.size() - 1);
        
        //TODO add tail verify

        return Ok<FlatPacket>(flat_packet);
    }

    static inline constexpr uint8_t frame_to_nodeid(
        const hal::BxCanFrame & frame
    ){
        return frame.identifier().to_extid().to_u29() >> 8;
    }

    static inline constexpr uint8_t frame_to_piececnt(
        const hal::BxCanFrame & frame
    ){
        return frame.identifier().to_extid().to_u29() & 0xff;
    }

};


void ZdtMotorTransport::can_write_flat_packet(
    hal::Can & can, 
    const FlatPacket & flat_packet
){
    auto iter = Bytes2CanFrameIterator(
        flat_packet.node_id, 
        flat_packet.func_code, 
        flat_packet.payload_bytes()
    );
    while(iter.has_next()){
        const auto frame = iter.next();
        can.try_write(frame).examine();
    }
}

void ZdtMotorTransport::uart_write_flat_packet(
    hal::Uart & uart, 
    const FlatPacket & flat_packet
){
    const auto bytes = flat_packet.headed_bytes();

    (void)uart.try_write_chars(
        reinterpret_cast<const char *>(bytes.data()),
        bytes.size()
    );
}

void ZdtMotorTransport::write_flat_packet(
    const FlatPacket & flat_packet
){
    if(may_uart_.is_some()){
        uart_write_flat_packet(
            may_uart_.unwrap(), 
            flat_packet
        );
    }else if(may_can_.is_some()){
        can_write_flat_packet(
            may_can_.unwrap(), 
            flat_packet
        );
    }else{
        PANIC{"no Transport available"};
    }
}

}