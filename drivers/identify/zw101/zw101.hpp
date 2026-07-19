#pragma once

#include <cstdint>
#include <span>


namespace ymd::drivers::zw101{

enum class PacketType:uint8_t{
    CommandPacket,
    DataPacket,
    LastPacket
};


// 确认码定义：
enum class [[nodiscard]] ConfirmCode : uint8_t {
    // 00H：表示指令执行完毕或 OK； 
    Ok = 0x00,

    // 01H：表示数据包接收错误； 
    PacketReceiveFailed = 0x01,

    // 02H：表示传感器上没有手指； 
    NoFinger = 0x02,

    // 03H：表示录入指纹图像失败；
    FailedRegisterFinger = 0x03,

    // 04H：表示指纹图像太干、太淡而生不成特征； 
    ImageTooDry = 0x04,

    // 05H：表示指纹图像太湿、太糊而生不成特征； 
    ImageTooWet = 0x05,

    // 06H：表示指纹图像太乱而生不成特征； 
    ImageTooMessy = 0x06,

    // 07H：表示指纹图像正常，但特征点太少（或面积太小）而生不成特征； 
    NotEnoughFeatures = 0x07,

    // 08H：表示指纹不匹配； 
    FingerMismatch = 0x08,

    // 09H：表示没搜索到指纹； 
    FingerNotFound = 0x09,

    // 0aH：表示特征合并失败； 
    MergeFeaturesFailed = 0x0A,

    // 0bH：表示访问指纹库时地址序号超出指纹库范围； 
    AddressOutOfRange = 0x0B,

    // 0cH：表示从指纹库读模板出错或无效； 
    ReadTemplateFailed = 0x0C,

    // 0dH：表示上传特征失败； 
    UploadFeaturesFailed = 0x0D,

    // 0eH：表示模组不能接收后续数据包； 
    CannotReceiveMore = 0x0E,

    // 0fH：表示上传图像失败； 
    UploadImageFailed = 0x0F,

    // 10H：表示删除模板失败； 
    DeleteTemplateFailed = 0x10,

    // 11H：表示清空指纹库失败； 
    ClearDatabaseFailed = 0x11,

    // 12H：表示不能进入低功耗状态； 
    EnterLowPowerFailed = 0x12,

    // 13H：表示口令不正确； 
    InvalidPassword = 0x13,

    // 14H：表示系统复位失败； 
    ResetSystemFailed = 0x14,

    // 15H：表示缓冲区内没有有效原始图而生不成图像； 
    NoValidImageInBuffer = 0x15,

    // 17H：表示残留指纹或两次采集之间手指没有移动过；
    ResidualFingerOrNoMove = 0x17,

    // 18H：表示读写 FLASH 出错； 
    FlashReadWriteError = 0x18,

    // 1aH：无效寄存器号； 
    InvalidRegisterNumber = 0x1A,

    // 1bH：寄存器设定内容错误号； 
    InvalidRegisterValue = 0x1B,

    // 1cH：记事本页码指定错误； 
    InvalidNotebookPage = 0x1C,

    // 1dH：端口操作失败；  
    PortOperationFailed = 0x1D,

    // 1eH：自动注册（enroll）失败； 
    AutoEnrollFailed = 0x1E,

    // 1fH：指纹库满； 
    DatabaseFull = 0x1F,

    // 20H：设备地址错误； 
    InvalidDeviceAddress = 0x20,

    // 21H：密码有误； 
    WrongPassword = 0x21,

    // 22 H：指纹模板非空； 
    TemplateNotEmpty = 0x22,

    // 23 H：指纹模板为空； 
    TemplateEmpty = 0x23,

    // 24 H：指纹库为空； 
    DatabaseEmpty = 0x24,

    // 25 H：录入次数设置错误； 
    InvalidEnrollCount = 0x25,

    // 26 H：超时； 
    Timeout = 0x26,

    // 27 H：指纹已存在； 
    FingerAlreadyExists = 0x27,

    // 28 H：指纹模板有关联； 
    TemplateHasAssociation = 0x28,

    // 29 H：传感器初始化失败； 
    SensorInitFailed = 0x29,

    // 2AH：模组信息非空； 
    ModuleInfoNotEmpty = 0x2A,

    // 2BH：模组信息为空； 
    ModuleInfoEmpty = 0x2B,

    // 33H：图像面积小 
    ImageAreaTooSmall = 0x33,

    // 34H：图像不可用 
    ImageUnavailable = 0x34,

    // 35H：非法数据 
    InvalidData = 0x35,

    // 40H：注册次数少于规定次数 
    InsufficientEnrollCount = 0x40,
};


struct [[nodiscard]] ChecksumBuilder final{
    static constexpr ChecksumBuilder from_default(){
        ChecksumBuilder self;
        self.checksum = 0;
        return self;
    }

    constexpr ChecksumBuilder push_bytes(std::span<const uint8_t> bytes) const noexcept{
        auto self = *this;
        const size_t len = bytes.size();
        for(size_t i = 0; i < len; i++){
            self.checksum += bytes[i];
        }
        return self;
    }

    constexpr ChecksumBuilder push_byte(const uint8_t byte) const noexcept{
        auto self = *this;
        self.checksum += byte;
        return self;
    }
private:
    uint16_t checksum;
};

using CommandId = uint8_t;


namespace req_msgs{

struct GetImage{
    static constexpr auto COMMAND_ID = CommandId{0x01};
};

struct GenerateCharaistics{
    static constexpr auto COMMAND_ID = CommandId{0x02};
};

struct MatchCharaistics{
    static constexpr auto COMMAND_ID = CommandId{0x03};
};

struct Search{
    static constexpr auto COMMAND_ID = CommandId{0x04};
};


}

}