#include "AllMP4Box.h"

MP4MvhdBox::MP4MvhdBox(MP4FileClass &file)
	: MP4FullBox(file, "mvhd")
{
	m_creationTime = 0;
	m_modificationTime = 0;
	m_timescale = 0;
	m_duration = 0;

	m_rate = 0x00010000;			// typically 1.0 (0x00010000)
	m_volume = 0x0100;				// typically, full volume (0x0100)
	m_reserved0 = 0;
	m_reserved[0] = m_reserved[1] = 0;
	memset(m_matrix, 0, sizeof(m_matrix));
	memset(m_pre_defined, 0, sizeof(m_pre_defined));
	m_next_track_ID = 0;
}

void MP4MvhdBox::Generate()
{
    uint8_t version = m_File.Use64Bits(GetType()) ? 1 : 0;
    SetVersion(version);

    MP4Box::Generate();

	// set creation and modification times
	MP4Timestamp now = MP4GetAbsTimestamp();
	m_creationTime = now;
	m_modificationTime = now;

	m_timescale = 1000;
	m_rate = 0x00010000;			// typically 1.0 (0x00010000)
	m_volume = 0x0100;				// typically, full volume (0x0100)

	m_reserved0 = 0;
	m_reserved[0] = m_reserved[1] = 0;

	static uint8_t matrix[36] = {
		0x00, 0x01, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x01, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x40, 0x00, 0x00, 0x00,
	};
	memcpy(m_matrix, matrix, sizeof(matrix));
	memset(m_pre_defined, 0, sizeof(m_pre_defined));
	m_next_track_ID = 1;
}

void MP4MvhdBox::ReadProperties()
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

	m_rate = m_File.ReadUInt32();				// typically 1.0 (0x00010000)
	m_volume = m_File.ReadUInt16();				// typically, full volume (0x0100)
	m_reserved0 = m_File.ReadUInt16();
	m_reserved[0] = m_reserved[1] = 0;
	m_File.ReadBytes((uint8_t*)m_reserved, sizeof(m_reserved));
	m_File.ReadBytes((uint8_t*)m_matrix, sizeof(m_matrix));
	m_File.ReadBytes((uint8_t*)m_pre_defined, sizeof(m_pre_defined));
	m_next_track_ID = m_File.ReadUInt32();
}

void MP4MvhdBox::DumpProperties(uint8_t indent, bool dumpImplicits)
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

	osDump(indent, "Rate: (0x%x 0x%x)\n", m_rate, m_rate);
	osDump(indent, "Volume: %"PRIu16"(0x%"PRIx16")\n", m_volume, m_volume);

	// { 0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000 };
	osDump(indent, "Matrix: { 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x }\n",
		m_matrix[0], m_matrix[1], m_matrix[2],
		m_matrix[3], m_matrix[4], m_matrix[5],
		m_matrix[6], m_matrix[7], m_matrix[8]);

	osDump(indent, "Next TrackID: %d(0x%x)\n", m_next_track_ID, m_next_track_ID);
}

void MP4MvhdBox::Read()
{
    ReadProperties();

    Skip(); // to end of atom
}
