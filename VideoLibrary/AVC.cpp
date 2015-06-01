#include "AVC.h"

void AVCDecoderConfigurationRecord::Read(VideoFileClass * file, uint64 endPos)
{
    m_configurationVersion = file->ReadUInt8();
    m_AVCProfileIndication = file->ReadUInt8();
    m_profile_compatibility = file->ReadUInt8();
    m_AVCLevelIndication = file->ReadUInt8();
    m_lengthSizeMinusOne = file->ReadUInt8() & 0x3;

    m_numOfSequenceParameterSets = file->ReadUInt8() & 0x1f;
    SPSENTRY sps;
    for (uint32 i = 0; i < m_numOfSequenceParameterSets; i++)
    {
        sps.sequenceParameterSetNALUnit.clear();
        sps.sequenceParameterSetLength = file->ReadUInt16();
        sps.sequenceParameterSetNALUnit.resize(sps.sequenceParameterSetLength);
        file->ReadBytes(sps.sequenceParameterSetNALUnit.data(), sps.sequenceParameterSetLength);

        m_SPSTable.push_back(sps);
    }

    uint8 m_numOfPictureParameterSets = file->ReadUInt8();
    PPSENTRY pps;
    for (uint32 i = 0; i < m_numOfPictureParameterSets; i++)
    {
        pps.pictureParameterSetNALUnit.clear();
        pps.pictureParameterSetLength = file->ReadUInt16();
        pps.pictureParameterSetNALUnit.resize(pps.pictureParameterSetLength);
        file->ReadBytes(pps.pictureParameterSetNALUnit.data(), pps.pictureParameterSetLength);

        m_PPSTable.push_back(pps);
    }

    m_hasExts = false;
    if ((file->GetPosition() <= endPos) && ((endPos - file->GetPosition()) >= 4))
    {
        m_hasExts = true;
    }

    if (m_hasExts && (m_AVCProfileIndication == 100 || m_AVCProfileIndication == 110
        || m_AVCProfileIndication == 122 || m_AVCProfileIndication == 144))
    {
        m_chromaFormat = file->ReadUInt8() & 0x3;
        m_bitDepthLumaMinus8 = file->ReadUInt8() & 0x7;
        m_bitDepthChromaMinus8 = file->ReadUInt8() & 0x7;

        m_numOfSequenceParameterSetExt = file->ReadUInt8();
        SPSEXTENTRY spsExt;
        for (uint32 i = 0; i < m_numOfSequenceParameterSetExt; i++)
        {
            spsExt.sequenceParameterSetExtNALUnit.clear();
            spsExt.sequenceParameterSetExtLength = file->ReadUInt16();
            spsExt.sequenceParameterSetExtNALUnit.resize(spsExt.sequenceParameterSetExtLength);
            file->ReadBytes(spsExt.sequenceParameterSetExtNALUnit.data(), spsExt.sequenceParameterSetExtLength);

            m_SPSExtTable.push_back(spsExt);
        }
    }
    else
    {
        m_chromaFormat = -1;
        m_bitDepthLumaMinus8 = -1;
        m_bitDepthChromaMinus8 = -1;
    }
    osAssert(file->GetPosition() <= endPos);
}

static void printByteBuffer(uint8 * pBuf, uint32 len)
{
    if (!pBuf) return;
    for (uint32 i = 0; i < len; i++)
    {
        osDump(0, "  %x", pBuf[i]);
    }
}

void AVCDecoderConfigurationRecord::Dump(uint8_t indent)
{
    osDump(indent, "Configuration Version: %d(0x%x)\n", m_configurationVersion, m_configurationVersion);
    osDump(indent, "AVCProfile Indication: %d(0x%x)\n", m_AVCProfileIndication, m_AVCProfileIndication);
    osDump(indent, "Profile Compatibility: %d(0x%x)\n", m_profile_compatibility, m_profile_compatibility);
    osDump(indent, "AVC Level Indication: %d(0x%x)\n", m_AVCLevelIndication, m_AVCLevelIndication);
    osDump(indent, "LengthSize MinusOne: %d(0x%x)\n", m_lengthSizeMinusOne, m_lengthSizeMinusOne);

    osDump(indent, "Num Of Sequence Parameter Sets: %d(0x%x)\n", m_numOfSequenceParameterSets, m_numOfSequenceParameterSets);
    for (uint32 i = 0; i < m_numOfSequenceParameterSets; i++)
    {
        printByteBuffer(m_SPSTable[i].sequenceParameterSetNALUnit.data(), m_SPSTable[i].sequenceParameterSetLength);
    }

    osDump(indent, "Num Of Picture Parameter Sets: %d(0x%x)\n", m_numOfPictureParameterSets, m_numOfPictureParameterSets);
    for (uint32 i = 0; i < m_numOfPictureParameterSets; i++)
    {
        printByteBuffer(m_PPSTable[i].pictureParameterSetNALUnit.data(), m_PPSTable[i].pictureParameterSetLength);
    }
}

