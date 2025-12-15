/*
    nanoMODBUS - A compact MODBUS RTU/TCP C library for microcontrollers

    MIT License

    Copyright (c) 2024 Valerio De Benedetto (@debevv)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/


/** @file */

/*! \mainpage nanoMODBUS - A compact MODBUS RTU/TCP C library for microcontrollers
 * nanoMODBUS is a small C library that implements the Modbus protocol. It is especially useful in resource-constrained
 * system like microcontrollers.
 *
 * GtiHub: <a href="https://github.com/debevv/nanoMODBUS">https://github.com/debevv/nanoMODBUS</a>
 *
 * API reference: \link nanomodbus.h \endlink
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "core/utils/Result.hpp"
#include "core/utils/sumtype.hpp"
#include "core/stream/ostream.hpp"
#include "core/container/bitarray.hpp"
#include "core/container/heapless_vector.hpp"


#define NMBS_UNUSED_PARAM(x) ((x) = (x))

#ifdef NMBS_DEBUG_EN
#include <stdio.h>
#define NMBS_DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define NMBS_DEBUG_PRINT(...) (void) (0)
#endif

namespace ymd::modbus{

enum class [[nodiscard]] LibError:uint8_t{
    InvalidRequst,
    InvalidUnitId,
    InvalidTcpMbap,
    InvalidCrc,
    Transport,
    Timeout,
    InvalidResponse,
    InvalidArgument,
};

enum class [[nodiscard]] ModbusException:uint8_t{
    IllegalFunction = 1,      /**< Modbus exception 1 */
    IllegalDataAddress = 2,  /**< Modbus exception 2 */
    IllegalDataValue = 3,    /**< Modbus exception 3 */
    ServerDeviceFailure = 4, /**< Modbus exception 4 */
};

enum class [[nodiscard]] FunctionCode:uint8_t{
    None = 0,
    ReadCoils = 1,
    ReadDiscreteInputs = 2,
    ReadHoldingRegisters = 3,
    ReadInputRegisters = 4,
    WriteSingleCoil = 5,
    WriteSingleRegister = 6,
    WriteMultipleCoils = 15,
    WriteMultipleRegisters = 16,
    ReadFileRecord = 20,
    WriteFileRecord = 21,
    ReadWriteRegisters = 23,
    ReadDeviceIdentification = 43
};

struct [[nodiscard]] NmbsError:public Sumtype<LibError,ModbusException>{
    using enum LibError;
    using enum ModbusException;
    [[nodiscard]] bool is_exception() const{
        return is<ModbusException>();
    }
};

static constexpr Err<NmbsError> make_err(auto err){
    return Err<NmbsError>(err);
}

static constexpr size_t NMBS_DEVICE_IDENTIFICATION_STRING_LENGTH = 128;

#if 0
typedef enum IResult<> {
    // Library errors
    NMBS_ERROR_INVALID_REQUEST = -8,  /**< Received invalid request from client */
    NMBS_ERROR_INVALID_UNIT_ID = -7,  /**< Received invalid unit ID in response from server */
    NMBS_ERROR_INVALID_TCP_MBAP = -6, /**< Received invalid TCP MBAP */
    NMBS_ERROR_CRC = -5,              /**< Received invalid CRC */
    NMBS_ERROR_TRANSPORT = -4,        /**< Transport error */
    NMBS_ERROR_TIMEOUT = -3,          /**< Read/write timeout occurred */
    NMBS_ERROR_INVALID_RESPONSE = -2, /**< Received invalid response from server */
    NMBS_ERROR_INVALID_ARGUMENT = -1, /**< Invalid argument provided */
    NMBS_ERROR_NONE = 0,              /**< No error */

    // Modbus exceptions
    NMBS_EXCEPTION_ILLEGAL_FUNCTION = 1,      /**< Modbus exception 1 */
    NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS = 2,  /**< Modbus exception 2 */
    NMBS_EXCEPTION_ILLEGAL_DATA_VALUE = 3,    /**< Modbus exception 3 */
    NMBS_EXCEPTION_SERVER_DEVICE_FAILURE = 4, /**< Modbus exception 4 */
} IResult<>;

const char* nmbs_strerror(IResult<> error) {
    switch (error) {
        case make_err(NmbsError::InvalidRequst):
            return "invalid request received";

        case make_err(NmbsError::InvalidUnitId):
            return "invalid unit ID received";

        case make_err(NmbsError::InvalidTcpMbap):
            return "invalid TCP MBAP received";

        case NmbsError::Crc:
            return "invalid CRC received";

        case Err(NmbsError::Transport):
            return "transport error";

        case NmbsError::Timeout :
            return "timeout";

        case make_err(NmbsError::InvalidResponse):
            return "invalid response received";

        case NMBS_ERROR_INVALID_ARGUMENT:
            return "invalid argument provided";

        case NMBS_ERROR_NONE:
            return "no error";

        case ModbusException::IllegalFunction:
            return "modbus exception 1: illegal function";

        case ModbusException::IllegalDataAddress:
            return "modbus exception 2: illegal data address";

        case ModbusException::IllegalDataValue:
            return "modbus exception 3: data value";

        case ModbusException::ServerDeviceFailure:
            return "modbus exception 4: server device failure";

        default:
            return "unknown error";
    }
}

