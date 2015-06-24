
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

uint32_t MP4TrackStream::GetSampleStscIndex(MP4SampleId sampleId)
{
    uint32_t stscIndex;
    uint32_t numStscs = m_pStscCountProperty->GetValue();

    if (numStscs == 0) {
        throw new Exception("No data chunks exist", __FILE__, __LINE__, __FUNCTION__);
    }

    for (stscIndex = 0; stscIndex < numStscs; stscIndex++) {
        if (sampleId < m_pStscFirstSampleProperty->GetValue(stscIndex)) {
            ASSERT(stscIndex != 0);
            stscIndex -= 1;
            break;
        }
    }
    if (stscIndex == numStscs) {
        ASSERT(stscIndex != 0);
        stscIndex -= 1;
    }

    return stscIndex;
}

uint64_t MP4TrackStream::GetSampleFileOffset(uint32 sampleId)
{
    uint32_t stscIndex =
        GetSampleStscIndex(sampleId);

    // firstChunk is the chunk index of the first chunk with
    // samplesPerChunk samples in the chunk.  There may be multiples -
    // ie: several chunks with the same number of samples per chunk.
    uint32_t firstChunk =
        m_pStscFirstChunkProperty->GetValue(stscIndex);

    MP4SampleId firstSample =
        m_pStscFirstSampleProperty->GetValue(stscIndex);

    uint32_t samplesPerChunk =
        m_pStscSamplesPerChunkProperty->GetValue(stscIndex);

    // chunkId tells which is the absolute chunk number that this sample
    // is stored in.
    MP4ChunkId chunkId = firstChunk +
        ((sampleId - firstSample) / samplesPerChunk);

    // chunkOffset is the file offset (absolute) for the start of the chunk
    uint64_t chunkOffset = m_pChunkOffsetProperty->GetValue(chunkId - 1);

    MP4SampleId firstSampleInChunk =
        sampleId - ((sampleId - firstSample) % samplesPerChunk);

    // need cumulative samples sizes from firstSample to sampleId - 1
    uint32_t sampleOffset = 0;
    for (MP4SampleId i = firstSampleInChunk; i < sampleId; i++) {
        sampleOffset += GetSampleSize(i);
    }

    return chunkOffset + sampleOffset;
}

