#pragma once

#include "sys/stream/ostream.hpp"
#include "sys/string/string.hpp"

class StringStream:public OutputStream{
protected:
    String str_;
public:
    StringStream(){;}

    void write(const char data) override;
    void write(const char * data_ptr, const size_t len) override;
    size_t pending() const override;

    operator String() const;
    operator StringView() const;
};
