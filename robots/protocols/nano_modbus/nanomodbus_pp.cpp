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

#include "nanomodbus_pp.hpp"


#include <stdbool.h>
#include <stdint.h>
#include <string.h>

using namespace ymd;
using namespace ymd::modbus;


/** Calculate the Modbus CRC of some data.
 * @param data Data
 * @param length Length of the data
 */

[[nodiscard]] static constexpr uint16_t nmbs_crc_calc(std::span<const uint8_t> bytes) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < bytes.size(); i++) {
        crc ^= (uint16_t) bytes[i];
        for (int j = 8; j != 0; j--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
                crc >>= 1;
        }
    }

    return (uint16_t) (crc << 8) | (uint16_t) (crc >> 8);
}


static constexpr void swap_u16_endian(std::span<uint16_t> pbuf) {

    for(size_t i = 0; i < pbuf.size(); i++) {
        pbuf[i] = __bswap16(pbuf[i]);
    }
}


IResult<> NmbsBase::recv_msg_footer() {
    NMBS_DEBUG_PRINT("\n");

    if (transport_ == NmbsTransport::RTU) {
        const uint16_t crc = nmbs_crc_calc(buf_.bytes());

        if(const auto res = recv(2);
            res.is_err()) return res;

        const uint16_t recv_crc = buf_.pop_u16();
        if (recv_crc != crc)
            return make_err(NmbsError::InvalidCrc);
    }

    return Ok();
}


IResult<> NmbsBase::send_msg() {
    NMBS_DEBUG_PRINT("\n");

    if (transport_ == NmbsTransport::RTU) {
        const uint16_t crc = nmbs_crc_calc(buf_.bytes());
        buf_.push_u16(crc);
    }

    return send(buf_.bytes());
}

IResult<> NmbsBase::recv_msg_header(bool* first_byte_received) {
    // We wait for the read timeout here, just for the first message byte
    const auto old_byte_timeout = byte_timeout_ms_;
    byte_timeout_ms_ = read_timeout_ms_;

    msg_state_reset();

    *first_byte_received = false;

    if (transport_ == NmbsTransport::RTU) {
        IResult<> res = recv(1);

        byte_timeout_ms_ = old_byte_timeout;

        if (res.is_err())
            return res;

        *first_byte_received = true;

        msg_status_.unit_id = buf_.pop_u8();

        res = recv(1);
        if (res.is_err())
            return res;

        msg_status_.fc = std::bit_cast<FunctionCode>(buf_.pop_u8());
    }
    else if (transport_ == NmbsTransport::TCP) {
        IResult<> res = recv(1);

        byte_timeout_ms_ = old_byte_timeout;

        if (res.is_err())
            return res;

        *first_byte_received = true;

        buf_.push_zero();

        res = recv(7);
        if (res.is_err())
            return res;

        // Starting over
        buf_.clear();

        msg_status_.transaction_id = buf_.pop_u16();
        const uint16_t protocol_id = buf_.pop_u16();
        const uint16_t length = buf_.pop_u16();
        msg_status_.unit_id = buf_.pop_u8();
        msg_status_.fc = std::bit_cast<FunctionCode>(buf_.pop_u8());

        if (length < 2 || length > 255)
            return make_err(NmbsError::InvalidTcpMbap);

        // Receive the rest of the message
        res = recv(length - 2);
        if (res.is_err())
            return res;

        if (protocol_id != 0)
            return make_err(NmbsError::InvalidTcpMbap);

        msg_status_.complete = true;
    }

    return Ok();
}


IResult<> NmbsBase::recv_res_header() {
    const uint16_t req_transaction_id = msg_status_.transaction_id;
    const uint8_t req_unit_id = msg_status_.unit_id;
    const auto req_fc = msg_status_.fc;

    bool first_byte_received = false;
    IResult<> res = recv_msg_header(&first_byte_received);
    if (res.is_err())
        return res;

    if (transport_ == NmbsTransport::TCP) {
        if (msg_status_.transaction_id != req_transaction_id)
            return make_err(NmbsError::InvalidTcpMbap);
    }

    if (transport_ == NmbsTransport::RTU && msg_status_.unit_id != req_unit_id)
        return make_err(NmbsError::InvalidUnitId);

    if (msg_status_.fc != req_fc) {
        if (static_cast<uint8_t>(msg_status_.fc) - 0x80 == static_cast<uint8_t>(req_fc)) {
            res = recv(1);
            if (res.is_err())
                return res;

            const uint8_t exception = buf_.pop_u8();
            res = recv_msg_footer();
            if (res.is_err())
                return res;

            if (exception < 1 || exception > 4)
                return make_err(NmbsError::InvalidResponse);

            NMBS_DEBUG_PRINT("%d NMBS res <- address_rtu %d\texception %d\n", address_rtu, msg_status_.unit_id, exception);
            return Err(NmbsError(std::bit_cast<ModbusException>(exception)));
        }

        return make_err(NmbsError::InvalidResponse);
    }

    NMBS_DEBUG_PRINT("%d NMBS res <- address_rtu %d\tfc %d\t", address_rtu, msg_status_.unit_id, msg_status_.fc);

    return Ok();
}


IResult<> NmbsBase::recv_read_discrete_res(nmbs_bitfield values) {
    IResult<> res = recv_res_header();
    if (res.is_err())
        return res;

    res = recv(1);
    if (res.is_err())
        return res;

    const uint8_t coils_bytes = buf_.pop_u8();
    NMBS_DEBUG_PRINT("b %d\t", coils_bytes);

    if (coils_bytes > NMBS_BITFIELD_BYTES_MAX) {
        return make_err(NmbsError::InvalidResponse);
    }

    res = recv(coils_bytes);
    if (res.is_err())
        return res;

    NMBS_DEBUG_PRINT("coils ");
    for (size_t i = 0; i < coils_bytes; i++) {
        const uint8_t coil = buf_.pop_u8();
        if (values)
            values[i] = coil;
        NMBS_DEBUG_PRINT("%d ", coil);
    }

    res = recv_msg_footer();
    if (res.is_err())
        return res;

    return Ok();
}

IResult<> NmbsBase::recv_read_registers_res(uint16_t quantity, uint16_t* registers) {
    IResult<> res = recv_res_header();
    if (res.is_err())
        return res;

    res = recv(1);
    if (res.is_err())
        return res;

    const uint8_t registers_bytes = buf_.pop_u8();
    NMBS_DEBUG_PRINT("b %d\t", registers_bytes);

    if (registers_bytes > 250)
        return make_err(NmbsError::InvalidResponse);

    res = recv(registers_bytes);
    if (res.is_err())
        return res;

    NMBS_DEBUG_PRINT("regs ");
    for (size_t i = 0; i < registers_bytes / 2; i++) {
        uint16_t reg = buf_.pop_u16();
        if (registers)
            registers[i] = reg;
        NMBS_DEBUG_PRINT("%d ", reg);
    }

    res = recv_msg_footer();
    if (res.is_err())
        return res;

    if (registers_bytes != quantity * 2)
        return make_err(NmbsError::InvalidResponse);

    return Ok();
}



IResult<> NmbsBase::recv_write_single_coil_res(uint16_t address, uint16_t value_req) {
    IResult<> res = recv_res_header();
    if (res.is_err())
        return res;

    res = recv(4);
    if (res.is_err())
        return res;

    const uint16_t address_res = buf_.pop_u16();
    const uint16_t value_res = buf_.pop_u16();

    NMBS_DEBUG_PRINT("a %d\tvalue %d", address, value_res);

    res = recv_msg_footer();
    if (res.is_err())
        return res;

    if (address_res != address)
        return make_err(NmbsError::InvalidResponse);

    if (value_res != value_req)
        return make_err(NmbsError::InvalidResponse);

    return Ok();
}


