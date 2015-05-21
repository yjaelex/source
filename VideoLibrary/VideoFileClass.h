#ifndef _VIDEO_FILE_CLASS_H
#define _VIDEO_FILE_CLASS_H

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

    virtual bool Open(const char* fileName);

    virtual void Close();

    virtual uint32 GetSampleByteSize(uint32 sampleIndex);

    virtual bool DumpSampleData(pvoid pBuffer, uint32 bufferSize);

private:
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

class MP4Box;

class MP4FileClass : VideoFileClass
{
public:
    MP4FileClass()
    {
    }

    ~MP4FileClass()
    {
    }

private:
    MP4Box *            m_pRootBox;
};



#endif
