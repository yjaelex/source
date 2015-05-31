#ifndef _VP_AVC_H
#define _VP_AVC_H

#include "vptypes.h"
#include "os.h"
#include "osFile.h"
#include "VideoFileClass.h"

typedef struct SPSENTRY
{
    SPSENTRY()
    {
        sequenceParameterSetLength = 0;
    }
    uint16              sequenceParameterSetLength;
    vector<uint8>       sequenceParameterSetNALUnit;
}SPSENTRY;

typedef struct PPSENTRY
{
    PPSENTRY()
    {
        pictureParameterSetLength = 0;
    }
    uint16              pictureParameterSetLength;
    vector<uint8>       pictureParameterSetNALUnit;
}PPSENTRY;

typedef struct SPSEXTENTRY
{
    SPSEXTENTRY()
    {
        sequenceParameterSetExtLength = 0;
    }
    uint16              sequenceParameterSetExtLength;
    vector<uint8>       sequenceParameterSetExtNALUnit;
}SPSEXTENTRY;

class AVCDecoderConfigurationRecord
{
public:
    AVCDecoderConfigurationRecord()
    {
        m_configurationVersion = 1;
        m_AVCProfileIndication = m_profile_compatibility = m_AVCLevelIndication = 0;
        m_lengthSizeMinusOne = 0;
        m_hasExts = false;
    }

    void Read(VideoFileClass * file, uint64 endPos);
    void Dump();

    uint32 GetNumOfSPS()
    {
        return m_SPSTable.size();
    }

    uint32 GetNumOfPPS()
    {
        return m_PPSTable.size();
    }

    uint8 * GetSPS(uint32 index = 0)
    {
        if (index < m_SPSTable.size())
        {
            return m_SPSTable[index].sequenceParameterSetNALUnit.data();
        }

        return NULL;
    }

    uint8 * GetPPS(uint32 index = 0)
    {
        if (index < m_PPSTable.size())
        {
            return m_PPSTable[index].pictureParameterSetNALUnit.data();
        }

        return NULL;
    }

    uint8                           m_configurationVersion;           // shall be 1
    uint8                           m_AVCProfileIndication;           // profile code as defined in ISO/IEC 14496-10
    uint8                           m_profile_compatibility;
    uint8                           m_AVCLevelIndication;

    //union
    //{
    //    struct len
    //    {
    //        uint8   reserved1 : 6;              // bit(6) reserved = ¡®111111¡¯b;
    //        uint8   lengthSizeMinusOne : 2;
    //    }
    //    uint8 all;
    //};
    uint8                           m_lengthSizeMinusOne;

    //union
    //{
    //    struct len
    //    {
    //        uint8   reserved1 : 3;              // bit(3) reserved = ¡®111¡¯b;;
    //        uint8   numOfSequenceParameterSets : 5;
    //    }
    //    uint8 all;
    //};
    uint8                           m_numOfSequenceParameterSets; // the length in bytes of the NALUnitLength

    vector<SPSENTRY>                m_SPSTable;
    uint8                           m_numOfPictureParameterSets;
    vector<PPSENTRY>                m_PPSTable;

    //if (profile_idc == 100 || profile_idc == 110 ||
    //    profile_idc == 122 || profile_idc == 144)
    //{
    //    bit(6) reserved = ¡®111111¡¯b;
    //    unsigned int(2) chroma_format;
    //    bit(5) reserved = ¡®11111¡¯b;
    //    unsigned int(3) bit_depth_luma_minus8;
    //    bit(5) reserved = ¡®11111¡¯b;
    //    unsigned int(3) bit_depth_chroma_minus8;
    //    uint8 m_numOfSequenceParameterSetExt;
    //    for (i = 0; i< numOfSequenceParameterSetExt; i++) {
    //        unsigned int(16) sequenceParameterSetExtLength;
    //        bit(8 * sequenceParameterSetExtLength) sequenceParameterSetExtNALUnit;
    //    }
    //}
    bool                            m_hasExts;
    uint8                           m_chromaFormat;         //bit(6) reserved = ¡®111111¡¯b;
    uint8                           m_bitDepthLumaMinus8;   //bit(5) reserved = ¡®11111¡¯b;
    uint8                           m_bitDepthChromaMinus8;  //bit(5) reserved = ¡®11111¡¯b;
    uint8                           m_numOfSequenceParameterSetExt;
    vector<SPSEXTENTRY>             m_SPSExtTable;
};


#endif