#else 
template<typename T = void>
using IResult = Result<T, NmbsError>;
#endif


/**
 * Return whether the IResult<> is a modbus exception
 * @e IResult<> to check
 */
#define nmbs_error_is_exception(e) ((e) > 0 && (e) < 5)

static constexpr size_t NMBS_BITFIELD_MAX = 2000;
static_assert((NMBS_BITFIELD_MAX % 8) == 0, "NMBS_BITFIELD_MAX must be divisible by 8");
static constexpr size_t NMBS_BITFIELD_BYTES_MAX = (NMBS_BITFIELD_MAX / 8);

/**
 * Bitfield consisting of 2000 coils/discrete inputs
 */
typedef uint8_t nmbs_bitfield[NMBS_BITFIELD_BYTES_MAX];





/**
 * Bitfield consisting of 256 values
 */
using nmbs_bitfield_256 = BitArray<256>;


/**
 * Modbus transport type.
 */
enum class NmbsTransport:uint8_t{
    RTU = 1,
    TCP = 2,
};


/**
 * nanoMODBUS platform_ configuration struct.
 * Passed to nmbs_server_create() and nmbs_client_create().
 *
 * read() and write() are the platform_-specific methods that read/write data to/from a serial port or a TCP connection.
 *
 * Both methods should block until either:
 * - `count` bytes of data are read/written
 * - the byte timeout, with `byte_timeout_ms_ >= 0`, expires
 *
 * A value `< 0` for `byte_timeout_ms_` means infinite timeout.
 * With a value `== 0` for `byte_timeout_ms_`, the method should read/write once in a non-blocking fashion and return immediately.
 *
 *
 * Their return value should be the number of bytes actually read/written, or `< 0` in case of error.
 * A return value between `0` and `count - 1` will be treated as if a timeout occurred on the transport side. All other
 * values will be treated as transport errors.
 *
 * Additionally, an optional crc_calc() function can be defined to override the default nanoMODBUS CRC calculation function.
 *
 * These methods accept a pointer to arbitrary user-data, which is the arg member of this struct.
 * After the creation of an instance it can be changed with nmbs_set_platform_arg().
 */


class NmbsPlatformIntf{
public:

    virtual size_t read(std::span<uint8_t> buf, int32_t byte_timeout_ms_) = 0;
    virtual size_t write(std::span<const uint8_t> buf, int32_t byte_timeout_ms_) = 0;
};


#if 0
/**
 * Modbus server request callback_. Passed to nmbs_server_create().
 *
 * These methods accept a pointer to arbitrary user data, which is the arg member of the NmbsPlatformIntf that was passed
 * to nmbs_server_create together with this struct.
 *
 * `unit_id` is the RTU unit ID of the request sender. It is always 0 on TCP.
 */
struct nmbs_callbacks {
#ifndef NMBS_SERVER_DISABLED
#ifndef NMBS_SERVER_READ_COILS_DISABLED
    IResult<> (*read_coils)(uint16_t address, uint16_t quantity, nmbs_bitfield coils_out, uint8_t unit_id);
#endif

#ifndef NMBS_SERVER_READ_DISCRETE_inputS_DISABLED
    IResult<> (*read_discrete_inputs)(uint16_t address, uint16_t quantity, nmbs_bitfield inputs_out, uint8_t unit_id);
#endif

#if !defined(NMBS_SERVER_READ_HOLDING_REGISTERS_DISABLED) || !defined(NMBS_SERVER_READ_WRITE_REGISTERS_DISABLED)
    IResult<> (*read_holding_registers)(uint16_t address, uint16_t quantity, uint16_t* registers_out, uint8_t unit_id);
#endif

#ifndef NMBS_SERVER_READ_input_REGISTERS_DISABLED
    IResult<> (*read_input_registers)(uint16_t address, uint16_t quantity, uint16_t* registers_out, uint8_t unit_id);
#endif

#ifndef NMBS_SERVER_WRITE_SINGLE_COIL_DISABLED
    IResult<> (*write_single_coil)(uint16_t address, bool value, uint8_t unit_id);
#endif

#ifndef NMBS_SERVER_WRITE_SINGLE_REGISTER_DISABLED
    IResult<> (*write_single_register)(uint16_t address, uint16_t value, uint8_t unit_id);
#endif

#ifndef NMBS_SERVER_WRITE_MULTIPLE_COILS_DISABLED
    IResult<> (*write_multiple_coils)(uint16_t address, uint16_t quantity, const nmbs_bitfield coils, uint8_t unit_id);
#endif

#if !defined(NMBS_SERVER_WRITE_MULTIPLE_REGISTERS_DISABLED) || !defined(NMBS_SERVER_READ_WRITE_REGISTERS_DISABLED)
    IResult<> (*write_multiple_registers)(uint16_t address, uint16_t quantity, const uint16_t* registers,
                                           uint8_t unit_id);
#endif

#ifndef NMBS_SERVER_READ_FILE_RECORD_DISABLED
    IResult<> (*read_file_record)(uint16_t file_number, uint16_t record_number, uint16_t* registers, uint16_t count,
                                   uint8_t unit_id);
#endif

#ifndef NMBS_SERVER_WRITE_FILE_RECORD_DISABLED
    IResult<> (*write_file_record)(uint16_t file_number, uint16_t record_number, const uint16_t* registers,
                                    uint16_t count, uint8_t unit_id);
#endif

#ifndef NMBS_SERVER_READ_DEVICE_IDENTIFICATION_DISABLED