IResult<> NmbsBase::recv_write_single_register_res(uint16_t address, uint16_t value_req) {
    IResult<> res = recv_res_header();
    if (res.is_err())
        return res;

    res = recv(4);
    if (res.is_err())
        return res;

    const uint16_t address_res = buf_.pop_u16();
    const uint16_t value_res = buf_.pop_u16();
    NMBS_DEBUG_PRINT("a %d\tvalue %d ", address, value_res);

    res = recv_msg_footer();
    if (res.is_err())
        return res;

    if (address_res != address)
        return make_err(NmbsError::InvalidResponse);

    if (value_res != value_req)
        return make_err(NmbsError::InvalidResponse);

    return Ok();
}


IResult<> NmbsBase::recv_write_multiple_coils_res(uint16_t address, uint16_t quantity) {
    IResult<> res = recv_res_header();
    if (res.is_err())
        return res;

    res = recv(4);
    if (res.is_err())
        return res;

    const uint16_t address_res = buf_.pop_u16();
    const uint16_t quantity_res = buf_.pop_u16();
    NMBS_DEBUG_PRINT("a %d\tq %d", address_res, quantity_res);

    res = recv_msg_footer();
    if (res.is_err())
        return res;

    if (address_res != address)
        return make_err(NmbsError::InvalidResponse);

    if (quantity_res != quantity)
        return make_err(NmbsError::InvalidResponse);

    return Ok();
}


IResult<> NmbsBase::recv_write_multiple_registers_res(uint16_t address, uint16_t quantity) {
    IResult<> res = recv_res_header();
    if (res.is_err())
        return res;

    res = recv(4);
    if (res.is_err())
        return res;

    const uint16_t address_res = buf_.pop_u16();
    const uint16_t quantity_res = buf_.pop_u16();
    NMBS_DEBUG_PRINT("a %d\tq %d", address_res, quantity_res);

    res = recv_msg_footer();
    if (res.is_err())
        return res;

    if (address_res != address)
        return make_err(NmbsError::InvalidResponse);

    if (quantity_res != quantity)
        return make_err(NmbsError::InvalidResponse);

    return Ok();
}


IResult<> NmbsBase::recv_read_file_record_res(uint16_t* registers, uint16_t count) {
    IResult<> res = recv_res_header();
    if (res.is_err())
        return res;

    res = recv(1);
    if (res.is_err())
        return res;

    const uint8_t response_size = buf_.pop_u8();
    if (response_size > 250) {
        return make_err(NmbsError::InvalidResponse);
    }

    res = recv(response_size);
    if (res.is_err())
        return res;

    const uint8_t subreq_data_size = buf_.pop_u8() - 1;
    const uint8_t subreq_reference_type = buf_.pop_u8();
    uint16_t* subreq_record_data = (uint16_t*) buf_.get_n(subreq_data_size);

    res = recv_msg_footer();
    if (res.is_err())
        return res;

    if (registers) {
        if (subreq_reference_type != 6)
            return make_err(NmbsError::InvalidResponse);

        if (count != (subreq_data_size / 2))
            return make_err(NmbsError::InvalidResponse);

        swap_u16_endian(std::span(subreq_record_data, subreq_data_size / 2));
        memcpy(registers, subreq_record_data, subreq_data_size);
    }

    return Ok();
}


IResult<> NmbsBase::recv_write_file_record_res(
    uint16_t file_number, uint16_t record_number,
    const uint16_t* registers, uint16_t count
) {
    IResult<> res = recv_res_header();
    if (res.is_err())
        return res;

    res = recv(1);
    if (res.is_err())
        return res;

    const uint8_t response_size = buf_.pop_u8();
    if (response_size > 251)
        return make_err(NmbsError::InvalidResponse);

    res = recv(response_size);
    if (res.is_err())
        return res;

    const uint8_t subreq_reference_type = buf_.pop_u8();
    const uint16_t subreq_file_number = buf_.pop_u16();
    const uint16_t subreq_record_number = buf_.pop_u16();
    const uint16_t subreq_record_length = buf_.pop_u16();
    NMBS_DEBUG_PRINT("a %d\tr %d\tl %d\t fwrite ", subreq_file_number, subreq_record_number, subreq_record_length);

    uint16_t subreq_data_size = subreq_record_length * 2;
    uint16_t* subreq_record_data = (uint16_t*) buf_.get_n(subreq_data_size);

    res = recv_msg_footer();
    if (res.is_err())
        return res;

    if (registers) {
        if (subreq_reference_type != 6)
            return make_err(NmbsError::InvalidResponse);

        if (subreq_file_number != file_number)
            return make_err(NmbsError::InvalidResponse);

        if (subreq_record_number != record_number)
            return make_err(NmbsError::InvalidResponse);

        if (subreq_record_length != count)
            return make_err(NmbsError::InvalidResponse);

        swap_u16_endian(std::span(subreq_record_data, subreq_record_length));
        if (memcmp(registers, subreq_record_data, subreq_data_size) != 0)
            return make_err(NmbsError::InvalidResponse);
    }

    return Ok();
}

IResult<> NmbsBase::recv_read_device_identification_res(uint8_t buffers_count, char** buffers_out,
    uint8_t buffers_length, const uint8_t* order, uint8_t* ids_out,
    uint8_t* next_object_id_out, uint8_t* objects_count_out
) {
    IResult<> res = recv_res_header();
    if (res.is_err())
        return res;

    res = recv(6);
    if (res.is_err())
        return res;

    const uint8_t mei_type = buf_.pop_u8();
    if (mei_type != 0x0E)
        return make_err(NmbsError::InvalidResponse);

    const uint8_t read_device_id_code = buf_.pop_u8();
    if (read_device_id_code < 1 || read_device_id_code > 4)
        return make_err(NmbsError::InvalidResponse);

    const uint8_t conformity_level = buf_.pop_u8();
    if (conformity_level < 1 || (conformity_level > 3 && conformity_level < 0x81) || conformity_level > 0x83)
        return make_err(NmbsError::InvalidResponse);

    const uint8_t more_follows = buf_.pop_u8();
    if (more_follows != 0 && more_follows != 0xFF)
        return make_err(NmbsError::InvalidResponse);

    uint8_t next_object_id = buf_.pop_u8();

    const uint8_t objects_count = buf_.pop_u8();
    if (objects_count_out)
        *objects_count_out = objects_count;

    if (buffers_count == 0) {
        buffers_out = nullptr;
    }
    else if (objects_count > buffers_count)
        return make_err(NmbsError::InvalidArgument);

    if (more_follows == 0)
        next_object_id = 0x7F;    // This value is reserved in the spec, we use it to signal stream is finished

    if (next_object_id_out)
        *next_object_id_out = next_object_id;

    uint8_t res_size_left = 253 - 7;
    for (size_t i = 0; i < objects_count; i++) {
        res = recv(2);
        if (res.is_err())
            return res;

        const uint8_t object_id = buf_.pop_u8();
        const uint8_t object_length = buf_.pop_u8();
        res_size_left -= 2;

        if (object_length > res_size_left)
            return make_err(NmbsError::InvalidResponse);

        res = recv(object_length);
        if (res.is_err())
            return res;

        const char* str = (const char*) buf_.get_n(object_length);

        if (ids_out)
            ids_out[i] = object_id;

        uint8_t buf_index = i;
        if (order)
            buf_index = order[object_id];
        if (buffers_out) {
            strncpy(buffers_out[buf_index], str, buffers_length);
            buffers_out[buf_index][object_length] = 0;
        }
    }

    return recv_msg_footer();
}



