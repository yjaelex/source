#include "AllMP4Box.h"


MP4VmhdBox::MP4VmhdBox(MP4FileClass &file)
        : MP4FullBox(file, "vmhd")
{

}

void MP4VmhdBox::Generate()
{
    MP4Box::Generate();

    SetFlags(1);
}