    IResult<> (*read_device_identification)(uint8_t object_id, char buffer[NMBS_DEVICE_IDENTIFICATION_STRING_LENGTH]);
    IResult<> (*read_device_identification_map)(nmbs_bitfield_256 map);
#endif
#endif
};

#else
struct NmbsEvents{
    struct ReadCoils{
        static constexpr FunctionCode FUNCTION_CODE = FunctionCode::ReadCoils;
        uint16_t address;
        uint16_t quantity; 
        std::span<uint8_t, NMBS_BITFIELD_BYTES_MAX> coils_out;
        uint8_t unit_id;
    };

    struct ReadDiscreteInputs{
        static constexpr FunctionCode FUNCTION_CODE = FunctionCode::ReadDiscreteInputs;
        uint16_t address;
        uint16_t quantity; 
        std::span<uint8_t, NMBS_BITFIELD_BYTES_MAX> inputs_out;
        uint8_t unit_id;
    };

    struct ReadHoldingRegisters{
        static constexpr FunctionCode FUNCTION_CODE = FunctionCode::ReadHoldingRegisters;
        uint16_t address;
        uint16_t quantity; 
        uint16_t * registers_out;
        uint8_t unit_id;
    };

    struct ReadInputRegisters{
        static constexpr FunctionCode FUNCTION_CODE = FunctionCode::ReadInputRegisters;
        uint16_t address;
        uint16_t quantity; 
        uint16_t * registers_out;
        uint8_t unit_id;
    };

    struct WriteSingleCoil{
        static constexpr FunctionCode FUNCTION_CODE = FunctionCode::WriteSingleCoil;
        uint16_t address;
        bool value;
        uint8_t unit_id;
    };

    struct WriteSingleRegister{
        static constexpr FunctionCode FUNCTION_CODE = FunctionCode::WriteSingleRegister;
        uint16_t address;
        uint16_t value;
        uint8_t unit_id;
    };

    struct WriteMultipleCoils{
        static constexpr FunctionCode FUNCTION_CODE = FunctionCode::WriteMultipleCoils;
        uint16_t address;
        uint16_t quantity;
        std::span<const uint8_t, NMBS_BITFIELD_BYTES_MAX> coils;
        uint8_t unit_id;
    };

    struct WriteMultipleRegisters{
        static constexpr FunctionCode FUNCTION_CODE = FunctionCode::WriteMultipleRegisters;
        uint16_t address;
        uint16_t quantity;
        const uint16_t * registers;
        uint8_t unit_id;
    };

    struct ReadFileRecord{
        static constexpr FunctionCode FUNCTION_CODE = FunctionCode::ReadFileRecord;
        uint16_t file_number;
        uint16_t record_number;
        uint16_t * reg;
        uint16_t count;
        uint8_t unit_id;
    };

    struct WriteFileRecord{
        static constexpr FunctionCode FUNCTION_CODE = FunctionCode::WriteFileRecord;
        uint16_t file_number;
        uint16_t record_number;
        const uint16_t * reg;
        uint16_t count;
        uint8_t unit_id;
    };

    struct ReadDeviceIdentification{
        static constexpr FunctionCode FUNCTION_CODE = FunctionCode::ReadDeviceIdentification;
        uint8_t object_id;
        std::span<char, NMBS_DEVICE_IDENTIFICATION_STRING_LENGTH> buffer;
    };

    struct ReadDeviceIdentificationMap{
        // static constexpr FunctionCode FUNCTION_CODE = FunctionCode::ReadDeviceIdentificationMap;
        nmbs_bitfield_256 map;
    };
};