IResult<> NmbsServer::handle_read_discrete(IResult<> (*cb)(uint16_t, uint16_t, nmbs_bitfield, uint8_t)) {
    IResult<> res = recv(4);
    if (res.is_err())
        return res;

    const uint16_t address = buf_.pop_u16();
    const uint16_t quantity = buf_.pop_u16();

    NMBS_DEBUG_PRINT("a %d\tq %d", address, quantity);

    res = recv_msg_footer();
    if (res.is_err())
        return res;

    if (msg_status_.ignored) return recv_read_discrete_res(nullptr);
    if (quantity < 1 || quantity > 2000)
        return send_exception_msg(ModbusException::IllegalDataValue);

    if ((uint32_t) address + (uint32_t) quantity > ((uint32_t) 0xFFFF) + 1)
        return send_exception_msg(ModbusException::IllegalDataAddress);

    if (cb == nullptr) return send_exception_msg(ModbusException::IllegalFunction);

    nmbs_bitfield bitfield = {0};
    res = cb(address, quantity, bitfield, msg_status_.unit_id);
    if (res.is_err()) {
        if (res.is_err() and res.unwrap_err().is_exception())
            return send_exception_msg(res.unwrap_err().unwrap_as<ModbusException>());

        return send_exception_msg(ModbusException::ServerDeviceFailure);
    }

    if (msg_status_.broadcast == false) {
        uint8_t discrete_bytes = (quantity + 7) / 8;
        push_res_header(1 + discrete_bytes);

        buf_.push_u8(discrete_bytes);

        NMBS_DEBUG_PRINT("b %d\t", discrete_bytes);

        NMBS_DEBUG_PRINT("coils ");
        for (size_t i = 0; i < discrete_bytes; i++) {
            buf_.push_u8(bitfield[i]);
            NMBS_DEBUG_PRINT("%d ", bitfield[i]);
        }

        res = send_msg();
        if (res.is_err())
            return res;
    }


    return Ok();
}




IResult<> NmbsServer::handle_read_registers(IResult<> (*cb)(uint16_t, uint16_t, uint16_t*, uint8_t)) {
    IResult<> res = recv(4);
    if (res.is_err())
        return res;

    const uint16_t address = buf_.pop_u16();
    const uint16_t quantity = buf_.pop_u16();

    NMBS_DEBUG_PRINT("a %d\tq %d", address, quantity);

    res = recv_msg_footer();
    if (res.is_err())
        return res;

    if (msg_status_.ignored) return recv_read_registers_res(quantity, nullptr);
    if (quantity < 1 || quantity > 125)
        return send_exception_msg(ModbusException::IllegalDataValue);

    if ((uint32_t) address + (uint32_t) quantity > ((uint32_t) 0xFFFF) + 1)
        return send_exception_msg(ModbusException::IllegalDataAddress);

    if (cb == nullptr) return send_exception_msg(ModbusException::IllegalFunction);

    uint16_t regs[125] = {0};
    res = cb(address, quantity, regs, msg_status_.unit_id);
    if (res.is_err()) {
        if (res.is_err() and res.unwrap_err().is_exception())
            return send_exception_msg(res.unwrap_err().unwrap_as<ModbusException>());

        return send_exception_msg(ModbusException::ServerDeviceFailure);
    }

    // TODO check all these read request broadcast use cases
    if (msg_status_.broadcast == false) {
        const uint8_t regs_bytes = quantity * 2;
        push_res_header(1 + regs_bytes);

        buf_.push_u8(regs_bytes);

        NMBS_DEBUG_PRINT("b %d\t", regs_bytes);

        NMBS_DEBUG_PRINT("regs ");
        for (size_t i = 0; i < quantity; i++) {
            buf_.push_u16(regs[i]);
            NMBS_DEBUG_PRINT("%d ", regs[i]);
        }

        res = send_msg();
        if (res.is_err())
            return res;
    }


    return Ok();
}





IResult<> NmbsServer::handle_read_coils() {
    // return handle_read_discrete(callback_.read_coils);
    TODO();
    return Ok();
}



IResult<> NmbsServer::handle_read_discrete_inputs() {
    // return handle_read_discrete(callback_.read_discrete_inputs);
    TODO();
    return Ok();
}


IResult<> NmbsServer::handle_read_holding_registers() {
    // return handle_read_registers(callback_.read_holding_registers);
    TODO();
    return Ok();
}




IResult<> NmbsServer::handle_read_input_registers() {
    // return handle_read_registers(callback_.read_input_registers);
    TODO();
    return Ok();
}



IResult<> NmbsServer::handle_write_single_coil() {
    if(const auto res = recv(4);
        res.is_err()) return res;

    const uint16_t address = buf_.pop_u16();
    const uint16_t value = buf_.pop_u16();

    NMBS_DEBUG_PRINT("a %d\tvalue %d", address, value);

    if(const auto res = recv_msg_footer();
        res.is_err()) return res;

    if (msg_status_.ignored)
        return recv_write_single_coil_res(address, value);

    if (value != 0 && value != 0xFF00)
        return send_exception_msg(ModbusException::IllegalDataValue);

    const auto ev = NmbsEvents::WriteSingleCoil{
        .address = address, 
        .value = value == 0 ? false : true, 
        .unit_id = msg_status_.unit_id
    };

    if(const auto res = callback(ev);
        res.is_err()) {
        if (res.unwrap_err().is_exception())
            return send_exception_msg(res.unwrap_err().unwrap_as<ModbusException>());

        return send_exception_msg(ModbusException::ServerDeviceFailure);
    }

    if (msg_status_.broadcast) return Ok();

    push_res_header(4);

    buf_.push_u16(address);
    buf_.push_u16(value);
    NMBS_DEBUG_PRINT("a %d\tvalue %d", address, value);

    if(const auto res = send_msg();
        res.is_err()) return res;

    return Ok();
}




IResult<> NmbsServer::handle_write_single_register() {
    IResult<> res = recv(4);
    if (res.is_err())
        return res;

    const uint16_t address = buf_.pop_u16();
    const uint16_t value = buf_.pop_u16();

    NMBS_DEBUG_PRINT("a %d\tvalue %d", address, value);

    res = recv_msg_footer();
    if (res.is_err())
        return res;

    if (msg_status_.ignored) return recv_write_single_register_res(address, value);
    res = callback(NmbsEvents::WriteSingleRegister{
        .address = address, 
        .value = value, 
        .unit_id = msg_status_.unit_id
    });
    if (res.is_err()) {
        if (res.is_err() and res.unwrap_err().is_exception())
            return send_exception_msg(res.unwrap_err().unwrap_as<ModbusException>());

        return send_exception_msg(ModbusException::ServerDeviceFailure);
    }

    if (msg_status_.broadcast == false) {
        push_res_header(4);

        buf_.push_u16(address);
        buf_.push_u16(value);
        NMBS_DEBUG_PRINT("a %d\tvalue %d", address, value);

        res = send_msg();
        if (res.is_err())
            return res;
    }
    return Ok();
}




