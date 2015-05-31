#ifndef _VIDEO_FILE_CLASS_H
#define _VIDEO_FILE_CLASS_H
#include "osFile.h"
#include "os.h"
#include "vptypes.h"

typedef enum VP_FILETYPE
{
	VP_FILE_RAW = 0,
	VP_FILE_MP4
};

typedef enum VP_COLORFORMAT
{
	VP_UNKNOWN_COLOR = 0,
	VP_RGB888,
	VP_RGBA8888,
	VP_YUV444,
	VP_YUV422,
	VP_YUV420,
	VP_NV12
};

typedef enum VP_STREAMTYPE
{
    VP_STREAM_UNKOWN = 0,
    VP_STREAM_AUDIO,
    VP_STREAM_VIDEO
};

// stream class, similar with track concept in MP4 file.
class AVStream
{
public:
    AVStream()
    {
        m_pMediaInfo = NULL;
        m_pStreamBuffer = NULL;
        m_Type = VP_STREAM_UNKOWN;
        m_colorFormat = VP_UNKNOWN_COLOR;
        m_nNumSamples = m_nDurationTime = m_nTimeScale = 0;
    }

    ~AVStream()
    {

    }

private:
    pvoid               m_pMediaInfo;
    pvoid               m_pStreamBuffer;

    // sample maybe different with different file type
    // RAW:         Frame
    // H.264 RAW:   NALU
    // MP4:         mp4 Samples
    uint32              m_nNumSamples;

    VP_STREAMTYPE       m_Type;
    uint64              m_nByteSize;
    uint32              m_nDurationTime;
    uint32              m_nTimeScale;
    VP_COLORFORMAT		m_colorFormat;
};

// Base class for video file;
// Default type is raw file(RGB/YUV)
class VideoFileClass
{
public:
	VideoFileClass()
	{
        m_fileType = VP_FILE_RAW;
        m_fileHandle = NULL;
        m_nNumStreams = m_nNumFrames = 0;
        m_nFileDurationTime = m_nFileTimeScale = 0;
        m_nWidth = m_nHeight = 0;
        m_vFileStreams.clear();
	}

    ~VideoFileClass()
    {
        m_vFileStreams.clear();
    }

    // IO functions
    uint64_t GetPosition(File* file = NULL);
    void SetPosition(uint64_t pos, File* file = NULL);
    uint64_t GetSize(File* file = NULL);

    bool ReadBytes(uint8_t* buf, uint32_t bufsiz, File* file = NULL);
    void PeekBytes(uint8_t* buf, uint32_t bufsiz, File* file = NULL);

    uint64_t ReadUInt(uint8_t size);
    uint8_t ReadUInt8();
    uint16_t ReadUInt16();
    uint32_t ReadUInt24();
    uint32_t ReadUInt32();
    uint64_t ReadUInt64();
    float ReadFixed16();
    float ReadFixed32();
    float ReadFloat();
    char* ReadString();
    char* ReadCountedString(
        uint8_t charSize = 1, bool allowExpandedCount = false, uint8_t fixedLength = 0);
    uint64_t ReadBits(uint8_t numBits);
    void FlushReadBits();

    // Called to free internal string buffer created bt ReadString&ReadCountedString
    void FreeString(char* string)
    {
        if (string)
        {
            osFree(string);
        }
    }

    void WriteBytes(uint8_t* buf, uint32_t bufsiz, File* file = NULL);
    void WriteUInt8(uint8_t value);
    void WriteUInt16(uint16_t value);
    void WriteUInt24(uint32_t value);
    void WriteUInt32(uint32_t value);
    void WriteUInt64(uint64_t value);
    void WriteFixed16(float value);
    void WriteFixed32(float value);
    void WriteFloat(float value);
    void WriteString(char* string);
    void WriteCountedString(char* string,
        uint8_t charSize = 1,
        bool allowExpandedCount = false,
        uint32_t fixedLength = 0);
    void WriteBits(uint64_t bits, uint8_t numBits);
    void PadWriteBits(uint8_t pad = 0);
    void FlushWriteBits();

    void EnableMemoryBuffer(
        uint8_t* pBytes = NULL, uint64_t numBytes = 0);
    void DisableMemoryBuffer(
        uint8_t** ppBytes = NULL, uint64_t* pNumBytes = NULL);

    // Get & Set functions
    string & GetFilename()
    {
        return m_FileName;
    }

    virtual bool Open(const char* fileName);

    virtual void Close();

    virtual uint32 GetSampleByteSize(uint32 sampleIndex);

    virtual bool DumpSampleData(pvoid pBuffer, uint32 bufferSize);

private:
    File*				m_file;
    string              m_FileName;
    // read/write in memory
    uint8_t*			m_memoryBuffer;
    uint64_t			m_memoryBufferPosition;
    uint64_t			m_memoryBufferSize;

    // bit read/write buffering
    uint8_t				m_numReadBits;
    uint8_t				m_bufReadBits;
    uint8_t				m_numWriteBits;
    uint8_t				m_bufWriteBits;

	VP_FILETYPE			m_fileType;
	VP_FILEHANDLE		m_fileHandle;

    // how many frames/pictures the video data has.
    // note one frame may contain lots of samples.
	uint32				m_nNumFrames;
    uint32              m_nFileDurationTime;
    uint32              m_nFileTimeScale;
	uint32				m_nWidth;
	uint32				m_nHeight;
	uint32				m_nNumStreams;
    vector<AVStream>    m_vFileStreams;
};


#endif
