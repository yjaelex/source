#include "AllMP4Box.h"

MP4StscBox::MP4StscBox(MP4FileClass &file)
        : MP4FullBox(file, "stsc")
{
	m_entryCount = 0;
	m_vSampletoChunkTable.clear();
}

void MP4StscBox::Read()
{
	MP4FullBox::ReadProperties();

    m_entryCount = m_File.ReadUInt32();

    SampletoChunkEntry entry;
    for (uint32 i = 0; i < m_entryCount; i++)
    {
        entry.first_chunk = m_File.ReadUInt32();
        entry.samples_per_chunk = m_File.ReadUInt32();
        entry.sample_description_index = m_File.ReadUInt32();
        entry.firstSample = 0;
        m_vSampletoChunkTable.push_back(entry);
    }

    uint32 sampleId = 1;
    for (uint32_t i = 0; i < m_entryCount; i++)
    {
        m_vSampletoChunkTable[i].firstSample = sampleId;

        if (i < m_entryCount - 1)
        {
            sampleId +=
                (m_vSampletoChunkTable[i + 1].first_chunk - m_vSampletoChunkTable[i].first_chunk)
                * m_vSampletoChunkTable[i].samples_per_chunk;
        }
    }

    Skip();
}