IResult<> NmbsServer::handle_write_multiple_coils() {
    IResult<> res = recv(5);
    if (res.is_err())
        return res;

    const uint16_t address = buf_.pop_u16();
    const uint16_t quantity = buf_.pop_u16();
    uint8_t coils_bytes = buf_.pop_u8();

    NMBS_DEBUG_PRINT("a %d\tq %d\tb %d\tcoils ", address, quantity, coils_bytes);

    if (coils_bytes > 246)
        return make_err(NmbsError::InvalidRequst);

    res = recv(coils_bytes);
    if (res.is_err())
        return res;

    nmbs_bitfield coils = {0};
    for (size_t i = 0; i < coils_bytes; i++) {
        coils[i] = buf_.pop_u8();
        NMBS_DEBUG_PRINT("%d ", coils[i]);
    }

    res = recv_msg_footer();
    if (res.is_err())
        return res;

    if (!msg_status_.ignored) {
        if (quantity < 1 || quantity > 0x07B0)
            return send_exception_msg(ModbusException::IllegalDataValue);

        if ((uint32_t) address + (uint32_t) quantity > ((uint32_t) 0xFFFF) + 1)
            return send_exception_msg(ModbusException::IllegalDataAddress);

        if (coils_bytes == 0)
            return send_exception_msg(ModbusException::IllegalDataValue);

        if ((quantity + 7) / 8 != coils_bytes)
            return send_exception_msg(ModbusException::IllegalDataValue);

        res = callback(NmbsEvents::WriteMultipleCoils{
            .address = address, 
            .quantity = quantity, 
            .coils = coils, 
            .unit_id = msg_status_.unit_id
        });

        if (res.is_err()) {
            if (res.is_err() and res.unwrap_err().is_exception())
                return send_exception_msg(res.unwrap_err().unwrap_as<ModbusException>());

            return send_exception_msg(ModbusException::ServerDeviceFailure);
        }

        if (msg_status_.broadcast == false) {
            push_res_header(4);

            buf_.push_u16(address);
            buf_.push_u16(quantity);
            NMBS_DEBUG_PRINT("a %d\tq %d", address, quantity);

            res = send_msg();
            if (res.is_err())
                return res;
        }

    }
    else {
        return recv_write_multiple_coils_res(address, quantity);
    }

    return Ok();
}



IResult<> NmbsServer::handle_write_multiple_registers() {
    if(const auto res = recv(5);
        res.is_err()) return res;

    const uint16_t address = buf_.pop_u16();
    const uint16_t quantity = buf_.pop_u16();
    const uint8_t registers_bytes = buf_.pop_u8();

    NMBS_DEBUG_PRINT("a %d\tq %d\tb %d\tregs ", address, quantity, registers_bytes);

    if(const auto res = recv(registers_bytes);
        res.is_err()) return res;

    if (registers_bytes > 246)
        return make_err(NmbsError::InvalidRequst);

    uint16_t registers[0x007B];
    for (size_t i = 0; i < registers_bytes / 2; i++) {
        registers[i] = buf_.pop_u16();
        NMBS_DEBUG_PRINT("%d ", registers[i]);
    }

    if(const auto res = recv_msg_footer();
        res.is_err()) return res;

    if (msg_status_.ignored)        return recv_write_multiple_registers_res(address, quantity);
    if (quantity < 1 || quantity > 0x007B)
        return send_exception_msg(ModbusException::IllegalDataValue);

    if ((uint32_t) address + (uint32_t) quantity > ((uint32_t) 0xFFFF) + 1)
        return send_exception_msg(ModbusException::IllegalDataAddress);

    if (registers_bytes == 0)
        return send_exception_msg(ModbusException::IllegalDataValue);

    if (registers_bytes != quantity * 2)
        return send_exception_msg(ModbusException::IllegalDataValue);

    if(const auto res = callback(NmbsEvents::WriteMultipleRegisters{
        .address = address, 
        .quantity = quantity, 
        .registers = registers, 
        .unit_id = msg_status_.unit_id
    }); res.is_err()) {
        if (res.is_err() and res.unwrap_err().is_exception())
            return send_exception_msg(res.unwrap_err().unwrap_as<ModbusException>());

        return send_exception_msg(ModbusException::ServerDeviceFailure);
    }

    if (msg_status_.broadcast == false) {
        push_res_header(4);

        buf_.push_u16(address);
        buf_.push_u16(quantity);
        NMBS_DEBUG_PRINT("a %d\tq %d", address, quantity);

        if(const auto res = send_msg();
            res.is_err()) return res;
    }


    return Ok();
}



IResult<> NmbsServer::handle_read_file_record() {
    IResult<> res = recv(1);
    if (res.is_err())
        return res;

    const uint8_t request_size = buf_.pop_u8();
    if (request_size > 245)
        return make_err(NmbsError::InvalidRequst);

    res = recv(request_size);
    if (res.is_err())
        return res;

    const uint8_t subreq_header_size = 7;
    const uint8_t subreq_count = request_size / subreq_header_size;

    struct {
        uint8_t reference_type;
        uint16_t file_number;
        uint16_t record_number;
        uint16_t record_length;
    }
#if defined(__STDC_NO_VLA__) || defined(_MSC_VER)
    subreq[35];    // 245 / subreq_header_size
#else
    subreq[subreq_count];
#endif

    uint8_t response_data_size = 0;

    for (uint8_t i = 0; i < subreq_count; i++) {
        subreq[i].reference_type = buf_.pop_u8();
        subreq[i].file_number = buf_.pop_u16();
        subreq[i].record_number = buf_.pop_u16();
        subreq[i].record_length = buf_.pop_u16();

        response_data_size += 2 + subreq[i].record_length * 2;
    }

    buf_.discard_n(request_size % subreq_header_size);

    res = recv_msg_footer();
    if (res.is_err())
        return res;

    if (msg_status_.ignored) return recv_read_file_record_res(nullptr, 0);
    if (request_size % subreq_header_size)
        return send_exception_msg(ModbusException::IllegalDataValue);

    if (request_size < 0x07 || request_size > 0xF5)
        return send_exception_msg(ModbusException::IllegalDataValue);

    for (uint8_t i = 0; i < subreq_count; i++) {
        if (subreq[i].reference_type != 0x06)
            return send_exception_msg(ModbusException::IllegalDataAddress);

        if (subreq[i].file_number == 0x0000)
            return send_exception_msg(ModbusException::IllegalDataAddress);

        if (subreq[i].record_number > 0x270F)
            return send_exception_msg(ModbusException::IllegalDataAddress);

        if (subreq[i].record_length > 124)
            return send_exception_msg(ModbusException::IllegalDataAddress);

        NMBS_DEBUG_PRINT("a %d\tr %d\tl %d\t fread ", subreq[i].file_number, subreq[i].record_number,
                            subreq[i].record_length);
    }

    push_res_header(1 + response_data_size);
    buf_.push_u8(response_data_size);


    for (uint8_t i = 0; i < subreq_count; i++) {
        uint16_t subreq_data_size = subreq[i].record_length * 2;
        buf_.push_u8(subreq_data_size + 1);
        buf_.push_u8(0x06);    // add Reference Type const
        uint16_t* subreq_data = (uint16_t*) buf_.get_n(subreq_data_size);

        const auto ev = NmbsEvents::ReadFileRecord{
            .file_number = subreq[i].file_number, 
            .record_number = subreq[i].record_number, 
            .reg = subreq_data,
            .count = subreq[i].record_length, 
            .unit_id = msg_status_.unit_id
        };
        res = callback(ev);
        if (res.is_err()) {
            if (res.is_err() and res.unwrap_err().is_exception())
                return send_exception_msg(res.unwrap_err().unwrap_as<ModbusException>());

            return send_exception_msg(ModbusException::ServerDeviceFailure);
        }

        swap_u16_endian(std::span(subreq_data, subreq[i].record_length));
    }


    if (msg_status_.broadcast == false) {
        res = send_msg();
        if (res.is_err())
            return res;
    }


    return Ok();
}

