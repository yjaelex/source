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

class MP4FileClass : public VideoFileClass
{
public:
	MP4FileClass()
	{
	}

	~MP4FileClass()
	{
	}

    uint32_t ReadMpegLength();
    void WriteMpegLength(uint32_t value, bool compact = false);
    bool Use64Bits(const char *atomName);
    void Check64BitStatus(const char *atomName);
    virtual void ReadFromFile();
    virtual void Dump(bool dumpImplicits);

private:
	MP4Box *            m_pRootBox;

    uint32_t            m_createFlags;

private:
	MP4FileClass(const MP4FileClass &src);
    MP4FileClass &operator= (const MP4FileClass &src);
};

class MP4TrackStream : public AVStream
{
    MP4TrackStream()
    {

    }

private:
    MP4FileClass *      m_File;
    uint32              m_TrackID;
    uint32              m_TrackType;

    MP4SttsBox *        m_DecodeTimeToSampleBox;
    MP4StssBox *        m_SyncSampleBox;
    MP4StscBox *        m_SampleToChunkBox;

};


#endif
