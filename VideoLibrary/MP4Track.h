#ifndef _VP_MP4TRACK_H
#define _VP_MP4TRACK_H

#include "MP4FileClass.h"
#include "AllMP4Box.h"

typedef enum MP4_SAMPLE_TYPE
{
    SAMPLE_I_FRAME = 0,                     // sync sample
    SAMPLE_P_FRAME,
    SAMPLE_B_FRAME
};

typedef struct SampleInfo
{
public:
    uint32 sampleIndex;                     // Sample index.
    MP4_SAMPLE_TYPE sampleType;
    uint32 sampleDecodingTime;
    uint32 sampleChunkNumber;               // The chunk this sample belongs.
    uint64 sampleOffset;                    // Total offset in file of this sample.
    uint32 sampleSizeInBytes;
}SampleInfo;

class MP4TrackStream : public AVStream
{
    MP4TrackStream()
    {

    }

    uint32 GetNumOfSyncSamples()
    {
        return m_SyncSampleBox->GetNumOfSyncSamples();
    }

    uint32 GetNumOfSamples()
    {
        return m_NumOfSamples;
    }

    uint32 GetSampleSize(uint32 sampleId)
    {
        osAssert(sampleId > 0);
        return m_SampleSizeBox->GetSampleSize(sampleId - 1);
    }

    uint32 GetNumOfChunks() const
    {
        if (m_pChunkOffsetTable)
        {
            return m_pChunkOffsetTable->size();
        }
        else if (m_pChunkOffsetTable64)
        {
            return m_pChunkOffsetTable64->size();
        }
        else
        {
            osAssert(!"No Valid Chunk Offset Table!!!");
        }
    }

    uint32 GetChunkSize(uint32 chunkId);

    bool GetSampleInfo(uint32 smpleIndex);

private:
    MP4FileClass *      m_File;
    uint32              m_TrackID;
    uint32              m_TrackType;
    uint32              m_NumOfSamples;

    MP4SttsBox *        m_DecodeTimeToSampleBox;
    MP4StssBox *        m_SyncSampleBox;
    MP4StscBox *        m_SampleToChunkBox;
    MP4StszBox *        m_SampleSizeBox;
    vector<uint32>*     m_pChunkOffsetTable;
    vector<uint64>*     m_pChunkOffsetTable64;

    //vector<SampleInfo>  m_vSampleInfoTable;

};


#endif

