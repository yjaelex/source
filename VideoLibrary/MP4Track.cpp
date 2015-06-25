#include "MP4Track.h"

uint32 MP4TrackStream::GetChunkSize(uint32 chunkId)
{
    uint32_t stscIndex = m_SampleToChunkBox->GetChunkStscIndex(chunkId);
    SampletoChunkEntry stscEntry = m_SampleToChunkBox->GetSampletoChunkEntry(stscIndex);

    uint32 firstChunkId = stscEntry.first_chunk;
    uint32 firstSample = stscEntry.firstSample;
    uint32_t samplesPerChunk = stscEntry.samples_per_chunk;
    uint32 firstSampleInChunk = firstSample + ((chunkId - firstChunkId) * samplesPerChunk);

    // need cumulative sizes of samples in chunk
    uint32_t chunkSize = 0;
    for (uint32_t i = 0; i < samplesPerChunk; i++)
    {
        chunkSize += GetSampleSize(firstSampleInChunk + i);
    }

    return chunkSize;
}

bool MP4TrackStream::GetSampleTimes(uint32 sampleId, uint64* pStartTime, uint64* pDuration)
{
    uint32_t numStts = m_DecodeTimeToSampleBox->GetTimeToSampleTableCount();
    uint32 sid;
    uint64 elapsed;
    DecodingTimeToSampleEntry entry;

    if (m_cachedSttsSid != MP4_INVALID_SAMPLE_ID && sampleId >= m_cachedSttsSid)
    {
        sid = m_cachedSttsSid;
        elapsed = m_cachedSttsElapsed;
    }
    else 
    {
        m_cachedSttsIndex = 0;
        sid = 1;
        elapsed = 0;
    }

    for (uint32_t sttsIndex = m_cachedSttsIndex; sttsIndex < numStts; sttsIndex++)
    {
        entry = m_DecodeTimeToSampleBox->GetTimeToSampleEntry(sttsIndex);
        uint32_t sampleCount = entry.sample_count;
        uint32_t sampleDelta = entry.sample_delta;

        if (sampleId <= sid + sampleCount - 1)
        {
            if (pStartTime)
            {
                *pStartTime = (sampleId - sid);
                *pStartTime *= sampleDelta;
                *pStartTime += elapsed;
            }
            if (pDuration)
            {
                *pDuration = sampleDelta;
            }

            m_cachedSttsIndex = sttsIndex;
            m_cachedSttsSid = sid;
            m_cachedSttsElapsed = elapsed;

            return true;
        }
        sid += sampleCount;
        elapsed += sampleCount * sampleDelta;
    }

    osLog(LOG_ERROR, "sample id out of range: %s, %d, %s.\n", __FILE__, __LINE__, __FUNCTION__);
    return false;
}

bool MP4TrackStream::IsSyncSample(uint32 sampleId)
{
    if (m_SyncSampleBox == NULL)
    {
        return true;
    }

    uint32_t numStss = m_SyncSampleBox->GetNumOfSyncSamples();
    uint32_t stssLIndex = 0;
    uint32_t stssRIndex = numStss - 1;

    while (stssRIndex >= stssLIndex)
    {
        uint32_t stssIndex = (stssRIndex + stssLIndex) >> 1;
        uint32 syncSampleId = m_SyncSampleBox->GetSyncSampleID(stssIndex);

        if (sampleId == syncSampleId)
        {
            return true;
        }

        if (sampleId > syncSampleId)
        {
            stssLIndex = stssIndex + 1;
        }
        else
        {
            stssRIndex = stssIndex - 1;
        }
    }

    return false;
}

// N.B. "next" is inclusive of this sample id
uint32 MP4TrackStream::GetNextSyncSample(uint32 sampleId)
{
    if (m_SyncSampleBox == NULL)
    {
        return true;
    }

    uint32_t numStss = m_SyncSampleBox->GetNumOfSyncSamples();

    for (uint32_t stssIndex = 0; stssIndex < numStss; stssIndex++)
    {
        uint32 syncSampleId = m_SyncSampleBox->GetSyncSampleID(stssIndex);

        if (sampleId > syncSampleId) 
        {
            continue;
        }
        return syncSampleId;
    }

    // LATER check stsh for alternate sample
    return MP4_INVALID_SAMPLE_ID;
}

uint32_t MP4TrackStream::GetSampleStscIndex(uint32 sampleId)
{
    uint32_t stscIndex;
    uint32_t numStscs = m_SampleToChunkBox->GetSampleToChunkTableCount();

    if (numStscs == 0)
    {
        osLog(LOG_ERROR, "No data chunks exist: file %s, line %d, %s\n", __FILE__, __LINE__, __FUNCTION__);
        osAssert(!"No data chunks exist");
    }

    SampletoChunkEntry entry;
    for (stscIndex = 0; stscIndex < numStscs; stscIndex++)
    {
        entry = m_SampleToChunkBox->GetSampletoChunkEntry(stscIndex);
        if (sampleId < entry.firstSample)
        {
            osAssert(stscIndex != 0);
            stscIndex -= 1;
            break;
        }
    }
    if (stscIndex == numStscs)
    {
        osAssert(stscIndex != 0);
        stscIndex -= 1;
    }

    return stscIndex;
}

