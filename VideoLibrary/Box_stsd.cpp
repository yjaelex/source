#include "AllMP4Box.h"

MP4StsdBox::MP4StsdBox(MP4FileClass &file)
        : MP4FullBox(file, "stsd")
{
    m_entryCount = 0;

    ExpectChildBox("mp4a", Optional, Many);
    ExpectChildBox("enca", Optional, Many);
    ExpectChildBox("mp4s", Optional, Many);
    ExpectChildBox("mp4v", Optional, Many);
    ExpectChildBox("encv", Optional, Many);
    ExpectChildBox("rtp ", Optional, Many);
    ExpectChildBox("samr", Optional, Many); // For AMR-NB
    ExpectChildBox("sawb", Optional, Many); // For AMR-WB
    ExpectChildBox("s263", Optional, Many); // For H.263
    ExpectChildBox("avc1", Optional, Many);
    ExpectChildBox("alac", Optional, Many);
    ExpectChildBox("text", Optional, Many);
    ExpectChildBox("ac-3", Optional, Many);
}

void MP4StsdBox::ReadProperties()
{
	// read version and flags */
	MP4FullBox::ReadProperties();
	m_entryCount = m_File.ReadUInt32();
}

void MP4StsdBox::DumpProperties(uint8_t indent, bool dumpImplicits)
{
	MP4FullBox::DumpProperties(indent, dumpImplicits);

	osDump(indent, "Entry Count: %d(0x%x)\n", m_entryCount, m_entryCount);
}

void MP4StsdBox::Read()
{
    /* do the usual read */
    MP4Box::Read();

    // check that number of children == entryCount
	if (m_vChildBoxs.size() != m_entryCount)
	{
        osLog(LOG_ERROR, "%s: \"%s\": stsd inconsistency with number of entries",
                     __FUNCTION__, GetFile().GetFilename().c_str() );

        /* fix it */
		m_entryCount = (m_vChildBoxs.size());
    }
}