IResult<> NmbsServer::handle_write_file_record() {
    IResult<> res = recv(1);
    if (res.is_err())
        return res;

    const uint8_t request_size = buf_.pop_u8();
    if (request_size > 251) {
        return make_err(NmbsError::InvalidRequst);
    }

    res = recv(request_size);
    if (res.is_err())
        return res;

    // We can save msg_status_.buf_ index and use it later for context recovery.
    const uint16_t msg_buf_idx = buf_.size();
    buf_.discard_n(request_size);

    res = recv_msg_footer();
    if (res.is_err())
        return res;

    if (msg_status_.ignored) return recv_write_file_record_res(0, 0, nullptr, 0);
    const uint8_t subreq_header_size = 7;
    uint16_t size = request_size;
    buf_.buf_idx = msg_buf_idx;    // restore context

    if (request_size < 7)
        return send_exception_msg(ModbusException::IllegalDataValue);

    do {
        const uint8_t subreq_reference_type = buf_.pop_u8();
        const uint16_t subreq_file_number_c = buf_.pop_u16();
        const uint16_t subreq_record_number_c = buf_.pop_u16();
        const uint16_t subreq_record_length_c = buf_.pop_u16();
        buf_.discard_n(subreq_record_length_c * 2);

        if (subreq_reference_type != 0x06)
            return send_exception_msg(ModbusException::IllegalDataAddress);

        if (subreq_file_number_c == 0x0000)
            return send_exception_msg(ModbusException::IllegalDataAddress);

        if (subreq_record_number_c > 0x270F)
            return send_exception_msg(ModbusException::IllegalDataAddress);

        if (subreq_record_length_c > 122)
            return send_exception_msg(ModbusException::IllegalDataAddress);

        NMBS_DEBUG_PRINT("a %d\tr %d\tl %d\t fwrite ", subreq_file_number_c, subreq_record_number_c,
            subreq_record_length_c);
        size -= (subreq_header_size + subreq_record_length_c * 2);
    } while (size >= subreq_header_size);

    if (size)
        return send_exception_msg(ModbusException::IllegalDataValue);

    // checks completed

    size = request_size;
    buf_.buf_idx = msg_buf_idx;    // restore context

    do {
        buf_.push_zero();
        const uint16_t subreq_file_number = buf_.pop_u16();
        const uint16_t subreq_record_number = buf_.pop_u16();
        const uint16_t subreq_record_length = buf_.pop_u16();
        uint16_t* subreq_data = buf_.get_regs(subreq_record_length);


        const auto ev = NmbsEvents::WriteFileRecord{
            .file_number = subreq_file_number, 
            .record_number = subreq_record_number, 
            .reg = subreq_data,
            .count = subreq_record_length, 
            .unit_id = msg_status_.unit_id
        };
        res = callback(ev);
        if (res.is_err()) {
            if (res.is_err() and res.unwrap_err().is_exception())
                return send_exception_msg(res.unwrap_err().unwrap_as<ModbusException>());

            return send_exception_msg(ModbusException::ServerDeviceFailure);
        }

        swap_u16_endian(std::span(subreq_data, subreq_record_length));    // restore swapping

        size -= (subreq_header_size + subreq_record_length * 2);
    } while (size >= subreq_header_size);

    if (msg_status_.broadcast == false) {
        // The normal response to 'Write File' is an echo of the request.
        // We can restore buffer index and response msg_status_.
        buf_.buf_idx = msg_buf_idx;
        buf_.discard_n(request_size);

        res = send_msg();
        if (res.is_err())
            return res;
    }


    return Ok();
}

IResult<> NmbsServer::handle_read_write_registers() {
    IResult<> res = recv(9);
    if (res.is_err())
        return res;

    const uint16_t read_address = buf_.pop_u16();
    const uint16_t read_quantity = buf_.pop_u16();
    const uint16_t write_address = buf_.pop_u16();
    const uint16_t write_quantity = buf_.pop_u16();

    const uint8_t byte_count_write = buf_.pop_u8();

    NMBS_DEBUG_PRINT("ra %d\trq %d\t wa %d\t wq %d\t b %d\tregs ", read_address, read_quantity, write_address, write_quantity, byte_count_write);

    if (byte_count_write > 242)
        return make_err(NmbsError::InvalidRequst);

    res = recv(byte_count_write);
    if (res.is_err())
        return res;

#if defined(__STDC_NO_VLA__) || defined(_MSC_VER)
    uint16_t registers[0x007B];
#else
    uint16_t registers[byte_count_write / 2];
#endif
    for (size_t i = 0; i < byte_count_write / 2; i++) {
        registers[i] = buf_.pop_u16();
        NMBS_DEBUG_PRINT("%d ", registers[i]);
    }

    res = recv_msg_footer();
    if (res.is_err())
        return res;

    if (msg_status_.ignored) return recv_write_multiple_registers_res(write_address, write_quantity);
    if (read_quantity < 1 || read_quantity > 0x007D)
        return send_exception_msg(ModbusException::IllegalDataValue);

    if (write_quantity < 1 || write_quantity > 0x007B)
        return send_exception_msg(ModbusException::IllegalDataValue);

    if (byte_count_write != write_quantity * 2)
        return send_exception_msg(ModbusException::IllegalDataValue);

    if ((uint32_t) read_address + (uint32_t) read_quantity > ((uint32_t) 0xFFFF) + 1)
        return send_exception_msg(ModbusException::IllegalDataAddress);

    if ((uint32_t) write_address + (uint32_t) write_quantity > ((uint32_t) 0xFFFF) + 1)
        return send_exception_msg(ModbusException::IllegalDataAddress);

    res = callback(NmbsEvents::WriteMultipleRegisters{
        .address = write_address, 
        .quantity = write_quantity, 
        .registers = registers, 
        .unit_id = msg_status_.unit_id
    });
    if (res.is_err()) {
        if (res.is_err() and res.unwrap_err().is_exception())
            return send_exception_msg(res.unwrap_err().unwrap_as<ModbusException>());

        return send_exception_msg(ModbusException::ServerDeviceFailure);
    }

    if (msg_status_.broadcast == false) {
#if defined(__STDC_NO_VLA__) || defined(_MSC_VER)
        uint16_t regs[125];
#else
        uint16_t regs[read_quantity];
#endif
        res = callback(NmbsEvents::ReadHoldingRegisters{
            .address = read_address, 
            .quantity = read_quantity, 
            .registers_out = regs, 
            .unit_id = msg_status_.unit_id
        });
        if (res.is_err()) {
            if (res.is_err() and res.unwrap_err().is_exception())
                return send_exception_msg(res.unwrap_err().unwrap_as<ModbusException>());

            return send_exception_msg(ModbusException::ServerDeviceFailure);
        }

        const uint8_t regs_bytes = read_quantity * 2;
        push_res_header(1 + regs_bytes);

        buf_.push_u8(regs_bytes);

        NMBS_DEBUG_PRINT("b %d\t", regs_bytes);

        NMBS_DEBUG_PRINT("regs ");
        for (size_t i = 0; i < read_quantity; i++) {
            buf_.push_u16(regs[i]);
            NMBS_DEBUG_PRINT("%d ", regs[i]);
        }

        res = send_msg();
        if (res.is_err())
            return res;
    }

    return Ok();
}

