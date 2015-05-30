#include "MP4FileClass.h"
#include "os.h"

// MP4FileClass low level IO support

uint64_t MP4FileClass::GetPosition( File* file )
{
    if( m_memoryBuffer )
        return m_memoryBufferPosition;

    if( !file )
        file = m_file;

    osAssert( file );
    return file->position;
}

void MP4FileClass::SetPosition( uint64_t pos, File* file )
{
    if( m_memoryBuffer ) {
        if( pos >= m_memoryBufferSize )
            osAssert(!"position out of range" );
        m_memoryBufferPosition = pos;
        return;
    }

    if( !file )
        file = m_file;

    osAssert( file );
    if( file->seek( pos ))
        osAssert(!"seek failed");
}

uint64_t MP4FileClass::GetSize( File* file )
{
    if( m_memoryBuffer )
        return m_memoryBufferSize;

    if( !file )
        file = m_file;

    osAssert( file );
    return file->size;
}

bool MP4FileClass::ReadBytes( uint8_t* buf, uint32_t bufsiz, File* file )
{
    if( bufsiz == 0 )
        return true;

    osAssert( buf );
    osWARNING( m_numReadBits > 0 );

    if( m_memoryBuffer ) {
		if (m_memoryBufferPosition + bufsiz > m_memoryBufferSize)
		{
			osAssert(!"not enough bytes, reached end-of-memory");
			return false;
		}
        memcpy( buf, &m_memoryBuffer[m_memoryBufferPosition], bufsiz );
        m_memoryBufferPosition += bufsiz;
        return true;
    }

    if( !file )
        file = m_file;

    osAssert( file );
    File::Size nin;
	if (file->read(buf, bufsiz, nin))
	{
		osAssert(!"read failed");
		return false;
	}

	if (nin != bufsiz)
	{
		osAssert(!"not enough bytes, reached end-of-file");
		return false;
	}

	return true;
}

void MP4FileClass::PeekBytes( uint8_t* buf, uint32_t bufsiz, File* file )
{
    const uint64_t pos = GetPosition( file );
    ReadBytes( buf, bufsiz, file );
    SetPosition( pos, file );
}

void MP4FileClass::EnableMemoryBuffer( uint8_t* pBytes, uint64_t numBytes )
{
    osAssert( !m_memoryBuffer );

    if (pBytes) {
        m_memoryBuffer = pBytes;
        m_memoryBufferSize = numBytes;
    } else {
        if (numBytes) {
            m_memoryBufferSize = numBytes;
        } else {
            m_memoryBufferSize = 4096;
        }
        m_memoryBuffer = (uint8_t*)MP4Malloc(m_memoryBufferSize);
    }
    m_memoryBufferPosition = 0;
}

void MP4FileClass::DisableMemoryBuffer( uint8_t** ppBytes, uint64_t* pNumBytes )
{
    osAssert(m_memoryBuffer != NULL);

    if (ppBytes) {
        *ppBytes = m_memoryBuffer;
    }
    if (pNumBytes) {
        *pNumBytes = m_memoryBufferPosition;
    }

    m_memoryBuffer = NULL;
    m_memoryBufferSize = 0;
    m_memoryBufferPosition = 0;
}

void MP4FileClass::WriteBytes( uint8_t* buf, uint32_t bufsiz, File* file )
{
    osAssert( m_numWriteBits == 0 || m_numWriteBits >= 8 );

    if( !buf || bufsiz == 0 )
        return;

    if( m_memoryBuffer ) {
        if( m_memoryBufferPosition + bufsiz > m_memoryBufferSize ) {
            m_memoryBufferSize = 2 * (m_memoryBufferSize + bufsiz);
            m_memoryBuffer = (uint8_t*)MP4Realloc( m_memoryBuffer, m_memoryBufferSize );
        }
        memcpy( &m_memoryBuffer[m_memoryBufferPosition], buf, bufsiz );
        m_memoryBufferPosition += bufsiz;
        return;
    }

    if( !file )
        file = m_file;

    osAssert( file );
    File::Size nout;
    if( file->write( buf, bufsiz, nout ))
        osAssert(!"write failed");
    if( nout != bufsiz )
        osAssert(!"not all bytes written" );
}

uint64_t MP4FileClass::ReadUInt(uint8_t size)
{
    switch (size) {
    case 1:
        return ReadUInt8();
    case 2:
        return ReadUInt16();
    case 3:
        return ReadUInt24();
    case 4:
        return ReadUInt32();
    case 8:
        return ReadUInt64();
    default:
        osAssert(false);
        return 0;
    }
}

uint8_t MP4FileClass::ReadUInt8()
{
    uint8_t data;
    ReadBytes(&data, 1);
    return data;
}

void MP4FileClass::WriteUInt8(uint8_t value)
{
    WriteBytes(&value, 1);
}