struct NmbsEvent:public std::variant<
    // std::monostate,
    NmbsEvents::ReadCoils,
    NmbsEvents::ReadDiscreteInputs,
    NmbsEvents::ReadHoldingRegisters,
    NmbsEvents::ReadInputRegisters,
    NmbsEvents::WriteSingleCoil,
    NmbsEvents::WriteSingleRegister,
    NmbsEvents::WriteMultipleCoils,
    NmbsEvents::WriteMultipleRegisters,
    NmbsEvents::ReadFileRecord,
    NmbsEvents::WriteFileRecord,
    NmbsEvents::ReadDeviceIdentification,
    NmbsEvents::ReadDeviceIdentificationMap
>
// , 
//     NmbsEvents
{
    
};


using NmbsServerCallback = std::function<IResult<>(NmbsEvent)>;

#endif




struct [[nodiscard]] RtuHeaderInfo{
    uint8_t unit_id;
    FunctionCode fc;
};

struct [[nodiscard]] TcpHeaderInfo{
    uint8_t unit_id;
    FunctionCode fc;
    uint16_t transaction_id;
};



struct [[nodiscard]] MsgStatus{
    using Self = MsgStatus;
    uint8_t unit_id;
    FunctionCode fc;
    uint16_t transaction_id;
    bool broadcast;
    bool ignored;
    bool complete;

    static constexpr Self from_default() {
        return Self{
            .unit_id = 0,
            .fc = FunctionCode::None,
            .transaction_id = 0,
            .broadcast = false,
            .ignored = false,
            .complete = false
        };
    }

    constexpr void reset(){
        unit_id = 0;
        fc = FunctionCode::None;
        transaction_id = 0;
        broadcast = false;
        ignored = false;
        complete = false;
    }

    [[nodiscard]] constexpr RtuHeaderInfo to_rtu_header_info() const{
        return RtuHeaderInfo{
            .unit_id = unit_id,
            .fc = fc,
        };
    }

    [[nodiscard]] constexpr TcpHeaderInfo to_tcp_header_info() const{
        return TcpHeaderInfo{
            .unit_id = unit_id,
            .fc = fc,
            .transaction_id = transaction_id,
        };
    }
};

struct [[nodiscard]]Buffer{

    [[nodiscard]] constexpr std::span<uint8_t> bytes(){
        return std::span<uint8_t>{buf, buf_idx};
    }


    [[nodiscard]] constexpr uint8_t pop_u8() {
        uint8_t result = buf[buf_idx];
        buf_idx++;
        return result;
    }

    void push_rtu_header(uint16_t data_length, const RtuHeaderInfo && info) {
        push_u8(info.unit_id);
        push_fc(info.fc);
    }



    void push_tcp_header(uint16_t data_length, const TcpHeaderInfo && info){
        push_u16(info.transaction_id);
        push_u16(0);
        push_u16((uint16_t) (1 + 1 + data_length));
        push_u8(info.unit_id);

        push_fc(info.fc);
    }

    constexpr void push_u8(uint8_t data) {
        buf[buf_idx] = data;
        buf_idx++;
    }

    constexpr void push_fc(const FunctionCode fc){
        push_u8(std::bit_cast<uint8_t>(fc));
    }


    constexpr void push_zero() {
        buf[buf_idx] = 0;
        buf_idx++;
    }


    constexpr void discard_n(uint16_t n) {
        buf_idx += n;
    }



    [[nodiscard]] constexpr uint16_t pop_u16() {
        const uint16_t result =
                ((uint16_t) buf[buf_idx]) << 8 | (uint16_t) buf[buf_idx + 1];
        buf_idx += 2;
        return result;
    }


    constexpr void push_u16(uint16_t data) {
        buf[buf_idx] = (uint8_t) ((data >> 8) & 0xFFU);
        buf[buf_idx + 1] = (uint8_t) data;
        buf_idx += 2;
    }

    constexpr void set_u8(uint8_t data, uint8_t index) {
        buf[index] = data;
    }


    constexpr void set_u16(uint16_t data, uint8_t index) {
        buf[index] = (uint8_t) ((data >> 8) & 0xFFU);
        buf[index + 1] = (uint8_t) data;
    }


    void push_n(const uint8_t* data, uint8_t size) {
        memcpy(&buf[buf_idx], data, size);
        buf_idx += size;
    }



    uint16_t* get_regs(uint16_t n) {
        uint16_t* msg_buf_ptr = (uint16_t*) (end());
        buf_idx += n * 2;
        while (n--) {
            msg_buf_ptr[n] = (msg_buf_ptr[n] << 8) | ((msg_buf_ptr[n] >> 8) & 0xFF);
        }
        return msg_buf_ptr;
    }


    void push_regs(const uint16_t* data, uint16_t n) {
        uint16_t* msg_buf_ptr = (uint16_t*) (end());
        buf_idx += n * 2;
        while (n--) {
            msg_buf_ptr[n] = (data[n] << 8) | ((data[n] >> 8) & 0xFF);
        }
    }

