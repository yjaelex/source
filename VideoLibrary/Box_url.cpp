#include "AllMP4Box.h"

MP4UrlBox::MP4UrlBox(MP4FileClass &file, const char *type)
        : MP4FullBox(file, type)
{
    m_location.clear();
}

void MP4UrlBox::Read()
{
    // read version and flags */
    MP4FullBox::ReadProperties();
    m_location.clear();

    // check if self-contained flag is set
    if (!(GetFlags() & 1))
    {
        // if not then read url location
        // read a null terminated string
        char * pStr = m_File.ReadString();
        osAssert(pStr);
        if (pStr)
        {
            m_location.append(pStr);
        }
        m_File.FreeString(pStr);
    }

    Skip(); // to end of atom
}

void MP4UrlBox::Write()
{
    // if no url location has been set
    // then set self-contained flag
    // and don't attempt to write anything
    if (m_location.empty())
    {
        SetFlags(GetFlags() | 1);
    } else 
    {
        SetFlags(GetFlags() & 0xFFFFFE);
    }

    // write atom as usual
    MP4Box::Write();
}