uint16_t MP4FileClass::ReadUInt16()
{
    uint8_t data[2];
    ReadBytes(&data[0], 2);
    return ((data[0] << 8) | data[1]);
}

void MP4FileClass::WriteUInt16(uint16_t value)
{
    uint8_t data[2];
    data[0] = (value >> 8) & 0xFF;
    data[1] = value & 0xFF;
    WriteBytes(data, 2);
}

uint32_t MP4FileClass::ReadUInt24()
{
    uint8_t data[3];
    ReadBytes(&data[0], 3);
    return ((data[0] << 16) | (data[1] << 8) | data[2]);
}

void MP4FileClass::WriteUInt24(uint32_t value)
{
    uint8_t data[3];
    data[0] = (value >> 16) & 0xFF;
    data[1] = (value >> 8) & 0xFF;
    data[2] = value & 0xFF;
    WriteBytes(data, 3);
}

uint32_t MP4FileClass::ReadUInt32()
{
    uint8_t data[4];
    ReadBytes(&data[0], 4);
    return ((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]);
}

void MP4FileClass::WriteUInt32(uint32_t value)
{
    uint8_t data[4];
    data[0] = (value >> 24) & 0xFF;
    data[1] = (value >> 16) & 0xFF;
    data[2] = (value >> 8) & 0xFF;
    data[3] = value & 0xFF;
    WriteBytes(data, 4);
}

uint64_t MP4FileClass::ReadUInt64()
{
    uint8_t data[8];
    uint64_t result = 0;
    uint64_t temp;

    ReadBytes(&data[0], 8);

    for (int i = 0; i < 8; i++) {
        temp = data[i];
        result |= temp << ((7 - i) * 8);
    }
    return result;
}

void MP4FileClass::WriteUInt64(uint64_t value)
{
    uint8_t data[8];

    for (int i = 7; i >= 0; i--) {
        data[i] = value & 0xFF;
        value >>= 8;
    }
    WriteBytes(data, 8);
}

float MP4FileClass::ReadFixed16()
{
    uint8_t iPart = ReadUInt8();
    uint8_t fPart = ReadUInt8();

    return iPart + (((float)fPart) / 0x100);
}

void MP4FileClass::WriteFixed16(float value)
{
    if (value >= 0x100) {
        osAssert(!" out of range");
    }

    uint8_t iPart = (uint8_t)value;
    uint8_t fPart = (uint8_t)((value - iPart) * 0x100);

    WriteUInt8(iPart);
    WriteUInt8(fPart);
}

float MP4FileClass::ReadFixed32()
{
    uint16_t iPart = ReadUInt16();
    uint16_t fPart = ReadUInt16();

    return iPart + (((float)fPart) / 0x10000);
}

void MP4FileClass::WriteFixed32(float value)
{
    if (value >= 0x10000) {
		osAssert(!" out of range");
    }

    uint16_t iPart = (uint16_t)value;
    uint16_t fPart = (uint16_t)((value - iPart) * 0x10000);

    WriteUInt16(iPart);
    WriteUInt16(fPart);
}

float MP4FileClass::ReadFloat()
{
    union {
        float f;
        uint32_t i;
    } u;

    u.i = ReadUInt32();
    return u.f;
}

void MP4FileClass::WriteFloat(float value)
{
    union {
        float f;
        uint32_t i;
    } u;

    u.f = value;
    WriteUInt32(u.i);
}

char* MP4FileClass::ReadString()
{
    uint32_t length = 0;
    uint32_t alloced = 64;
    char* data = (char*)MP4Malloc(alloced);

    do {
        if (length == alloced) {
            data = (char*)MP4Realloc(data, alloced * 2);
            if (data == NULL) return NULL;
            alloced *= 2;
        }
		if (false == ReadBytes((uint8_t*)&data[length], 1))
		{
			if (data) MP4Free(data);
		}
        length++;
    } while (data[length - 1] != 0);

    data = (char*)MP4Realloc(data, length);
    return data;
}

void MP4FileClass::WriteString(char* string)
{
    if (string == NULL) {
        uint8_t zero = 0;
        WriteBytes(&zero, 1);
    } else {
        WriteBytes((uint8_t*)string, (uint32_t)strlen(string) + 1);
    }
}