IResult<> NmbsServer::handle_read_device_identification() {
    IResult<> res = recv(3);
    if (res.is_err())
        return res;

    const uint8_t mei_type = buf_.pop_u8();
    const uint8_t read_device_id_code = buf_.pop_u8();
    const uint8_t object_id = buf_.pop_u8();

    NMBS_DEBUG_PRINT("c %d\to %d", read_device_id_code, object_id);

    res = recv_msg_footer();
    if (res.is_err())
        return res;

    if (msg_status_.ignored) 
        return recv_read_device_identification_res(0, nullptr, 0, nullptr, nullptr, nullptr, nullptr);

    if (mei_type != 0x0E)
        return send_exception_msg(ModbusException::IllegalFunction);

    if (read_device_id_code < 1 || read_device_id_code > 4)
        return send_exception_msg(ModbusException::IllegalDataValue);

    if (object_id > 6 && object_id < 0x80)
        return send_exception_msg(ModbusException::IllegalDataAddress);

    if (msg_status_.broadcast == false) {
        char str[NMBS_DEVICE_IDENTIFICATION_STRING_LENGTH];

        nmbs_bitfield_256 map = nmbs_bitfield_256::from_zero();

        res = callback(NmbsEvents::ReadDeviceIdentificationMap{
            .map = map}
        );
        if (res.is_err()) {
            if (res.is_err() and res.unwrap_err().is_exception())
                return send_exception_msg(res.unwrap_err().unwrap_as<ModbusException>());

            return send_exception_msg(ModbusException::ServerDeviceFailure);
        }

        push_res_header(0);    // Length will be set later
        buf_.push_u8(0x0E);
        buf_.push_u8(read_device_id_code);
        buf_.push_u8(0x83);

        if (read_device_id_code == 4) {
            if (! bool(map[object_id]))
                return send_exception_msg(ModbusException::IllegalDataAddress);

            buf_.push_u8(0);    // More follows
            buf_.push_u8(0);    // Next Object Id
            buf_.push_u8(1);    // Number of objects

            str[0] = 0;
            res = callback(NmbsEvents::ReadDeviceIdentification{
                .object_id = object_id, 
                .buffer = str
            });
            if (res.is_err()) {
                if (res.is_err() and res.unwrap_err().is_exception())
                    return send_exception_msg(res.unwrap_err().unwrap_as<ModbusException>());

                return send_exception_msg(ModbusException::ServerDeviceFailure);
            }

            const size_t str_len = strlen(str);

            buf_.push_u8(object_id);    // Object id
            buf_.push_u8(str_len);      // Object length
            buf_.push_n((uint8_t*) str, str_len);

            set_msg_header_size(6 + 2 + str_len);

            return send_msg();
        }

        const uint8_t more_follows_idx = buf_.size();
        buf_.push_u8(0);
        const uint8_t next_object_id_idx = buf_.size();
        buf_.push_u8(0);
        const uint8_t number_of_objects_idx = buf_.size();
        buf_.push_u8(0);

        int16_t res_size_left = 253 - 7;

        uint8_t last_id = 0;
        uint8_t msg_size = 6;
        uint8_t res_more_follows = 0;
        uint8_t res_next_object_id = 0;
        uint8_t res_number_of_objects = 0;

        switch (read_device_id_code) {
            case 1:
                if (object_id > 0x02)
                    return send_exception_msg(ModbusException::IllegalDataAddress);
                last_id = 0x02;
                break;
            case 2:
                if (object_id < 0x03 || object_id > 0x06)
                    return send_exception_msg(ModbusException::IllegalDataAddress);
                last_id = 0x06;
                break;
            case 3:
                if (object_id < 0x80)
                    return send_exception_msg(ModbusException::IllegalDataAddress);
                last_id = 0xFF;
                break;
            default:
                // Unreachable
                break;
        }

        for (uint16_t id = object_id; id <= last_id; id++) {
            if (! bool(map[id])) {
                if (id < 0x03)
                    return send_exception_msg(ModbusException::ServerDeviceFailure);
                continue;
            }

            str[0] = 0;
            const auto ev = NmbsEvents::ReadDeviceIdentification{
                .object_id = (uint8_t) id, 
                .buffer = std::span(str)
            };
            res = callback(ev);
            if (res.is_err()) {
                if (res.is_err() and res.unwrap_err().is_exception())
                    return send_exception_msg(res.unwrap_err().unwrap_as<ModbusException>());

                return send_exception_msg(ModbusException::ServerDeviceFailure);
            }

            const int16_t str_len = (int16_t) strlen(str);

            res_size_left = (int16_t) (res_size_left - 2 - str_len);
            if (res_size_left < 0) {
                res_more_follows = 0xFF;
                res_next_object_id = id;
                break;
            }

            buf_.push_u8((uint8_t) id);    // Object id
            buf_.push_u8(str_len);         // Object length
            buf_.push_n((uint8_t*) str, str_len);

            msg_size += (2 + str_len);

            res_number_of_objects++;
        }

        buf_.set_u8(res_more_follows, more_follows_idx);
        buf_.set_u8(res_next_object_id, next_object_id_idx);
        buf_.set_u8(res_number_of_objects, number_of_objects_idx);

        set_msg_header_size(msg_size);

        return send_msg();
    }

    return Ok();
}




IResult<> NmbsServer::poll() {
    msg_state_reset();

    bool first_byte_received = false;
    IResult<> res = recv_req_header(&first_byte_received);
    if (res.is_err()) {
        if (!first_byte_received && (res.unwrap_err() == NmbsError::Timeout))
            return Ok();

        return res;
    }

#ifdef NMBS_DEBUG_EN
    printf("%d ", address_rtu);
    printf("NMBS req <- ");
    if (transport_ == NmbsTransport::RTU) {
        if (msg_status_.broadcast)
            printf("broadcast\t");
        else
            printf("address_rtu %d\t", msg_status_.unit_id);
    }
#endif

    res = handle_req_fc();
    if (res.is_err()) {
        if (res.unwrap_err() != NmbsError::Timeout)
            flush();

        return res;
    }

    return Ok();
}






IResult<> NmbsClient::read_coils(uint16_t address, uint16_t quantity, nmbs_bitfield coils_out) {
    return read_discrete(FunctionCode::ReadCoils, address, quantity, coils_out);
}


IResult<> NmbsClient::read_discrete_inputs(uint16_t address, uint16_t quantity, nmbs_bitfield inputs_out) {
    return read_discrete(FunctionCode::ReadDiscreteInputs, address, quantity, inputs_out);
}



IResult<> NmbsClient::read_holding_registers(uint16_t address, uint16_t quantity, uint16_t* registers_out) {
    return read_registers(FunctionCode::ReadHoldingRegisters, address, quantity, registers_out);
}


IResult<> NmbsClient::read_input_registers(uint16_t address, uint16_t quantity, uint16_t* registers_out) {
    return read_registers(FunctionCode::ReadInputRegisters, address, quantity, registers_out);
}


