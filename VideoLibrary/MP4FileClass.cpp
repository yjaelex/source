#include "MP4FileClass.h"
#include "MP4Track.h"

bool MP4FileClass::Use64Bits(const char *atomName)
{
    uint32_t atomid = BoxID(atomName);
    if (atomid == BoxID("mdat") || atomid == BoxID("stbl")) {
        return (m_createFlags & MP4_CREATE_64BIT_DATA) == MP4_CREATE_64BIT_DATA;
    }
    if (atomid == BoxID("mvhd") ||
        atomid == BoxID("tkhd") ||
        atomid == BoxID("mdhd")) {
        return (m_createFlags & MP4_CREATE_64BIT_TIME) == MP4_CREATE_64BIT_TIME;
    }
    return false;
}

void MP4FileClass::Check64BitStatus(const char *atomName)
{
    uint32_t atomid = BoxID(atomName);

    if (atomid == BoxID("mdat") || atomid == BoxID("stbl")) {
        m_createFlags |= MP4_CREATE_64BIT_DATA;
    }
    else if (atomid == BoxID("mvhd") ||
        atomid == BoxID("tkhd") ||
        atomid == BoxID("mdhd")) {
        m_createFlags |= MP4_CREATE_64BIT_TIME;
    }
}

void MP4FileClass::GenerateTracks()
{
    uint32_t trackIndex = 0;

    while (1)
    {
        char trackName[32];
        sprintf_s(trackName, sizeof(trackName), "moov.trak[%u]", trackIndex);

        // find next trak atom
        MP4Box* pTrakBox = m_pRootBox->FindBox(trackName);

        // done, no more trak atoms
        if (pTrakBox == NULL)
        {
            break;
        }

        // find track id property
        MP4Box * pTkhd =  pTrakBox->FindBox("trak.tkhd");
        // find track type property
        MP4Box * pHdlrBox = pTrakBox->FindBox("trak.mdia.hdlr");

        // ensure we have the basics properties
        if (pTkhd && pHdlrBox)
        {
            MP4TrackStream* pTrack = NULL;
            pTrack = new MP4TrackStream();
            pTrack->Create(this, pTrakBox);
            m_vMP4Track.push_back(pTrack);
            m_vTrackIds.push_back(pTrack->GetTrackId());
        }
        else
        {
            m_vTrackIds.push_back(0);
            osAssert(!"Invalid Track");
        }

        trackIndex++;
    }
}

uint32_t MP4FileClass::FindTrackIndex(uint32 trackId)
{
    for (uint32_t i = 0; i < m_vMP4Track.size(); i++)
    {
        if (m_vMP4Track[i]->GetTrackId() == trackId)
        {
            return i;
        }
    }

    return (uint32_t)-1; // satisfy MS compiler
}

uint32_t MP4FileClass::GetNumOfSamples(uint32 trackId)
{
    return m_vMP4Track[FindTrackIndex(trackId)]->GetNumOfSamples();
}
uint32_t MP4FileClass::GetSampleSize(uint32 trackId, uint32 sampleId)
{
    return m_vMP4Track[FindTrackIndex(trackId)]->GetSampleSize(sampleId);
}
uint32_t MP4FileClass::GetTrackMaxSampleSize(uint32 trackId)
{
    return m_vMP4Track[FindTrackIndex(trackId)]->GetMaxSampleSize();
}
uint64 MP4FileClass::GetSampleTime(uint32 trackId, uint32 sampleId)
{
    uint64 timestamp;
    m_vMP4Track[FindTrackIndex(trackId)]->GetSampleTimes(sampleId, &timestamp, NULL);
    return timestamp;
}

uint64 MP4FileClass::GetSampleDuration(uint32 trackId, uint32 sampleId)
{
    uint64 duration;
    m_vMP4Track[FindTrackIndex(trackId)]->GetSampleTimes(sampleId, NULL, &duration);
    return duration;
}
bool MP4FileClass::IsSampleSync(uint32 trackId, uint32 sampleId)
{
    return m_vMP4Track[FindTrackIndex(trackId)]->IsSyncSample(sampleId);
}

uint32 MP4FileClass::ReadSample(uint32 trackId, uint32 sampleId, uint8_t* pBuffer, uint32_t bufferSize, bool* pIsSyncSample,
    uint64* pStartTime, uint64*  pDuration, uint64*  pRenderingOffset)
{
    return m_vMP4Track[FindTrackIndex(trackId)]->ReadSample(sampleId, pBuffer, bufferSize,
        pIsSyncSample, pStartTime, pDuration, NULL);
}


bool MP4FileClass::Extract264RawData(const char * fileName)
{
    osAssert(fileName);
    File* pFile = new File(fileName, File::Mode::MODE_CREATE);
    if (!pFile) return false;
    if (pFile->open())
    {
        osAssert(!"Open File failed!");
        return false;
    }

    uint32 SPSSize = 0;
    uint32 PPSSize = 0;
    int64_t outSize = 0;
    uint32 trackId = 1;
    uint32 maxSize = GetTrackMaxSampleSize(trackId);
    uint32 sampleCount = GetNumOfSamples(trackId);
    uint32 sampleSize = 0;
    uint8 * pBuffer = (uint8 *)osMalloc(maxSize + 1);

    uint8 H264Separator[] = { 0, 0, 0, 1 };
    // Write start code
    pFile->write(H264Separator, sizeof(H264Separator), outSize);

    //Write SPS
    uint8* pSPS = (m_vMP4Track[FindTrackIndex(trackId)])->GetSPS(&SPSSize);
    pFile->write(pSPS, SPSSize, outSize);

    //Write PPS
    pFile->write(H264Separator, sizeof(H264Separator), outSize);
    uint8* pPPS = (m_vMP4Track[FindTrackIndex(trackId)])->GetPPS(&PPSSize);
    pFile->write(pPPS, PPSSize, outSize);

    //Write samples.
    uint8 lengthSize = (m_vMP4Track[FindTrackIndex(trackId)])->GetLengthSizeMinusOne() + 1;
    bool isSyncSample = false;
    for (uint32 i = 0; i < sampleCount; i++)
    {
        memset(pBuffer, 0, maxSize + 1);
        sampleSize = 0;
        isSyncSample = false;
        sampleSize = ReadSample(trackId, i + 1, pBuffer, maxSize + 1, &isSyncSample, NULL, NULL, NULL);
        if (!sampleSize || (sampleSize == (uint32)-1))
        {
            pFile->close();
            delete pFile;
            return false;
        }

        if (isSyncSample && i)
        {
            pFile->write(H264Separator, sizeof(H264Separator), outSize);
            pFile->write(pSPS, SPSSize, outSize);
            pFile->write(H264Separator, sizeof(H264Separator), outSize);
            pFile->write(pPPS, PPSSize, outSize);
        }

        pFile->write(H264Separator, sizeof(H264Separator), outSize);
        pFile->write(pBuffer + lengthSize, sampleSize - lengthSize, outSize);
    }
    pFile->close();
    delete pFile;

    return true;
}