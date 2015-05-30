#ifndef _VP_AVC_H
#define _VP_AVC_H
#include "vptypes.h"
#include "os.h"
#include "osFile.h"

typedef struct SPSENTRY
{
    SPSENTRY()
    {
        sequenceParameterSetLength = 0;
    }
    uint16              sequenceParameterSetLength;
    vector<uint8>       sequenceParameterSetNALUnit;
};
typedef struct PPSENTRY
{
    PPSENTRY()
    {
        pictureParameterSetLength = 0;
    }
    uint16              pictureParameterSetLength;
    vector<uint8>       pictureParameterSetNALUnit;
};
typedef struct SPSEXTENTRY
{
    SPSEXTENTRY()
    {
        sequenceParameterSetExtLength = 0;
    }
    uint16              sequenceParameterSetExtLength;
    vector<uint8>       sequenceParameterSetExtNALUnit;
};

class AVCDecoderConfigurationRecord
{
public:
    AVCDecoderConfigurationRecord()
    {
        m_configurationVersion = 1;
        m_AVCProfileIndication = m_profile_compatibility = m_AVCLevelIndication = 0;
        m_hasExts = false;
    }

    void Read(File * file);

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
    //        uint8   lengthSizeMinusOne : 5;
    //    }
    //    uint8 all;
    //};
    uint8                           m_lengthSizeMinusOne; // the length in bytes of the NALUnitLength

    uint8                           m_numOfSequenceParameterSets;
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
}


#endif
