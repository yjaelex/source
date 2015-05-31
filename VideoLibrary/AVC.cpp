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