    constexpr uint8_t* get_n(uint16_t n) {
        uint8_t* msg_buf_ptr = buf + buf_idx;
        buf_idx += n;
        return msg_buf_ptr;
    }

    [[nodiscard]] constexpr uint8_t * end() {return buf + buf_idx;}
    [[nodiscard]] constexpr const uint8_t * end() const {return buf + buf_idx;}

    [[nodiscard]] constexpr uint8_t * begin() {return buf;}
    [[nodiscard]] constexpr const uint8_t * begin() const {return buf;}

    [[nodiscard]] constexpr size_t size() const {return buf_idx;}

    constexpr void clear() {buf_idx = 0;}

// private:

    uint8_t buf[260];
    uint16_t buf_idx;

};

/**
 * Modbus broadcast address. Can be passed to nmbs_set_destination_rtu_address().
 */
static constexpr uint8_t NMBS_BROADCAST_ADDRESS = 0;

/**
 * nanoMODBUS client/server instance type. All struct members are to be considered private,
 * it is not advisable to read/write them directly.
 */
struct NmbsBase {


    /** Set the request/response timeout.
     * If the target instance is a server, sets the timeout of the nmbs_server_poll() function.
     * If the target instance is a client, sets the response timeout after sending a request. In case of timeout,
     * the called method will return NMBS_ERROR_TIMEOUT.
     * @param timeout_ms timeout in milliseconds. If < 0, the timeout is disabled.
     */
    void set_read_timeout(int32_t timeout_ms){
        read_timeout_ms_ = timeout_ms;
    }

    /** Set the timeout between the reception/transmission of two consecutive bytes.
     * @param timeout_ms timeout in milliseconds. If < 0, the timeout is disabled.
     */
    void set_byte_timeout(int32_t timeout_ms){
        byte_timeout_ms_ = timeout_ms;
    }


    /** Create a new NmbsPlatformIntf struct.
     * @param platform_conf pointer to the NmbsPlatformIntf instance
     */
    void platform_conf_create(NmbsPlatformIntf* platform_conf);


    void set_destination_rtu_address(uint8_t address) {
        dest_address_rtu = address;
    }

    IResult<> send(std::span<const uint8_t> bytes) {
        const size_t count = bytes.size();
        const size_t sended = platform_.write(bytes, byte_timeout_ms_);

        if (sended == count)
            return Ok();

        if (sended < count) {
            if (sended < 0)
                return make_err(NmbsError::Transport);

            return make_err(NmbsError::Timeout);
        }

        return make_err(NmbsError::Transport);
    }

    IResult<> recv(size_t count) {
        if (msg_status_.complete) {
            return Ok();
        }

        const size_t ret =
            platform_.read(std::span(buf_.end(), count), byte_timeout_ms_);

        if (ret == count)
            return Ok();

        if (ret < count) {
            if (ret < 0)
                return make_err(NmbsError::Transport);

            return make_err(NmbsError::Timeout);
        }

        return make_err(NmbsError::Transport);
    }

    void flush() {
        platform_.read(buf_.bytes(), 0);
    }

    explicit NmbsBase(const NmbsTransport transport, NmbsPlatformIntf & platform) : platform_(platform) {
        msg_state_reset();
    }
protected:
    void msg_state_reset() {
        buf_.clear();
        msg_status_.reset();
    }

    void push_msg_header(uint16_t data_length) {
        buf_.clear();
        switch(transport_){
            case NmbsTransport::RTU:
                buf_.push_rtu_header(data_length, msg_status_.to_rtu_header_info());
                break;
            case NmbsTransport::TCP:
                buf_.push_tcp_header(data_length, msg_status_.to_tcp_header_info());
                break;
        }
    }


    void set_msg_header_size(uint16_t data_length) {
        if (transport_ == NmbsTransport::TCP) {
            data_length += 2;
            buf_.set_u16(data_length, 4);
        }
    }

    void msg_state_req(FunctionCode fc) {
        if (current_tid == UINT16_MAX)
            current_tid = 1;
        else
            current_tid++;

        // Flush the remaining data on the line before sending the request
        flush();

        msg_state_reset();
        msg_status_.unit_id = dest_address_rtu;
        msg_status_.fc = fc;
        msg_status_.transaction_id = current_tid;
        if (msg_status_.unit_id == NMBS_BROADCAST_ADDRESS && transport_ == NmbsTransport::RTU)
            msg_status_.broadcast = true;
    }

    IResult<> send_msg();

    IResult<> recv_req_header(bool* first_byte_received) {
        const IResult<> res = recv_msg_header(first_byte_received);
        if (res.is_err())
            return res;

        if (transport_ == NmbsTransport::RTU) {
            // Check if request is for us
            if (msg_status_.unit_id == NMBS_BROADCAST_ADDRESS)
                msg_status_.broadcast = true;
            else if (msg_status_.unit_id != address_rtu)
                msg_status_.ignored = true;
            else
                msg_status_.ignored = false;
        }

        return Ok();
    }



