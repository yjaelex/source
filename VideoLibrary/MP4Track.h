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

// TrackID, SampleID, ChunkID all start with 1. 0 is invalid num.

class MP4TrackStream : public AVStream
{
    MP4TrackStream()
    {
        m_File = NULL;
        m_TrackID = m_TrackType = 0;
        m_NumOfSamples = 0;

        m_TrakBox = NULL;
        m_MdhdBox = NULL;
        m_DecodeTimeToSampleBox = NULL;
        m_SyncSampleBox = NULL;
        m_SampleToChunkBox = NULL;
        m_SampleSizeBox = NULL;
        m_pChunkOffsetTable = NULL;
        m_pChunkOffsetTable64 = NULL;

        m_cachedSttsSid = m_cachedSttsElapsed = m_cachedSttsIndex = 0;
    }

    bool Create(MP4FileClass * pFile, MP4Box * pTrakBox);

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

    uint64_t GetChunkOffset(uint32 chunkId)
    {
        osAssert(chunkId > 0);
        if (m_pChunkOffsetTable)
        {
            return m_pChunkOffsetTable->at(chunkId-1);
        }
        else if (m_pChunkOffsetTable64)
        {
            return m_pChunkOffsetTable64->at(chunkId - 1);
        }
        else
        {
            osAssert(!"No Valid Chunk Offset Table!!!");
            return -1;
        }
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

    uint64 GetDuration()
    {
        return m_MdhdBox->GetDuration();
    }

    uint32 GetTimeScale()
    {
        return m_MdhdBox->GetTimeScale();
    }

    uint64 GetSampleRenderingOffset(uint32 sampleId)
    {
        //ctts time.
        /// TODO:
        return 0;
    }

    uint32 GetChunkSize(uint32 chunkId);
    bool GetSampleTimes(uint32 sampleId, uint64* pStartTime, uint64* pDuration);
    bool GetSampleInfo(uint32 smpleIndex);
    bool IsSyncSample(uint32 sampleId);
    uint32 GetNextSyncSample(uint32 sampleId);
    uint64_t GetSampleFileOffset(uint32 sampleId, uint32_t * pChunkID = NULL);

    /// ReadChunk
    /// Read an entire chunk.
    /// pChunkBuffer: a buffer for read to
    /// bufferSize:   the size of this buffer.
    /// Return: actual read size. -1 indicate a read error; 0 indicate bufferSize is too small.
    uint32 ReadChunk(uint32_t chunkId, uint8_t* pChunkBuffer, uint32_t bufferSize);

    /// Return: actual read size. -1 indicate a read error; 0 indicate bufferSize is too small.
    uint32 ReadSample(uint32 sampleId, uint8_t* pBuffer, uint32_t bufferSize, bool* pIsSyncSample,
        uint64* pStartTime, uint64*  pDuration, uint64*  pRenderingOffset);

private:
    uint32_t GetSampleStscIndex(uint32 sampleId);

private:
    MP4FileClass *      m_File;
    uint32              m_TrackID;
    uint32              m_TrackType;
    uint32              m_NumOfSamples;

    MP4Box*             m_TrakBox;                  // moov.trak[]
    MP4MdhdBox *        m_MdhdBox;
    MP4SttsBox *        m_DecodeTimeToSampleBox;
    MP4StssBox *        m_SyncSampleBox;
    MP4StscBox *        m_SampleToChunkBox;
    MP4StszBox *        m_SampleSizeBox;
    vector<uint32>*     m_pChunkOffsetTable;
    vector<uint64>*     m_pChunkOffsetTable64;

    //vector<SampleInfo>  m_vSampleInfoTable;

    //cached value
    uint32              m_cachedSttsSid;
    uint64              m_cachedSttsElapsed;
    uint32              m_cachedSttsIndex;
};


#endif

