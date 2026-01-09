#pragma once

#include <cstdint>

namespace ymd::file::wav{

enum class AudioFormat:uint16_t {
    PCM = 1

};
struct [[nodiscard]] FileDescriptor{
    char     chunk_id[4]; //内容为"RIFF"
    uint32_t chunk_size;  //存储文件的字节数（不包含ChunkID和ChunkSize这8个字节）
    char     format[4];  //内容为"WAVE“

    [[nodiscard]] constexpr bool is_valid() const{
        return chunk_id[0] == 'R' && chunk_id[1] == 'I' && chunk_id[2] == 'F' && chunk_id[3] == 'F' &&
            format[0] == 'W' && format[1] == 'A' && format[2] == 'V' && format[3] == 'E';
    }
};

struct [[nodiscard]] FileFormatSubchunk{
   char     subchunk1_id[4]; //内容为"fmt"
   uint32_t subchunk1_size;  //存储该子块的字节数（不含前面的Subchunk1ID和Subchunk1Size这8个字节）
   AudioFormat audio_format;    //存储音频文件的编码格式，例如若为PCM则其存储值为1。
   uint16_t num_channels;    //声道数，单声道(Mono)值为1，双声道(Stereo)值为2，等等
   uint32_t fs;     //采样率，如8k，44.1k等
   uint32_t byte_rate;       //每秒存储的bit数，其值 = SampleRate * NumChannels * BitsPerSample / 8
   uint16_t block_align;     //块对齐大小，其值 = NumChannels * BitsPerSample / 8
   uint16_t bits_per_sample;  //每个采样点的bit数，一般为8,16,32等。

    [[nodiscard]] constexpr bool is_valid() const{
        return subchunk1_id[0] == 'f' && subchunk1_id[1] == 'm' && subchunk1_id[2] == 't';
    }
};

struct [[nodiscard]] FileDataSubchunk{
    char     subchunk2_id[4]; //内容为“data”

    //接下来的正式的数据部分的字节数，其值 == NumSamples * NumChannels * BitsPerSample / 8
    uint32_t subchunk2_size;  

    [[nodiscard]] constexpr bool is_valid() const{
        return subchunk2_id[0] == 'd' && subchunk2_id[1] == 'a' && subchunk2_id[2] == 't' && subchunk2_id[3] == 'a';
    }
};
}
