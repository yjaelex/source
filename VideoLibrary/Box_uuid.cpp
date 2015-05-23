
#include "AllMP4Box.h"


IPodUUIDBox::IPodUUIDBox(MP4FileClass &file)
        : MP4Box(file, "uuid")
{
    //
    // This is a hack, the contents of this atom need to be well defined.
    //
    static uint8_t ipod_magic[] = {
        0x6b, 0x68, 0x40, 0xf2, 0x5f, 0x24, 0x4f, 0xc5,
        0xba, 0x39, 0xa5, 0x1b, 0xcf, 0x03, 0x23, 0xf3
    };

    SetExtendedType(ipod_magic);

    value = (1);
}