    IResult<> send_exception_msg(ModbusException exception) {
        if (msg_status_.broadcast) {
            return Ok();
        }

        msg_status_.fc = std::bit_cast<FunctionCode>(
            static_cast<uint8_t>(0x80 + static_cast<uint8_t>(msg_status_.fc)));
        push_msg_header(1);
        buf_.push_u8(std::bit_cast<uint8_t>(exception));

        NMBS_DEBUG_PRINT("%d NMBS res -> address_rtu %d\texception %d", address_rtu, address_rtu, exception);

        return send_msg();
    }

    void push_res_header(uint16_t data_length) {
        push_msg_header(data_length);
        NMBS_DEBUG_PRINT("%d NMBS res -> address_rtu %d\tfc %d\t", address_rtu, address_rtu, msg_status_.fc);
    }


    IResult<> recv_read_file_record_res(uint16_t* registers, uint16_t count) ;
    IResult<> recv_write_multiple_registers_res(uint16_t address, uint16_t quantity) ;
    IResult<> recv_write_multiple_coils_res(uint16_t address, uint16_t quantity);
    IResult<> recv_write_single_register_res(uint16_t address, uint16_t value_req);
    IResult<> recv_write_single_coil_res(uint16_t address, uint16_t value_req);
    IResult<> recv_read_registers_res(uint16_t quantity, uint16_t* registers);
    IResult<> recv_read_discrete_res(nmbs_bitfield values);
    IResult<> recv_res_header() ;
    IResult<> recv_msg_footer();
    IResult<> recv_msg_header(bool* first_byte_received);
    IResult<> recv_read_device_identification_res(uint8_t buffers_count, char** buffers_out,
        uint8_t buffers_length, const uint8_t* order, uint8_t* ids_out,
        uint8_t* next_object_id_out, uint8_t* objects_count_out);

    IResult<> recv_write_file_record_res(uint16_t file_number, uint16_t record_number,
        const uint16_t* registers, uint16_t count);

    Buffer buf_;
    MsgStatus msg_status_;



    int32_t byte_timeout_ms_ = -1;
    int32_t read_timeout_ms_ = -1;

    NmbsTransport transport_ = NmbsTransport::RTU;
    NmbsPlatformIntf & platform_;

    uint8_t address_rtu;
    uint8_t dest_address_rtu;
    uint16_t current_tid;
};



struct NmbsServer:public NmbsBase{ 
    using Self = NmbsServer;
    struct ServerConfig{
        NmbsTransport transport;
        NmbsPlatformIntf & platform;
        NmbsServerCallback && callback;
        uint8_t address_rtu; 
    };

    static IResult<Self> from(const ServerConfig& cfg) {
        if (cfg.transport == NmbsTransport::RTU && cfg.address_rtu == 0)
            return make_err(NmbsError::InvalidArgument);

        return Ok(Self(cfg));
    }



    /** Handle incoming requests to the server.
     * This function should be called in a loop in order to serve any incoming request. Its maximum duration, in case of no
     * received request, is the value set with nmbs_set_read_timeout() (unless set to < 0).
     *
     * @return NMBS_ERROR_NONE if successful, other errors otherwise.
     */
    IResult<> poll();
private:
    NmbsServerCallback callback_;
    
    template<typename Ev>
    IResult<> callback(Ev ev){
        // auto variant_ev = NmbsEvent(std::monostate{});
        return callback_(NmbsEvent(ev));
    }


    IResult<> handle_read_coils();
    IResult<> handle_read_discrete_inputs();
    IResult<> handle_read_holding_registers();
    IResult<> handle_read_input_registers();
    IResult<> handle_read_device_identification();
    IResult<> handle_read_write_registers();
    IResult<> handle_write_file_record();
    IResult<> handle_read_file_record();
    IResult<> handle_write_multiple_registers();
    IResult<> handle_write_multiple_coils();
    IResult<> handle_write_single_register();
    IResult<> handle_write_single_coil();
    IResult<> handle_read_registers(IResult<> (*callback)(uint16_t, uint16_t, uint16_t*, uint8_t));
    IResult<> handle_read_discrete(IResult<> (*callback)(uint16_t, uint16_t, nmbs_bitfield, uint8_t));

    IResult<> handle_req_fc();

    explicit NmbsServer(const ServerConfig& cfg):NmbsBase(cfg.transport, cfg.platform){ 
        address_rtu = cfg.address_rtu;
        callback_ = std::move(cfg.callback);
    }

};
struct NmbsClient:public NmbsBase{
public:
    using Self = NmbsClient;
    struct ClientConfig{
        NmbsTransport transport;
        NmbsPlatformIntf & platform;
    };

    IResult<Self> from(const ClientConfig & cfg) {
        return Ok(Self(cfg));
    }


