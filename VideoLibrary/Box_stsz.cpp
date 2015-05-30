#include "AllMP4Box.h"


MP4StszBox::MP4StszBox(MP4FileClass &file)
        : MP4FullBox(file, "stsz")
{
    m_sample_size = m_sample_count = 0;
    m_vSampleSizeTable.clear();
}

void MP4StszBox::Read()
{
    MP4FullBox::ReadProperties();

    m_sample_size = m_File.ReadUInt32();
    m_sample_count = m_File.ReadUInt32();

    uint32 size = 0;
    if (m_sample_size == 0)
    {
        for (uint32 i = 0; i < m_sample_count; i++)
        {
            size = m_File.ReadUInt32();
            m_vSampleSizeTable.push_back(size);
        }
    }

    Skip(); // to end of atom
}

void MP4StszBox::DumpProperties(uint8_t indent, bool dumpImplicits)
{
    osDump(indent, "Sample Size: %d(0x%x)\n", m_sample_size, m_sample_size);
    osDump(indent, "Sample Count: %d(0x%x)\n", m_sample_count, m_sample_count);

    if (m_sample_size == 0)
    {
        uint32 printCount = min(m_sample_count, 8);
        osDump(indent, "Index     - SampleSize\n");
        for (uint32 i = 0; i < printCount; i++)
        {
            osDump(indent, "%d      - %d       \n", i, m_vSampleSizeTable[i]);
        }
    }
}

void MP4StszBox::WriteProperties()
{
    MP4FullBox::WriteProperties();
    m_File.WriteUInt32(m_sample_size);
    m_File.WriteUInt32(m_sample_count);
    if (m_sample_size == 0)
    {
        for (uint32 i = 0; i < m_vSampleSizeTable.size(); i++)
        {
            m_File.WriteUInt32(m_vSampleSizeTable[i]);
        }
    }
}