IResult<> NmbsClient::write_single_coil(uint16_t address, bool value) {
    msg_state_req(FunctionCode::WriteSingleCoil);
    push_req_header(4);

    const uint16_t value_req = value ? 0xFF00 : 0;

    buf_.push_u16(address);
    buf_.push_u16(value_req);

    NMBS_DEBUG_PRINT("a %d\tvalue %d ", address, value_req);

    if(const auto res = send_msg();
        res.is_err()) return res;

    if (msg_status_.broadcast == false)
        return recv_write_single_coil_res(address, value_req);

    return Ok();
}


IResult<> NmbsClient::write_single_register(uint16_t address, uint16_t value) {
    msg_state_req(FunctionCode::WriteSingleRegister);
    push_req_header(4);

    buf_.push_u16(address);
    buf_.push_u16(value);

    NMBS_DEBUG_PRINT("a %d\tvalue %d", address, value);

    if(const auto res = send_msg();
        res.is_err()) return res;

    if (msg_status_.broadcast == false)
        return recv_write_single_register_res(address, value);

    return Ok();
}


IResult<> NmbsClient::write_multiple_coils(uint16_t address, uint16_t quantity, const nmbs_bitfield coils) {
    if (quantity < 1 || quantity > 0x07B0)
        return make_err(NmbsError::InvalidArgument);

    if ((uint32_t) address + (uint32_t) quantity > ((uint32_t) 0xFFFF) + 1)
        return make_err(NmbsError::InvalidArgument);

    uint8_t coils_bytes = (quantity + 7) / 8;

    msg_state_req(FunctionCode::WriteMultipleCoils);
    push_req_header(5 + coils_bytes);

    buf_.push_u16(address);
    buf_.push_u16(quantity);
    buf_.push_u8(coils_bytes);
    NMBS_DEBUG_PRINT("a %d\tq %d\tb %d\t", address, quantity, coils_bytes);

    NMBS_DEBUG_PRINT("coils ");
    for (size_t i = 0; i < coils_bytes; i++) {
        buf_.push_u8(coils[i]);
        NMBS_DEBUG_PRINT("%d ", coils[i]);
    }

    if(const auto res = send_msg();
        res.is_err()) return res;

    if (msg_status_.broadcast == false)
        return recv_write_multiple_coils_res(address, quantity);

    return Ok();
}


IResult<> NmbsClient::write_multiple_registers(uint16_t address, uint16_t quantity, const uint16_t* registers) {
    if (quantity < 1 || quantity > 0x007B)
        return make_err(NmbsError::InvalidArgument);

    if ((uint32_t) address + (uint32_t) quantity > ((uint32_t) 0xFFFF) + 1)
        return make_err(NmbsError::InvalidArgument);

    const uint8_t registers_bytes = quantity * 2;

    msg_state_req(FunctionCode::WriteMultipleRegisters);
    push_req_header(5 + registers_bytes);

    buf_.push_u16(address);
    buf_.push_u16(quantity);
    buf_.push_u8(registers_bytes);
    NMBS_DEBUG_PRINT("a %d\tq %d\tb %d\t", address, quantity, registers_bytes);

    NMBS_DEBUG_PRINT("regs ");
    for (size_t i = 0; i < quantity; i++) {
        buf_.push_u16(registers[i]);
        NMBS_DEBUG_PRINT("%d ", registers[i]);
    }

    if(const auto res = send_msg();
        res.is_err()) return res;

    if (msg_status_.broadcast == false)
        return recv_write_multiple_registers_res(address, quantity);

    return Ok();
}


IResult<> NmbsClient::read_file_record(
    uint16_t file_number, uint16_t record_number, uint16_t* registers,
    uint16_t count
) {
    if (file_number == 0x0000)
        return make_err(NmbsError::InvalidArgument);

    if (record_number > 0x270F)
        return make_err(NmbsError::InvalidArgument);

    // In expected response: max PDU length = 253, assuming a single file request, (253 - 1 - 1 - 1 - 1) / 2 = 124
    if (count > 124)
        return make_err(NmbsError::InvalidArgument);

    msg_state_req(FunctionCode::ReadFileRecord);
    push_req_header(8);

    buf_.push_u8(7);    // add Byte Count
    buf_.push_u8(6);    // add Reference Type const
    buf_.push_u16(file_number);
    buf_.push_u16(record_number);
    buf_.push_u16(count);
    NMBS_DEBUG_PRINT("a %d\tr %d\tl %d\t fread ", file_number, record_number, count);

    if(const auto res = send_msg();
        res.is_err()) return res;

    return recv_read_file_record_res(registers, count);
}


IResult<> NmbsClient::write_file_record(
    
    uint16_t file_number, 
    uint16_t record_number, 
    const uint16_t* registers,
    uint16_t count
) {
    if (file_number == 0x0000)
        return make_err(NmbsError::InvalidArgument);

    if (record_number > 0x270F)
        return make_err(NmbsError::InvalidArgument);

    if (count > 122)
        return make_err(NmbsError::InvalidArgument);

    const uint16_t data_size = count * 2;

    msg_state_req(FunctionCode::WriteFileRecord);
    push_req_header(8 + data_size);

    buf_.push_u8(7 + data_size);    // add Byte Count
    buf_.push_u8(6);                // add Reference Type const
    buf_.push_u16(file_number);
    buf_.push_u16(record_number);
    buf_.push_u16(count);
    buf_.push_regs(registers, count);
    NMBS_DEBUG_PRINT("a %d\tr %d\tl %d\t fwrite ", file_number, record_number, count);

    if(const auto res = send_msg();
        res.is_err()) return res;

    if (msg_status_.broadcast == false)
        return recv_write_file_record_res(file_number, record_number, registers, count);

    return Ok();
}


IResult<> NmbsClient::read_write_registers(uint16_t read_address, uint16_t read_quantity,
    uint16_t* registers_out, uint16_t write_address, uint16_t write_quantity,
    const uint16_t* registers
) {
    if (read_quantity < 1 || read_quantity > 0x007D)
        return make_err(NmbsError::InvalidArgument);

    if ((uint32_t) read_address + (uint32_t) read_quantity > ((uint32_t) 0xFFFF) + 1)
        return make_err(NmbsError::InvalidArgument);

    if (write_quantity < 1 || write_quantity > 0x0079)
        return make_err(NmbsError::InvalidArgument);

    if ((uint32_t) write_address + (uint32_t) write_quantity > ((uint32_t) 0xFFFF) + 1)
        return make_err(NmbsError::InvalidArgument);

    const uint8_t registers_bytes = write_quantity * 2;

    msg_state_req(FunctionCode::ReadWriteRegisters);
    push_req_header(9 + registers_bytes);

    buf_.push_u16(read_address);
    buf_.push_u16(read_quantity);
    buf_.push_u16(write_address);
    buf_.push_u16(write_quantity);
    buf_.push_u8(registers_bytes);

    NMBS_DEBUG_PRINT("read a %d\tq %d ", read_address, read_quantity);
    NMBS_DEBUG_PRINT("write a %d\tq %d\tb %d\t", write_address, write_quantity, registers_bytes);

    NMBS_DEBUG_PRINT("regs ");
    for (size_t i = 0; i < write_quantity; i++) {
        buf_.push_u16(registers[i]);
        NMBS_DEBUG_PRINT("%d ", registers[i]);
    }

    if(const auto res = send_msg();
        res.is_err()) return res;

    return recv_read_registers_res(read_quantity, registers_out);
}