    /** Send a FC 01 (0x01) Read Coils request
     * @param address starting address
     * @param quantity quantity of coils
     * @param coils_out nmbs_bitfield where the coils will be stored
     *
     * @return NMBS_ERROR_NONE if successful, other errors otherwise.
     */
    IResult<> read_coils(uint16_t address, uint16_t quantity, nmbs_bitfield coils_out);

    /** Send a FC 02 (0x02) Read Discrete inputs request
     * @param address starting address
     * @param quantity quantity of inputs
     * @param inputs_out nmbs_bitfield where the discrete inputs will be stored
     *
     * @return NMBS_ERROR_NONE if successful, other errors otherwise.
     */
    IResult<> read_discrete_inputs(uint16_t address, uint16_t quantity, nmbs_bitfield inputs_out);

    /** Send a FC 03 (0x03) Read Holding Registers request
     * @param address starting address
     * @param quantity quantity of registers
     * @param registers_out array where the registers will be stored
     *
     * @return NMBS_ERROR_NONE if successful, other errors otherwise.
     */
    IResult<> read_holding_registers(uint16_t address, uint16_t quantity, uint16_t* registers_out);

    /** Send a FC 04 (0x04) Read input Registers request
     * @param address starting address
     * @param quantity quantity of registers
     * @param registers_out array where the registers will be stored
     *
     * @return NMBS_ERROR_NONE if successful, other errors otherwise.
     */
    IResult<> read_input_registers(uint16_t address, uint16_t quantity, uint16_t* registers_out);

    /** Send a FC 05 (0x05) Write Single Coil request
     * @param address coil address
     * @param value coil value
     *
     * @return NMBS_ERROR_NONE if successful, other errors otherwise.
     */
    IResult<> write_single_coil(uint16_t address, bool value);

    /** Send a FC 06 (0x06) Write Single Register request
     * @param address register address
     * @param value register value
     *
     * @return NMBS_ERROR_NONE if successful, other errors otherwise.
     */
    IResult<> write_single_register(uint16_t address, uint16_t value);

    /** Send a FC 15 (0x0F) Write Multiple Coils
     * @param address starting address
     * @param quantity quantity of coils
     * @param coils bitfield of coils values
     *
     * @return NMBS_ERROR_NONE if successful, other errors otherwise.
     */
    IResult<> write_multiple_coils(uint16_t address, uint16_t quantity, const nmbs_bitfield coils);

    /** Send a FC 16 (0x10) Write Multiple Registers
     * @param address starting address
     * @param quantity quantity of registers
     * @param registers array of registers values
     *
     * @return NMBS_ERROR_NONE if successful, other errors otherwise.
     */
    IResult<> write_multiple_registers(uint16_t address, uint16_t quantity, const uint16_t* registers);

    /** Send a FC 20 (0x14) Read File Record
     * @param file_number file number (1 to 65535)
     * @param record_number record number from file (0000 to 9999)
     * @param registers array of registers to read
     * @param count count of registers
     *
     * @return NMBS_ERROR_NONE if successful, other errors otherwise.
     */
    IResult<> read_file_record(uint16_t file_number, uint16_t record_number, uint16_t* registers,
                                    uint16_t count);

    /** Send a FC 21 (0x15) Write File Record
     * @param file_number file number (1 to 65535)
     * @param record_number record number from file (0000 to 9999)
     * @param registers array of registers to write
     * @param count count of registers
     *
     * @return NMBS_ERROR_NONE if successful, other errors otherwise.
     */
    IResult<> write_file_record(uint16_t file_number, uint16_t record_number, const uint16_t* registers,
                                    uint16_t count);

    /** Send a FC 23 (0x17) Read Write Multiple registers
     * @param read_address starting read address
     * @param read_quantity quantity of registers to read
     * @param registers_out array where the read registers will be stored
     * @param write_address starting write address
     * @param write_quantity quantity of registers to write
     * @param registers array of registers values to write
     *
     * @return NMBS_ERROR_NONE if successful, other errors otherwise.
     */
    IResult<> read_write_registers(uint16_t read_address, uint16_t read_quantity,
                                        uint16_t* registers_out, uint16_t write_address, uint16_t write_quantity,
                                        const uint16_t* registers);

    /** Send a FC 43 / 14 (0x2B / 0x0E) Read Device Identification to read all Basic Object Id values (Read Device ID code 1)
     * @param vendor_name char array where the read VendorName value will be stored
     * @param product_code char array where the read ProductCode value will be stored
     * @param major_minor_revision char array where the read MajorMinorRevision value will be stored
     * @param buffers_length length of every char array
     *
     * @return NMBS_ERROR_NONE if successful, other errors otherwise.
     */
    IResult<> read_device_identification_basic(char* vendor_name, char* product_code,
                                                    char* major_minor_revision, uint8_t buffers_length);

