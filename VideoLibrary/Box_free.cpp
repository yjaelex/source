#include "AllMP4Box.h"


MP4FreeBox::MP4FreeBox( MP4FileClass &file, const char* type )
    : MP4Box( file, type ? type : "free" )
{
}

void MP4FreeBox::Read()
{
    Skip();
}

void MP4FreeBox::Write()
{
    bool use64 = (GetSize() > (0xFFFFFFFF - 8));
    BeginWrite(use64);
#if 1
    for (uint64_t ix = 0; ix < GetSize(); ix++) {
        m_File.WriteUInt8(0);
    }
#else
    m_File.SetPosition(m_File.GetPosition() + GetSize());
#endif
    FinishWrite(use64);
}
