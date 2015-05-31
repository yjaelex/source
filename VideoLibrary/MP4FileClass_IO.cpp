#include "MP4FileClass.h"
#include "os.h"

uint32_t MP4FileClass::ReadMpegLength()
{
    uint32_t length = 0;
    uint8_t numBytes = 0;
    uint8_t b;

    do {
        b = ReadUInt8();
        length = (length << 7) | (b & 0x7F);
        numBytes++;
    } while ((b & 0x80) && numBytes < 4);

    return length;
}

void MP4FileClass::WriteMpegLength(uint32_t value, bool compact)
{
    if (value > 0x0FFFFFFF) {
		osAssert(!"out of range: "); 
    }

    int8_t numBytes;

    if (compact) {
        if (value <= 0x7F) {
            numBytes = 1;
        } else if (value <= 0x3FFF) {
            numBytes = 2;
        } else if (value <= 0x1FFFFF) {
            numBytes = 3;
        } else {
            numBytes = 4;
        }
    } else {
        numBytes = 4;
    }

    int8_t i = numBytes;
    do {
        i--;
        uint8_t b = (value >> (i * 7)) & 0x7F;
        if (i > 0) {
            b |= 0x80;
        }
        WriteUInt8(b);
    } while (i > 0);
}


