#include "AllMP4Box.h"

MP4Stz2Box::MP4Stz2Box(MP4FileClass &file)
        : MP4FullBox(file, "stz2")
{
    m_field_size = 0;
    m_sample_count = 0;
}

void MP4Stz2Box::Read()
{
    MP4FullBox::ReadProperties();

    m_field_size = 0x0000000f & m_File.ReadUInt32();
    m_sample_count = m_File.ReadUInt32();
    osAssert((m_field_size == 4) || (m_field_size == 8) || (m_field_size == 16));

    uint16 size = 0;
    switch (m_field_size)
    {
    case 4:
        for (uint32 i = 0; i < m_sample_count/2; i++)
        {
            size = m_File.ReadUInt8();
            m_vSampleSizeTable.push_back( (size&0xf0)>>4 );
            m_vSampleSizeTable.push_back(size&0x0f);
        }
        break;
    case 8:
        for (uint32 i = 0; i < m_sample_count; i++)
        {
            size = m_File.ReadUInt8();
            m_vSampleSizeTable.push_back(size);
        }
        break;
    case 16:
        for (uint32 i = 0; i < m_sample_count; i++)
        {
            size = m_File.ReadUInt16();
            m_vSampleSizeTable.push_back(size);
        }
        break;
    default:
        osAssert(0);
        break;
    }

    Skip(); // to end of atom
}

void MP4Stz2Box::DumpProperties(uint8_t indent, bool dumpImplicits)
{
    osDump(indent, "Field Size: %d(0x%x)\n", m_field_size, m_field_size);
    osDump(indent, "Sample Count: %d(0x%x)\n", m_sample_count, m_sample_count);
}
