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
public:
    MP4TrackStream()
    {
        m_File = NULL;
        m_TrackID = 0;
        m_NumOfSamples = 0;
        m_StreamFileFormat = VP_FILE_MP4;
        m_TrakBox = NULL;
        m_MdhdBox = NULL;
        m_DecodeTimeToSampleBox = NULL;
        m_SyncSampleBox = NULL;
        m_SampleToChunkBox = NULL;
        m_SampleSizeBox = NULL;
        m_pChunkOffsetTable = NULL;
        m_pChunkOffsetTable64 = NULL;
        m_pAVCConfig = NULL;
        m_cachedSttsSid = m_cachedSttsElapsed = m_cachedSttsIndex = 0;
    }

    bool Create(MP4FileClass * pFile, MP4Box * pTrakBox);

    uint32 GetNumOfSyncSamples()
    {
        return m_SyncSampleBox->GetNumOfSyncSamples();
    }

    uint32 GetSampleSize(uint32 sampleId)
    {
        osAssert(sampleId > 0);
        return m_SampleSizeBox->GetSampleSize(sampleId - 1);
    }
    uint32_t GetMaxSampleSize()
    {
        return m_SampleSizeBox->GetMaxSampleSize();
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

    uint64 GetSampleRenderingOffset(uint32 sampleId)
    {
        //ctts time.
        /// TODO:
        return 0;
    }

    uint32 GetTrackId()
    {
        return m_TrackID;
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

    uint32 GetNumOfSPS()
    {
        if (m_Type != VP_STREAM_VIDEO) return 0;
        return m_pAVCConfig->GetNumOfSPS();
    }

    uint32 GetNumOfPPS()
    {
        if (m_Type != VP_STREAM_VIDEO) return 0;
        return m_pAVCConfig->GetNumOfPPS();
    }

    uint8 * GetSPS(uint32 * pSize, uint32 index = 0)
    {
        if (m_Type != VP_STREAM_VIDEO) return NULL;
        return m_pAVCConfig->GetSPS(pSize, index);
    }

    uint8 * GetPPS(uint32 * pSize, uint32 index = 0)
    {
        if (m_Type != VP_STREAM_VIDEO) return NULL;
        return m_pAVCConfig->GetPPS(pSize, index);
    }

    uint8 GetLengthSizeMinusOne()
    {
        if (m_Type != VP_STREAM_VIDEO) return -1;
        return m_pAVCConfig->GetLengthSizeMinusOne();
    }

private:
    uint32_t GetSampleStscIndex(uint32 sampleId);

private:
    MP4FileClass *      m_File;
    uint32              m_TrackID;
    
    MP4Box*             m_TrakBox;                  // moov.trak[]
    MP4TkhdBox *        m_TkhdBox;
    MP4MdhdBox *        m_MdhdBox;
    MP4SttsBox *        m_DecodeTimeToSampleBox;
    MP4StssBox *        m_SyncSampleBox;
    MP4StscBox *        m_SampleToChunkBox;
    MP4StszBox *        m_SampleSizeBox;
    vector<uint32>*     m_pChunkOffsetTable;
    vector<uint64>*     m_pChunkOffsetTable64;

    AVCDecoderConfigurationRecord *     m_pAVCConfig;
    //vector<SampleInfo>  m_vSampleInfoTable;

    //cached value
    uint32              m_cachedSttsSid;
    uint64              m_cachedSttsElapsed;
    uint32              m_cachedSttsIndex;
};


#endif

