
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
