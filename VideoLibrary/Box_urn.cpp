#include "AllMP4Box.h"

MP4UrnBox::MP4UrnBox(MP4FileClass &file)
        : MP4FullBox(file, "urn ")
{

}

void MP4UrnBox::Read()
{
    // read version and flags */
    MP4FullBox::ReadProperties();
    m_location.clear();
    m_name.clear();

    // check if location is present
    if (m_File.GetPosition() < GetEnd())
    {
        // read a null terminated string
        char * pStr = m_File.ReadString();
        osAssert(pStr);
        if (pStr)
        {
            m_name.append(pStr);
        }
        m_File.FreeString(pStr);
        pStr = m_File.ReadString();
        osAssert(pStr);
        if (pStr)
        {
            m_location.append(pStr);
        }
        m_File.FreeString(pStr);
    }

    Skip(); // to end of atom
}
