#include "AllMP4Box.h"


MP4MdhdBox::MP4MdhdBox(MP4FileClass &file)
        : MP4FullBox(file, "mdhd")
{
	m_creationTime = 0;
	m_modificationTime = 0;
	m_timescale = 0;
	m_duration = 0;
	m_language = m_pre_defined = 0;
	m_code[0] = m_code[1] = m_code[2] = 0;
}

void MP4MdhdBox::Generate()
{
    uint8_t version = m_File.Use64Bits(GetType()) ? 1 : 0;
    SetVersion(version);

    MP4FullBox::Generate();

	MP4Timestamp now = MP4GetAbsTimestamp();
	m_creationTime = now;
	m_modificationTime = now;
	m_timescale = 1000;
}

void MP4MdhdBox::ReadProperties()
{
	// read version and flags */
	MP4FullBox::ReadProperties();

	if (m_version == 1)
	{
		m_creationTime = m_File.ReadUInt64();
		m_modificationTime = m_File.ReadUInt64();
		m_timescale = m_File.ReadUInt32();
		m_duration = m_File.ReadUInt64();
	}
	else
	{	// m_version==0
		m_creationTime = m_File.ReadUInt32();
		m_modificationTime = m_File.ReadUInt32();
		m_timescale = m_File.ReadUInt32();
		m_duration = m_File.ReadUInt32();
	}

	uint16_t data = m_File.ReadBits(16);
	m_language = data;
	m_code[0] = ((data & 0x7c00) >> 10) + 0x60;
	m_code[1] = ((data & 0x03e0) >> 5) + 0x60;
	m_code[2] = ((data & 0x001f)) + 0x60;
}

void MP4MdhdBox::DumpProperties(uint8_t indent, bool dumpImplicits)
{
	MP4FullBox::DumpProperties(indent, dumpImplicits);

	if (m_version == 1)
	{
		osDump(indent, "Creation Time: %"PRIu64"(0x%"PRIx64")\n", m_creationTime, m_creationTime);
		osDump(indent, "Modification Time: %"PRIu64"(0x%"PRIx64")\n", m_modificationTime, m_modificationTime);
		osDump(indent, "Time Scale: %d(0x%x)\n", m_timescale, m_timescale);
		osDump(indent, "Duration: %"PRIu64"(0x%"PRIx64")\n", m_duration, m_duration);
	}
	else
	{	// m_version==0
		osDump(indent, "Creation Time: %"PRIu32"(0x%"PRIx32")\n", (uint32)m_creationTime, (uint32)m_creationTime);
		osDump(indent, "Modification Time: %"PRIu32"(0x%"PRIx32")\n", (uint32)m_modificationTime, (uint32)m_modificationTime);
		osDump(indent, "Time Scale: %d(0x%x)\n", m_timescale, m_timescale);
		osDump(indent, "Duration: %"PRIu32"(0x%"PRIx32")\n", (uint32)m_duration, (uint32)m_duration);
	}

	osDump(indent, "Volume: %s (0x%"PRIx16")\n", m_code, m_language);
}


void MP4MdhdBox::Read()
{
    ReadProperties();

    Skip(); // to end of atom
}
