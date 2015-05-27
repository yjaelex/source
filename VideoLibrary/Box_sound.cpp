#include "AllMP4Box.h"

MP4SoundBox::MP4SoundBox(MP4FileClass &file, const char *boxid)
        : MP4Box(file, boxid)
{
	memset(&m_entry, 0, sizeof(m_entry));
	m_reserved1[0] = m_reserved1[1] = 0;
	m_channelcount = 0;
	m_samplesize = 0;
	m_pre_defined = 0;
	m_reserved2 = 0;
	m_samplerate = 0;

    if (BoxID(boxid) == BoxID("mp4a")) {
        ExpectChildBox("esds", Required, OnlyOne);
        ExpectChildBox("wave", Optional, OnlyOne);
    } else if (BoxID(boxid) == BoxID("alac")) {
        ExpectChildBox("alac", Optional, Optional);
    }
}

void MP4SoundBox::Generate()
{
    MP4Box::Generate();

	m_channelcount = 2;
	m_samplesize = 16;
}

void MP4SoundBox::Read()
{
    MP4Box *parent = GetParentBox();
    if (BoxID(parent->GetType()) != BoxID("stsd"))
    {
        // Quicktime has an interesting thing - they'll put an mp4a box
        // which is blank inside a wave box, which is inside an mp4a box
        // we have a mp4a inside an wave inside an mp4a - delete all properties
        if (BoxID(GetType()) == BoxID("alac"))
        {
            //AddProperty(new MP4BytesProperty(*this, "decoderConfig", m_size));
        }
        if (m_vChildBoxInfos.size() > 0)
        {
            ReadChildBoxs();
        }
    }
    else
    {
        // ByteBuffer content = ByteBuffer.allocate(28);
        // total 28 bytes if m_soundVersion is 0
        m_File.ReadBytes(m_entry.m_reserved, sizeof(m_entry.m_reserved));
        m_entry.m_data_reference_index = m_File.ReadUInt16();

        // 8 bytes already parsed
        //reserved bits - used by qt
        //soundVersion = IsoTypeReader.readUInt16(content);
        //reserved
        //reserved1 = IsoTypeReader.readUInt16(content);
        //reserved2 = IsoTypeReader.readUInt32(content);
        m_soundVersion = m_File.ReadUInt16();
        m_reserved1[0] = (uint32) m_File.ReadUInt16();
        m_reserved1[1] = m_File.ReadUInt32();
        m_channelcount = m_File.ReadUInt16();
        m_samplesize = m_File.ReadUInt16();
        m_pre_defined = m_File.ReadUInt16();
        //reserved bits - used by qt
        m_compressionId = m_pre_defined;
        m_reserved2 = m_File.ReadUInt16();
        //reserved bits - used by qt
        m_packetSize = m_reserved2;
        m_samplerate = m_File.ReadUInt32();

        if (m_soundVersion == 1)
        {
            m_samplesPerPacket = m_File.ReadUInt32();
            m_bytesPerPacket = m_File.ReadUInt32();
            m_bytesPerFrame = m_File.ReadUInt32();
            m_bytesPerSample = m_File.ReadUInt32();
        }
        if (m_soundVersion == 2)
        {
            m_samplesPerPacket = m_File.ReadUInt32();
            m_bytesPerPacket = m_File.ReadUInt32();
            m_bytesPerFrame = m_File.ReadUInt32();
            m_bytesPerSample = m_File.ReadUInt32();
            m_File.ReadBytes(m_soundVersion2Data, sizeof(m_soundVersion2Data));
        }

        if (m_vChildBoxInfos.size() > 0)
        {
            ReadChildBoxs();
        }
    }

    Skip();
}

void MP4SoundBox::DumpProperties(uint8_t indent, bool dumpImplicits)
{
    osDump(indent, "Data Reference Index: %"PRIu16"(0x%"PRIx16")\n", m_entry.m_data_reference_index, m_entry.m_data_reference_index);
    osDump(indent, "Sound Version: %"PRIu16"(0x%"PRIx16")\n", m_soundVersion, m_soundVersion);
    osDump(indent, "Channel Count: %"PRIu16"(0x%"PRIx16")\n", m_channelcount, m_channelcount);
    osDump(indent, "Sample Size: %"PRIu16"(0x%"PRIx16")\n", m_samplesize, m_samplesize);
    osDump(indent, "Sample Rate: %d(0x%x)\n", m_samplerate, m_samplerate);
}
