#ifndef _VP_MP4FILECLASS_H
#define _VP_MP4FILECLASS_H

#include "VideoFileClass.h"
#include "MP4Box.h"
#include "os.h"

/** Bit: enable 64-bit data-box. */
#define MP4_CREATE_64BIT_DATA 0x01
/** Bit: enable 64-bit time-atoms. @note Incompatible with QuickTime. */
#define MP4_CREATE_64BIT_TIME 0x02

class MP4Box;
class MP4TrackStream;

class MP4FileClass : public VideoFileClass
{
public:
	MP4FileClass()
	{
        m_vMP4Track.clear();
	}

	~MP4FileClass()
	{
	}
    virtual void Close();

    uint32_t ReadMpegLength();
    void WriteMpegLength(uint32_t value, bool compact = false);
    bool Use64Bits(const char *atomName);
    void Check64BitStatus(const char *atomName);
    void GenerateTracks();
    uint32_t FindTrackIndex(uint32 trackId);
    virtual void ReadFromFile();
    virtual void Dump(bool dumpImplicits);

    uint32_t GetNumOfSamples(uint32 trackId);
    uint32_t GetSampleSize(uint32 trackId, uint32 sampleId);
    uint32_t GetTrackMaxSampleSize(uint32 trackId);
    uint64 GetSampleTime(uint32 trackId, uint32 sampleId);
    uint64 GetSampleDuration(uint32 trackId, uint32 sampleId);
    bool IsSampleSync(uint32 trackId, uint32 sampleId);
    uint32 ReadSample(uint32 trackId, uint32 sampleId, uint8_t* pBuffer, uint32_t bufferSize, bool* pIsSyncSample,
        uint64* pStartTime, uint64*  pDuration, uint64*  pRenderingOffset);
    bool Extract264RawData(const char * fileName);

private:
	MP4Box *            m_pRootBox;
    vector<uint32>              m_vTrackIds;
    vector<MP4TrackStream*>     m_vMP4Track;
    uint32_t            m_createFlags;

private:
	MP4FileClass(const MP4FileClass &src);
    MP4FileClass &operator= (const MP4FileClass &src);
};


#endif