uint64_t MP4TrackStream::GetSampleFileOffset(uint32 sampleId, uint32_t * pChunkID)
{
    uint32_t stscIndex = GetSampleStscIndex(sampleId);
    SampletoChunkEntry & entry = m_SampleToChunkBox->GetSampletoChunkEntry(stscIndex);

    // firstChunk is the chunk index of the first chunk with
    // samplesPerChunk samples in the chunk.  There may be multiples -
    // ie: several chunks with the same number of samples per chunk.
    uint32_t firstChunk = entry.first_chunk;
    uint32_t firstSample = entry.firstSample;
    uint32_t samplesPerChunk = entry.samples_per_chunk;

    // chunkId tells which is the absolute chunk number that this sample
    // is stored in.
    uint32_t chunkId = firstChunk + ((sampleId - firstSample) / samplesPerChunk);
    if (pChunkID)   *pChunkID = chunkId;

    // chunkOffset is the file offset (absolute) for the start of the chunk
    uint64_t chunkOffset = GetChunkOffset(chunkId);

    uint32_t firstSampleInChunk = sampleId - ((sampleId - firstSample) % samplesPerChunk);

    // need cumulative samples sizes from firstSample to sampleId - 1
    uint32_t sampleOffset = 0;
    for (uint32_t i = firstSampleInChunk; i < sampleId; i++)
    {
        sampleOffset += GetSampleSize(i);
    }

    return chunkOffset + sampleOffset;
}

uint32 MP4TrackStream::ReadChunk(uint32_t chunkId, uint8_t* pChunkBuffer, uint32_t bufferSize)
{
    bool readSuccess = false;
    osAssert(chunkId);
    osAssert(pChunkBuffer);

    uint64_t chunkOffset = GetChunkOffset(chunkId);
    uint32 chunkSize = GetChunkSize(chunkId);
    if (chunkSize > bufferSize) return 0;

    uint64_t oldPos = m_File->GetPosition(); // only used in mode == 'w'
    m_File->SetPosition(chunkOffset);
    readSuccess = m_File->ReadBytes(pChunkBuffer, chunkSize);

    if (m_File->IsWriteMode())   m_File->SetPosition(oldPos);

    if (!readSuccess)
    {
        return -1;
    }

    return chunkSize;
}

uint32 MP4TrackStream::ReadSample(uint32 sampleId, uint8_t* pBuffer, uint32_t bufferSize, bool* pIsSyncSample,
    uint64* pStartTime, uint64*  pDuration, uint64*  pRenderingOffset)
{
    bool readSuccess = false;
    if (sampleId == MP4_INVALID_SAMPLE_ID)
    {
        osAssert(!"sample id can't be zero");
    }

    // handle unusual case of wanting to read a sample
    // that is still sitting in the write chunk buffer

    uint64_t fileOffset = GetSampleFileOffset(sampleId);

    uint32_t sampleSize = GetSampleSize(sampleId);
    if (bufferSize < sampleSize)
    {
        return 0;
    }

    uint64_t oldPos = m_File->GetPosition(); // only used in mode == 'w'
    m_File->SetPosition(fileOffset);
    readSuccess = m_File->ReadBytes(pBuffer, sampleSize);

    if (pStartTime || pDuration)
    {
        GetSampleTimes(sampleId, pStartTime, pDuration);
    }
    if (pRenderingOffset)
    {
        *pRenderingOffset = GetSampleRenderingOffset(sampleId);
    }
    if (pIsSyncSample)
    {
        *pIsSyncSample = IsSyncSample(sampleId);
    }

    if (m_File->IsWriteMode())  m_File->SetPosition(oldPos);
    if (!readSuccess)
    {
        return -1;
    }

    return sampleSize;
}

bool MP4TrackStream::Create(MP4FileClass * pFile, MP4Box * pTrakBox)
{
    if (pFile == NULL)   return false;
    if (pTrakBox == NULL) return false;
    m_File = pFile;
    m_TrackID = 0;
    m_NumOfSamples = 0;

    m_TrakBox = pTrakBox;
    m_TkhdBox = (MP4TkhdBox *) m_TrakBox->FindChildBox("tkhd");
    m_TrackID = m_TkhdBox->GetTrackID();

    m_MdhdBox = (MP4MdhdBox *)m_TrakBox->FindBox("trak.mdia.mdhd");

    MP4Box * pStblBox = m_TrakBox->FindBox("trak.mdia.minf.stbl");
    m_DecodeTimeToSampleBox = (MP4SttsBox *)pStblBox->FindBox("stbl.stts");
    m_SyncSampleBox = (MP4StssBox *)pStblBox->FindBox("stbl.stss");
    m_SampleToChunkBox = (MP4StscBox *)pStblBox->FindBox("stbl.stsc");
    m_SampleSizeBox = (MP4StszBox *)pStblBox->FindBox("stbl.stsz");
    MP4Box * pAVCBox = pStblBox->FindBox("stbl.stsd.avc1");

    MP4Box * pStcoBox = pStblBox->FindBox("stbl.stco");
    if (pStcoBox)
    {
        m_pChunkOffsetTable = ((MP4StcoBox *)pStcoBox)->GetChunkOffsetTable();
        osAssert(m_pChunkOffsetTable);
        m_pChunkOffsetTable64 = NULL;
    }
    else
    {
        pStcoBox = pStblBox->FindBox("stbl.co64");
        osAssert(pStcoBox);
        m_pChunkOffsetTable64 = ((MP4Co64Box *)pStcoBox)->GetChunkOffsetTable();
        osAssert(m_pChunkOffsetTable64);
    }

    MP4HdlrBox * pHdlrBox = (MP4HdlrBox *)m_TrakBox->FindBox("trak.mdia.hdlr");
    m_Type = (VP_STREAMTYPE) (pHdlrBox->m_Tracktype);
    m_NumOfSamples = m_SampleSizeBox->GetNumOfSamples();
    m_nDurationTime =  m_MdhdBox->GetDuration();
    m_nTimeScale =  m_MdhdBox->GetTimeScale();

    osAssert(pAVCBox);    // Only support AVC codec now.
    pAVCBox = pStblBox->FindBox("stbl.stsd.avc1.avcC");
    m_pAVCConfig = ((MP4AvcCBox *)pAVCBox)->GetAVCConfig();
}