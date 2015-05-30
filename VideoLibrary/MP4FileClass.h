#ifndef _VP_MP4FILECLASS_H
#define _VP_MP4FILECLASS_H

#include "VideoFileClass.h"
#include "MP4Box.h"
#include "osFile.h"
#include "os.h"

/** Bit: enable 64-bit data-box. */
#define MP4_CREATE_64BIT_DATA 0x01
/** Bit: enable 64-bit time-atoms. @note Incompatible with QuickTime. */
#define MP4_CREATE_64BIT_TIME 0x02

#define MP4Malloc           osMalloc
#define MP4Calloc           osCalloc
#define MP4Stralloc         osStralloc
#define MP4Realloc          osRealloc
#define MP4Free				osFree

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
	uint32_t ReadMpegLength();

	// Called to free internal string buffer created bt ReadString&ReadCountedString
	void FreeString(char* string)
	{
		if (string)
		{
			MP4Free(string);
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
	void WriteMpegLength(uint32_t value, bool compact = false);

	void EnableMemoryBuffer(
		uint8_t* pBytes = NULL, uint64_t numBytes = 0);
	void DisableMemoryBuffer(
		uint8_t** ppBytes = NULL, uint64_t* pNumBytes = NULL);

    // Get & Set functions
    string & GetFilename()
    {
        return m_FileName;
    }


    bool Use64Bits(const char *atomName);
    void Check64BitStatus(const char *atomName);

private:
	File*				m_file;
    string              m_FileName;
	MP4Box *            m_pRootBox;
	// read/write in memory
	uint8_t*			m_memoryBuffer;
	uint64_t			m_memoryBufferPosition;
	uint64_t			m_memoryBufferSize;

	// bit read/write buffering
	uint8_t				m_numReadBits;
	uint8_t				m_bufReadBits;
	uint8_t				m_numWriteBits;
	uint8_t				m_bufWriteBits;

    uint32_t            m_createFlags;

private:
	MP4FileClass(const MP4FileClass &src);
    MP4FileClass &operator= (const MP4FileClass &src);
};

#endif
