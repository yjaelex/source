
#include "AllMP4Box.h"


MP4VmhdBox::MP4VmhdBox(MP4FileClass &file)
        : MP4Box(file, "vmhd")
{
    AddVersionAndFlags();
    AddReserved(*this, "reserved", 8);
}

void MP4VmhdBox::Generate()
{
    MP4Box::Generate();

    SetFlags(1);
}
