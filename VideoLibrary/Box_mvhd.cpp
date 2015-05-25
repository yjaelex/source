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
}

void MP4MvhdBox::Read()
{
    ReadProperties();

    Skip(); // to end of atom
}
