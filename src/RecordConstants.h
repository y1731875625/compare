#ifndef RECORDCONSTANTS_H
#define RECORDCONSTANTS_H

#include <QtGlobal>

namespace VCDU {
    constexpr int RECORD_SIZE = 892;
    constexpr int VC0_OFFSET = 0;
    constexpr int VC0_SIZE = 2;
    constexpr int FRAME_OFFSET = 2;
    constexpr int FRAME_SIZE = 3;
    constexpr int SIGNAL_OFFSET = 5;
    constexpr int RESERVED_OFFSET = 6;
    constexpr int RESERVED_SIZE = 32;
    constexpr int DATA_OFFSET = 38;
    constexpr int DATA_SIZE = 852;
    constexpr int CRC_OFFSET = 890;
    constexpr int CRC_SIZE = 2;

    constexpr quint16 CRC_INIT = 0xFFFF;
    constexpr quint16 CRC_XOROUT = 0xFFFF;
    constexpr quint16 CRC_POLY = 0x1021;
}

#endif // RECORDCONSTANTS_H