#ifndef _VP_MPEGTS_H
#define _VP_MPEGTS_H
#include "VideoFileClass.h"


/* PIDs */
#define PAT_PID         0x0000
#define SDT_PID         0x0011
#define EIT_PID         0x0012
#define TDT_PID         0x0014

/* TIDs */
#define SDT_TID         0x42
#define EIT_TID         0x4e
#define TDT_TID         0x70
#define PAT_TID         0x00
#define PMT_TID         0x02
#define M4OD_TID        0x05

#define STREAM_TYPE_VIDEO_MPEG1     0x01
#define STREAM_TYPE_VIDEO_MPEG2     0x02
#define STREAM_TYPE_AUDIO_MPEG1     0x03
#define STREAM_TYPE_AUDIO_MPEG2     0x04
#define STREAM_TYPE_PRIVATE_SECTION 0x05
#define STREAM_TYPE_PRIVATE_DATA    0x06
#define STREAM_TYPE_AUDIO_AAC       0x0f
#define STREAM_TYPE_AUDIO_AAC_LATM  0x11
#define STREAM_TYPE_VIDEO_MPEG4     0x10
#define STREAM_TYPE_VIDEO_H264      0x1b
#define STREAM_TYPE_VIDEO_HEVC      0x24
#define STREAM_TYPE_VIDEO_CAVS      0x42
#define STREAM_TYPE_VIDEO_VC1       0xea
#define STREAM_TYPE_VIDEO_DIRAC     0xd1

#define STREAM_TYPE_AUDIO_AC3       0x81
#define STREAM_TYPE_AUDIO_DTS       0x82
#define STREAM_TYPE_AUDIO_TRUEHD    0x83

#define  TSPKT_LENGTH  188

enum adaptation_field_control_e
{
    PAYLOAD_ONLY = 1,
    ADAPT_FIELD_ONLY = 2,
    ADAPT_FIELD_AND_PAYLOAD = 3,
};

//transport_packet()
//{
//    sync_byte
//    transport_error_indicator
//    payload_unit_start_indicator
//    transport_priority
//    PID
//    transport_scrambling_control
//    adaptation_field_control
//    continuity_counter
//    if(adaptation_field_control=='10' || adaptation_field_control=='11')
//    {
//        adaptation_field()
//    }
//    if(adaptation_field_control=='01' || adaptation_field_control=='11')
//    {
//        for (i=0;i<N;i++)
//        {
//            data_byte
//        }
//    }
//}
typedef struct ts_header
{
    uint8   sync_byte;
    uint32  transport_error_indicator : 1;
    uint32  payload_unit_start_indicator : 1;
    uint32  transport_priority : 1;
    uint32  PID : 13;
    uint32  transport_scrambling_control : 2;
    uint32  adaptation_field_control : 2;
    uint32  continuity_counter : 4;
    // adaptation_field.
}ts_header;

// adaptation_field_control
// 00 Reserved for future use by ISO / IEC
// 01 No adaptation_field, payload only
// 10 Adaptation_field only, no payload
// 11 Adaptation_field followed by payload
typedef struct ts_adaptation_field
{
    uint8     adaptation_field_length;
    uint32    discontinuity_idicator : 1;
    uint32    random_access_indicator : 1;
    uint32    elementary_stream_priority_indicator : 1;
    uint32    PCR_flag : 1;
    uint32    OPCR_flag : 1;
    uint32    splicing_point_flag : 1;
    uint32    transport_private_data_flag : 1;
    uint32    adaptation_field_extension_flag : 1;
    // adaptation_field variable bytes.
}ts_adaptation_field;

class MpegTSSection
{
public:
    uint32 pid;
    uint32 cc;

    MpegTSSection()
    {
        pid = 0;
        /* Initialize at 15 so that it wraps and is equal to 0 for the
        * first packet we write. */
        cc = 15;
    }
};

typedef struct MpegTSProgramInfo
{
    uint32              progNum;
    vector<AVStream*>   avStream;


};

class MpegTSClass : public VideoFileClass
{
public:
    MpegTSClass()
    {

    }

    ~MpegTSClass()
    {

    }

private:
    MpegTSSection       m_SDT;
    MpegTSSection       m_PAT;
    MpegTSSection       m_PMT;

};


#endif
