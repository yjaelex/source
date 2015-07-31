#ifndef _VP_MPEGTS_H
#define _VP_MPEGTS_H
#include "VideoFileClass.h"
#include <stdint.h>

/* PIDs */
#define PAT_PID         0x0000
#define SDT_PID         0x0011
#define EIT_PID         0x0012
#define TDT_PID         0x0014

// user defined PIDs
#define PMT_PID         0x1000
#define AVSTREAM_START_PID         0x100
#define H264_PID        0x200
#define AUDIO_PID       0x300

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
#define AV_NOPTS_VALUE   INT64_C(0x8000000000000000)

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

class MpegTSClass;

class MpegTSSection
{
public:
    uint16 pid;
    uint16 cc;

    MpegTSSection()
    {
        pid = 0;
        /* Initialize at 15 so that it wraps and is equal to 0 for the
        * first packet we write. */
        cc = 15;
    }

    void write_packet(const uint8_t *packet)
    {

    }
};

class MpegTSPacket
{
public:
    uint16  pid;
    uint16  cc;
    int64   pts;
    int64   dts;
    uint8*  payload;
    uint32  maxBufferSize;
    uint32  payload_size;
    bool    keyFrame;

    MpegTSPacket()
    {
        pid = H264_PID;
        cc = (uint16)-1;
        pts = dts = 0;
        payload = NULL;
        payload_size = 0;
        maxBufferSize = 0;
        keyFrame = false;
    }

    ~MpegTSPacket()
    {
        if (payload)
        {
            osFree(payload);
            payload_size = 0;
        }
    }

    uint8* alloc(uint32 maxSize)
    {
        if (maxSize > maxBufferSize)
        {
            maxBufferSize = maxSize;
            if (payload)
            {
                osFree(payload);
            }
            payload = (uint8*)osMalloc(maxBufferSize);
        }
        return payload;
    }

    void init(uint16 s_pid, uint32 size, int64 s_pts, int64 s_dts, bool isSync)
    {
        pid = s_pid;
        cc = (uint16)-1;
        pts = s_pts;
        dts = s_dts;
        payload_size = size;
        keyFrame = isSync;
    }

    void write(const uint8_t *packet, uint32 size)
    {

    }
};

#define MPEGTS_FLAG_REEMIT_PAT_PMT  0x01
#define MPEGTS_FLAG_AAC_LATM        0x02
typedef struct MpegTSProgramInfo
{
    MpegTSProgramInfo()
    {
        progNum = 1;
        pmtPID = PMT_PID;
        pcr_pid = H264_PID;
        flags = 0;
        sdt_packet_count = pat_packet_count = 0;
        sdt_packet_period = pat_packet_period = 0;
        mux_rate = 1;
        max_delay = 0;      //AV_TIME_BASE * 7 / 10;
        pcr_packet_count = pcr_packet_period = 0;
        tsFile = NULL;
    }
    uint16              progNum;
    uint16              pmtPID;
    uint32              pcr_pid;
    uint32              flags;
    uint32              sdt_packet_count;
    uint32              pat_packet_count;
    uint32              sdt_packet_period;
    uint32              pat_packet_period;
    uint64              mux_rate;
    uint64              max_delay;
    uint32              pcr_packet_count;
    uint32              pcr_packet_period;
    MpegTSClass*        tsFile;
    MpegTSSection       sdt;
    MpegTSSection       pat;
    MpegTSSection       pmt;
    vector<AVStream*>   avStream;
}MpegTSProgramInfo;

class MpegTSClass : public VideoFileClass
{
public:
    MpegTSClass()
    {

    }

    ~MpegTSClass()
    {

    }

    uint32 addH264Stream(uint64 bitrate, AVStream* av, uint32 maxPktSize)
    {
        m_prog.pcr_pid = H264_PID;
        //m_prog.mux_rate = bitrate;
        m_prog.avStream.push_back(av);
        m_pkt.alloc(maxPktSize);
        return m_prog.avStream.size() - 1;
    }

    bool writePacket(uint32 index, uint8 * buf, uint32 size, bool isSync,
        int64 pts = AV_NOPTS_VALUE, int64 dts = AV_NOPTS_VALUE);

    MpegTSProgramInfo   m_prog;
    MpegTSPacket        m_pkt;
private:

    uint16 getStreamPID(uint32 index)
    {
        if (m_prog.avStream.size() > index)
        {
            if (m_prog.avStream[index]->GetType() == VP_STREAM_VIDEO)
            {
                return H264_PID;
            }
            else if (m_prog.avStream[index]->GetType() == VP_STREAM_AUDIO)
            {
                return AUDIO_PID;
            }
            else
            {
                return 0;
            }
        }
    }
};


#endif
