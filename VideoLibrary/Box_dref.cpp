#include "AllMP4Box.h"

MP4DrefBox::MP4DrefBox(MP4FileClass &file)
        : MP4FullBox(file, "dref")
{
    m_entry_count = 0;

    ExpectChildBox("url ", Optional, Many);
    ExpectChildBox("urn ", Optional, Many);
    ExpectChildBox("alis", Optional, Many);
}

void MP4DrefBox::ReadProperties()
{
    MP4FullBox::ReadProperties();
    m_entry_count = m_File.ReadUInt32();
}

void MP4DrefBox::Read()
{
    /* do the usual read */
    MP4Box::Read();

    // check that number of children == entryCount

    if (m_vChildBoxs.size() != m_entry_count)
    {
        osLog(LOG_WARN, "%s: \"%s\": dref inconsistency with number of entries",
                     __FUNCTION__, GetFile().GetFilename().c_str() );

        m_entry_count = m_vChildBoxs.size();
    }
}

void MP4DrefBox::DumpProperties(uint8_t indent, bool dumpImplicits)
{
    MP4FullBox::DumpProperties(indent, dumpImplicits);
    osDump(indent, "Entry Count: %d(0x%x)\n", m_entry_count, m_entry_count);
}