char* MP4FileClass::ReadCountedString(uint8_t charSize, bool allowExpandedCount, uint8_t fixedLength)
{
    uint32_t charLength;
    if (allowExpandedCount) {
        uint8_t b;
        uint32_t ix = 0;
        charLength = 0;
        do {
            b = ReadUInt8();
            charLength += b;
            ix++;
			if (ix > 25)
				osAssert(!"Counted string too long 25 * 255");
        } while (b == 255);
    } else {
        charLength = ReadUInt8();
    }
    
    if (fixedLength && (charLength > fixedLength - 1)) {
        /*
         * The counted length of this string is greater than the
         * maxiumum fixed length, so truncate the string to the
         * maximum fixed length amount (take 1 byte away from the
         * fixedlength since we've already sacrificed one byte for
         * reading the counted length, and there has been a bug where
         * a non counted string has been used in the place of a
         * counted string).
         */  
        osWARNING(charLength > fixedLength - 1);
        charLength = fixedLength - 1U;
    }

    uint32_t byteLength = charLength * charSize;
    char* data = (char*)MP4Malloc(byteLength + 1);
    if (byteLength > 0) 
	{
		if (false == ReadBytes((uint8_t*)data, byteLength))
		{
			MP4Free(data);
		}
    }
    data[byteLength] = '\0';

    // read padding
    if (fixedLength) {
        const uint8_t padsize = fixedLength - byteLength -1U;
        if( padsize ) {
            uint8_t* padbuf = (uint8_t*)malloc( padsize );
            if (false == ReadBytes( padbuf, padsize ))
			{
				MP4Free(data);
			}
            MP4Free( padbuf );
        }
    }

    return data;
}

void MP4FileClass::WriteCountedString(char* string,
                                 uint8_t charSize, bool allowExpandedCount,
                                 uint32_t fixedLength)
{
    uint32_t byteLength;
    uint8_t zero[1];

    if (string) {
        byteLength = (uint32_t)strlen(string);
        if (fixedLength && (byteLength >= fixedLength)) {
            byteLength = fixedLength-1;
        }
    }
    else {
        byteLength = 0;
    }
    uint32_t charLength = byteLength / charSize;

    if (allowExpandedCount) {
        while (charLength >= 0xFF) {
            WriteUInt8(0xFF);
            charLength -= 0xFF;
        }
        // Write the count
        WriteUInt8(charLength);
    } else {
        if (charLength > 255) {
			osAssert(!"Length is ");
        }
        // Write the count
        WriteUInt8(charLength);
    }

    if (byteLength > 0) {
        // Write the string (or the portion that we want to write)
        WriteBytes((uint8_t*)string, byteLength);
    }

    // Write any padding if this is a fixed length counted string
    if (fixedLength) {
        zero[0] = 0;
        while (byteLength < fixedLength-1U) {
            WriteBytes(zero, 1);
            byteLength++;
        }
    }
}

uint64_t MP4FileClass::ReadBits(uint8_t numBits)
{
    osAssert(numBits > 0);
    osAssert(numBits <= 64);

    uint64_t bits = 0;

    for (uint8_t i = numBits; i > 0; i--) {
        if (m_numReadBits == 0) {
            ReadBytes(&m_bufReadBits, 1);
            m_numReadBits = 8;
        }
        bits = (bits << 1) | ((m_bufReadBits >> (--m_numReadBits)) & 1);
    }

    return bits;
}

void MP4FileClass::FlushReadBits()
{
    // eat any remaining bits in the read buffer
    m_numReadBits = 0;
}

void MP4FileClass::WriteBits(uint64_t bits, uint8_t numBits)
{
    osAssert(numBits <= 64);

    for (uint8_t i = numBits; i > 0; i--) {
        m_bufWriteBits |=
            (((bits >> (i - 1)) & 1) << (8 - ++m_numWriteBits));

        if (m_numWriteBits == 8) {
            FlushWriteBits();
        }
    }
}

void MP4FileClass::PadWriteBits(uint8_t pad)
{
    if (m_numWriteBits) {
        WriteBits(pad ? 0xFF : 0x00, 8 - m_numWriteBits);
    }
}

void MP4FileClass::FlushWriteBits()
{
    if (m_numWriteBits > 0) {
        WriteBytes(&m_bufWriteBits, 1);
        m_numWriteBits = 0;
        m_bufWriteBits = 0;
    }
}

uint32_t MP4FileClass::ReadMpegLength()
{
    uint32_t length = 0;
    uint8_t numBytes = 0;
    uint8_t b;

    do {
        b = ReadUInt8();
        length = (length << 7) | (b & 0x7F);
        numBytes++;
    } while ((b & 0x80) && numBytes < 4);

    return length;
}

void MP4FileClass::WriteMpegLength(uint32_t value, bool compact)
{
    if (value > 0x0FFFFFFF) {
		osAssert(!"out of range: "); 
    }

    int8_t numBytes;

    if (compact) {
        if (value <= 0x7F) {
            numBytes = 1;
        } else if (value <= 0x3FFF) {
            numBytes = 2;
        } else if (value <= 0x1FFFFF) {
            numBytes = 3;
        } else {
            numBytes = 4;
        }
    } else {
        numBytes = 4;
    }

    int8_t i = numBytes;
    do {
        i--;
        uint8_t b = (value >> (i * 7)) & 0x7F;
        if (i > 0) {
            b |= 0x80;
        }
        WriteUInt8(b);
    } while (i > 0);
}