    /** Send a FC 43 / 14 (0x2B / 0x0E) Read Device Identification to read all Regular Object Id values (Read Device ID code 2)
     * @param vendor_url char array where the read VendorUrl value will be stored
     * @param product_name char array where the read ProductName value will be stored
     * @param model_name char array where the read ModelName value will be stored
     * @param user_application_name char array where the read UserApplicationName value will be stored
     * @param buffers_length length of every char array
     *
     * @return NMBS_ERROR_NONE if successful, other errors otherwise.
     */
    IResult<> read_device_identification_regular(char* vendor_url, char* product_name, char* model_name,
                                                    char* user_application_name, uint8_t buffers_length);

    /** Send a FC 43 / 14 (0x2B / 0x0E) Read Device Identification to read all Extended Object Id values (Read Device ID code 3)
     * @param object_id_start Object Id to start reading from
     * @param ids array where the read Object Ids will be stored
     * @param buffers array of char arrays where the read values will be stored
     * @param ids_length length of the ids array and buffers array
     * @param buffer_length length of each char array
     * @param objects_count_out retrieved Object Ids count
     *
     * @return NMBS_ERROR_NONE if successful, NMBS_INVALID_ARGUMENT if buffers_count is less than retrieved Object Ids count,
     * other errors otherwise.
     */
    IResult<> read_device_identification_extended(
        uint8_t object_id_start, uint8_t* ids, char** buffers,
        uint8_t ids_length, uint8_t buffer_length,
        uint8_t* objects_count_out);

    /** Send a FC 43 / 14 (0x2B / 0x0E) Read Device Identification to retrieve a single Object Id value (Read Device ID code 4)
     * @param object_id requested Object Id
     * @param buffer char array where the resulting value will be stored
     * @param buffer_length length of the char array
     *
     * @return NMBS_ERROR_NONE if successful, other errors otherwise.
     */
    IResult<> read_device_identification(uint8_t object_id, char* buffer, uint8_t buffer_length);

    /** Send a raw Modbus PDU.
     * CRC on RTU will be calculated and sent by this function.
     * @param fc request function code
     * @param data request data. It's up to the caller to convert this data to network byte order
     * @param data_len length of the data parameter
     *
     * @return NMBS_ERROR_NONE if successful, other errors otherwise.
     */
    IResult<> send_raw_pdu(FunctionCode fc, const uint8_t* data, uint16_t data_len);

    /** Receive a raw response Modbus PDU.
     * @param data_out response data. It's up to the caller to convert this data to host byte order. Can be NULL.
     * @param data_out_len number of bytes to receive
     *
     * @return NMBS_ERROR_NONE if successful, other errors otherwise.
     */
    IResult<> receive_raw_pdu_response(uint8_t* data_out, uint8_t data_out_len);

private:
    explicit NmbsClient(ClientConfig cfg):NmbsBase(cfg.transport,cfg.platform){
        byte_timeout_ms_ = -1;
        read_timeout_ms_ = -1;
    }

    IResult<> read_registers(FunctionCode fc, uint16_t address, uint16_t quantity, uint16_t* registers) {
        if (quantity < 1 || quantity > 125)
            return make_err(NmbsError::InvalidArgument);

        if ((uint32_t) address + (uint32_t) quantity > ((uint32_t) 0xFFFF) + 1)
            return make_err(NmbsError::InvalidArgument);

        msg_state_req(fc);
        push_req_header(4);

        buf_.push_u16(address);
        buf_.push_u16(quantity);

        NMBS_DEBUG_PRINT("a %d\tq %d ", address, quantity);

        const IResult<> res = send_msg();
        if (res.is_err())
            return res;

        return recv_read_registers_res(quantity, registers);
    }

    IResult<> read_discrete(FunctionCode fc, uint16_t address, uint16_t quantity, nmbs_bitfield values) {
        if (quantity < 1 || quantity > NMBS_BITFIELD_MAX)
            return make_err(NmbsError::InvalidArgument);

        if ((uint32_t) address + (uint32_t) quantity > ((uint32_t) 0xFFFF) + 1)
            return make_err(NmbsError::InvalidArgument);

        msg_state_req(fc);
        push_req_header(4);

        buf_.push_u16(address);
        buf_.push_u16(quantity);

        NMBS_DEBUG_PRINT("a %d\tq %d", address, quantity);

        const IResult<> res = send_msg();
        if (res.is_err())
            return res;

        return recv_read_discrete_res(values);
    }

    void push_req_header(uint16_t data_length) {
        push_msg_header(data_length);
        #ifdef NMBS_DEBUG_EN
        printf("%d ", address_rtu);
        printf("NMBS req -> ");
        if (transport_ == NmbsTransport::RTU) {
            if (msg_status_.broadcast)
                printf("broadcast\t");
            else
                printf("address_rtu %d\t", dest_address_rtu);
        }

        printf("fc %d\t", msg_status_.fc);
        #endif
    }
};

}