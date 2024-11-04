#pragma once


namespace yumud{

enum class CommMethod:unsigned char{
    None = 0,
    Blocking,
    Interrupt,
    Dma,
};

enum class CommMode:unsigned char{
    RxOnly = 1, TxOnly, TxRx = TxOnly | RxOnly
};

}

