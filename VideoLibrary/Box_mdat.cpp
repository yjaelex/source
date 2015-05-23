

#include "AllMP4Box.h"





MP4MdatBox::MP4MdatBox(MP4FileClass &file)
        : MP4Box(file, "mdat")
{
}

void MP4MdatBox::Read()
{
    Skip();
}

void MP4MdatBox::Write()
{
    // should never get here
    ASSERT(false);
}



}
} // namespace mp4v2::impl