IResult<> NmbsClient::read_device_identification_basic(char* vendor_name, char* product_code,
                                                 char* major_minor_revision, uint8_t buffers_length) {
    const uint8_t order[3] = {0, 1, 2};
    char* buffers[3] = {vendor_name, product_code, major_minor_revision};
    uint8_t total_received = 0;
    uint8_t next_object_id = 0x00;

    while (next_object_id != 0x7F) {
        msg_state_req(FunctionCode::ReadDeviceIdentification);
        push_msg_header(3);
        buf_.push_u8(0x0E);
        buf_.push_u8(1);
        buf_.push_u8(next_object_id);

        IResult<> res = send_msg();
        if (res.is_err())
            return res;

        uint8_t objects_received = 0;
        res = recv_read_device_identification_res(3, buffers, buffers_length, order, nullptr, &next_object_id,
                                                  &objects_received);
        if (res.is_err())
            return res;

        total_received += objects_received;
        if (total_received > 3)
            return make_err(NmbsError::InvalidResponse);

        if (objects_received == 0)
            return make_err(NmbsError::InvalidResponse);
    }

    return Ok();
}


IResult<> NmbsClient::read_device_identification_regular(char* vendor_url, char* product_name, char* model_name,
                                                   char* user_application_name, uint8_t buffers_length) {
    const uint8_t order[7] = {0, 0, 0, 0, 1, 2, 3};
    char* buffers[4] = {vendor_url, product_name, model_name, user_application_name};
    uint8_t total_received = 0;
    uint8_t next_object_id = 0x03;

    while (next_object_id != 0x7F) {
        msg_state_req(FunctionCode::ReadDeviceIdentification);
        push_req_header(3);
        buf_.push_u8(0x0E);
        buf_.push_u8(2);
        buf_.push_u8(next_object_id);

        IResult<> res = send_msg();
        if (res.is_err())
            return res;

        uint8_t objects_received = 0;
        res = recv_read_device_identification_res(4, buffers, buffers_length, order, nullptr, &next_object_id,
                                                  &objects_received);
        if (res.is_err())
            return res;

        total_received += objects_received;
        if (total_received > 4)
            return make_err(NmbsError::InvalidResponse);

        if (objects_received == 0)
            return make_err(NmbsError::InvalidResponse);
    }

    return Ok();
}


IResult<> NmbsClient::read_device_identification_extended(uint8_t object_id_start, uint8_t* ids, char** buffers,
                                                    uint8_t ids_length, uint8_t buffer_length,
                                                    uint8_t* objects_count_out) {
    if (object_id_start < 0x80)
        return make_err(NmbsError::InvalidArgument);

    uint8_t total_received = 0;
    uint8_t next_object_id = object_id_start;

    while (next_object_id != 0x7F) {
        msg_state_req(FunctionCode::ReadDeviceIdentification);
        push_req_header(3);
        buf_.push_u8(0x0E);
        buf_.push_u8(3);
        buf_.push_u8(next_object_id);

        IResult<> res = send_msg();
        if (res.is_err())
            return res;

        uint8_t objects_received = 0;
        res = recv_read_device_identification_res(
            ids_length - total_received, &buffers[total_received],
            buffer_length, nullptr, &ids[total_received], &next_object_id,
            &objects_received);
        if (res.is_err())
            return res;

        total_received += objects_received;
    }

    *objects_count_out = total_received;

    return Ok();
}


IResult<> NmbsClient::read_device_identification(uint8_t object_id, char* buffer, uint8_t buffer_length) {
    if (object_id > 0x06 && object_id < 0x80)
        return make_err(NmbsError::InvalidArgument);

    msg_state_req(FunctionCode::ReadDeviceIdentification);
    push_req_header(3);
    buf_.push_u8(0x0E);
    buf_.push_u8(4);
    buf_.push_u8(object_id);

    if(const auto res = send_msg();
        res.is_err()) return res;

    char* _buf[1] = {buffer};
    return recv_read_device_identification_res(1, _buf, buffer_length, nullptr, nullptr, nullptr, nullptr);
}


IResult<> NmbsClient::send_raw_pdu(FunctionCode fc, const uint8_t* data, uint16_t data_len) {
    msg_state_req(fc);
    push_msg_header(data_len);

    NMBS_DEBUG_PRINT("raw ");
    for (uint16_t i = 0; i < data_len; i++) {
        buf_.push_u8(data[i]);
        NMBS_DEBUG_PRINT("%d ", data[i]);
    }

    return send_msg();
}


IResult<> NmbsClient::receive_raw_pdu_response(uint8_t* data_out, uint8_t data_out_len) {
    IResult<> res = recv_res_header();
    if (res.is_err())
        return res;

    res = recv(data_out_len);
    if (res.is_err())
        return res;

    if (data_out) {
        for (uint16_t i = 0; i < data_out_len; i++)
            data_out[i] = buf_.pop_u8();
    }
    else {
        for (uint16_t i = 0; i < data_out_len; i++)
            (void)buf_.pop_u8();
    }

    res = recv_msg_footer();
    if (res.is_err())
        return res;

    return Ok();
}


IResult<> NmbsServer::handle_req_fc() {

    NMBS_DEBUG_PRINT("fc %d\t", msg_status_.fc);

    switch (msg_status_.fc) {
#ifndef NMBS_SERVER_READ_COILS_DISABLED
        case FunctionCode::ReadCoils:
            return handle_read_coils();
#endif

#ifndef NMBS_SERVER_READ_DISCRETE_inputS_DISABLED
        case FunctionCode::ReadDiscreteInputs:
            return handle_read_discrete_inputs();
#endif

#ifndef NMBS_SERVER_READ_HOLDING_REGISTERS_DISABLED
        case FunctionCode::ReadHoldingRegisters:
            return handle_read_holding_registers();
#endif

#ifndef NMBS_SERVER_READ_input_REGISTERS_DISABLED
        case FunctionCode::ReadInputRegisters:
            return handle_read_input_registers();
#endif

#ifndef NMBS_SERVER_WRITE_SINGLE_COIL_DISABLED
        case FunctionCode::WriteSingleCoil:
            return handle_write_single_coil();
#endif

#ifndef NMBS_SERVER_WRITE_SINGLE_REGISTER_DISABLED
        case FunctionCode::WriteSingleRegister:
            return handle_write_single_register();
#endif

#ifndef NMBS_SERVER_WRITE_MULTIPLE_COILS_DISABLED
        case FunctionCode::WriteMultipleCoils:
            return handle_write_multiple_coils();
#endif

#ifndef NMBS_SERVER_WRITE_MULTIPLE_REGISTERS_DISABLED
        case FunctionCode::WriteMultipleRegisters:
            return handle_write_multiple_registers();
#endif

#ifndef NMBS_SERVER_READ_FILE_RECORD_DISABLED
        case FunctionCode::ReadFileRecord:
            return handle_read_file_record();
#endif

#ifndef NMBS_SERVER_WRITE_FILE_RECORD_DISABLED
        case FunctionCode::WriteFileRecord:
            return handle_write_file_record();
#endif

#ifndef NMBS_SERVER_READ_WRITE_REGISTERS_DISABLED
        case FunctionCode::ReadWriteRegisters:
            return handle_read_write_registers();
#endif

#ifndef NMBS_SERVER_READ_DEVICE_IDENTIFICATION_DISABLED
        case FunctionCode::ReadDeviceIdentification:
            return handle_read_device_identification();
#endif
        default:
            flush();
            if (!msg_status_.ignored)
                return send_exception_msg(ModbusException::IllegalFunction);
    }
    return Ok();
}


