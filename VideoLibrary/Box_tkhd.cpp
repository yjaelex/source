#include "AllMP4Box.h"

MP4TkhdBox::MP4TkhdBox(MP4FileClass &file)
        : MP4FullBox(file, "tkhd")
{
    m_creationTime = 0;
    m_modificationTime = 0;
    m_trackId = 0;
    m_reserved0 = 0;
    m_duration = 0;

    m_reserved1[1] = m_reserved1[0] = 0;
    m_layer = 0;
    m_alternate_group = 0;

    //{ if track_is_audio 0x0100 else 0 };
    m_volume = 0;
    m_reserved2 = 0;

    // { 0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000 };
    // unity matrix
    memset(m_matrix,0,sizeof(m_matrix));
    m_width = 0;
    m_height = 0;
}

void MP4TkhdBox::Generate()
{
    uint8_t version = m_File.Use64Bits(GetType()) ? 1 : 0;
    SetVersion(version);

    MP4Box::Generate();

    // set creation and modification times
    MP4Timestamp now = MP4GetAbsTimestamp();
    m_creationTime = now;
    m_modificationTime = now;

    // property "matrix" has non-zero fixed values
    // this default identity matrix indicates no transformation, i.e.
    // 1, 0, 0
    // 0, 1, 0
    // 0, 0, 1
    // see http://developer.apple.com/documentation/QuickTime/QTFF/QTFFChap4/chapter_5_section_4.html

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
}

void MP4TkhdBox::ReadProperties()
{
	// read version and flags */
    MP4FullBox::ReadProperties();

	if (m_version == 1)
	{
		m_creationTime = m_File.ReadUInt64();
		m_modificationTime = m_File.ReadUInt64();
		m_trackId = m_File.ReadUInt32();
		m_reserved0 = m_File.ReadUInt32();
		m_duration = m_File.ReadUInt64();
	}
	else
	{	// m_version==0
		m_creationTime = m_File.ReadUInt32();
		m_modificationTime = m_File.ReadUInt32();
		m_trackId = m_File.ReadUInt32();
		m_reserved0 = m_File.ReadUInt32();
		m_duration = m_File.ReadUInt32();
	}

	m_reserved1[0] = m_File.ReadUInt32();
	m_reserved1[1] = m_File.ReadUInt32();
	m_layer = m_File.ReadUInt16();
	m_alternate_group = m_File.ReadUInt16();

	//{ if track_is_audio 0x0100 else 0 };
	m_volume = m_File.ReadUInt16();
	m_reserved2 = m_File.ReadUInt16();

	// { 0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000 };
	// unity matrix
	m_File.ReadBytes((uint8_t*)m_matrix, sizeof(m_matrix));
	m_width = m_File.ReadUInt32();
	m_height = m_File.ReadUInt32();
}

void MP4TkhdBox::DumpProperties(uint8_t indent, bool dumpImplicits)
{
	MP4FullBox::DumpProperties(indent, dumpImplicits);

	if (m_version == 1)
	{
		osDump(indent, "Creation Time: %"PRIu64"(0x%"PRIx64")\n", m_creationTime, m_creationTime);
		osDump(indent, "Modification Time: %"PRIu64"(0x%"PRIx64")\n", m_modificationTime, m_modificationTime);
		osDump(indent, "Track Id: %d(0x%x)\n", m_trackId, m_trackId);
		osDump(indent, "Reserved: %d(0x%x)\n", m_reserved0, m_reserved0);
		osDump(indent, "Duration: %"PRIu64"(0x%"PRIx64")\n", m_duration, m_duration);
	}
	else
	{	// m_version==0
		osDump(indent, "Creation Time: %"PRIu32"(0x%"PRIx32")\n", (uint32)m_creationTime, (uint32)m_creationTime);
		osDump(indent, "Modification Time: %"PRIu32"(0x%"PRIx32")\n", (uint32)m_modificationTime, (uint32)m_modificationTime);
		osDump(indent, "Track Id: %d(0x%x)\n", m_trackId, m_trackId);
		osDump(indent, "Reserved: %d(0x%x)\n", m_reserved0, m_reserved0);
		osDump(indent, "Duration: %"PRIu32"(0x%"PRIx32")\n", (uint32)m_duration, (uint32)m_duration);
	}

	osDump(indent, "Reserved[2]: (0x%x 0x%x)\n", m_reserved1[0], m_reserved1[1]);
	osDump(indent, "Layer: %"PRIu16"(0x%"PRIx16")\n", m_layer, m_layer);
	osDump(indent, "Alternate Group: %"PRIu16"(0x%"PRIx16")\n", m_alternate_group, m_alternate_group);
	
	osDump(indent, "Duration: %"PRIu32"(0x%"PRIx32")\n", m_duration, m_duration);

	osDump(indent, "Volume: %"PRIu16"(0x%"PRIx16")\n", m_volume, m_volume);
	osDump(indent, "Reserved2: %"PRIu16"(0x%"PRIx16")\n", m_reserved2, m_reserved2);

	// { 0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000 };
	osDump(indent, "Matrix: { 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x }\n", 
		m_matrix[0], m_matrix[1], m_matrix[2],
		m_matrix[3], m_matrix[4], m_matrix[5],
		m_matrix[6], m_matrix[7], m_matrix[8]);

	osDump(indent, "Width: %d(0x%x >>16)\n", m_width>>16, m_width);
	osDump(indent, "Height: %d(0x%x >>16)\n", m_height>>16, m_height);

}

void MP4TkhdBox::Read()
{
    ReadProperties();

    Skip(); // to end of atom
}

