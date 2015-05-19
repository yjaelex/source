#ifndef _VIDEO_FILE_CLASS_H
#define _VIDEO_FILE_CLASS_H

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

class VideoFileClass
{
public:
	VideoFileClass()
	{

	}

private:
	VP_FILETYPE			m_fileType;
	VP_COLORFORMAT		m_colorFormat;
	VP_FILEHANDLE		m_fileHandle;
	uint32				m_nNumFrames;
	uint32				m_nWidth;
	uint32				m_nHeight;
	uint32				m_nNumStreams;

};


#endif
