#ifndef _VP_ALLMP4BOX_H
#define _VP_ALLMP4BOX_H

#include "MP4Box.h"

class MP4FtypBox;
class MP4FreeBox;

/// ISO base media full-Box.
class MP4FullBox : public MP4Box
{
public:
	MP4FullBox(MP4FileClass &file, const char* type) : MP4Box(file, type)
	{
		m_flags = 0;
		m_version = 0;
	}

    uint8                    m_version;
    uint32                   m_flags;

    uint8_t GetVersion()
    {
        return m_version;
    }

    void SetVersion(uint8_t boxversion)
    {
        m_version = boxversion;
    }

    uint32_t GetFlags()
    {
        return m_flags;
    }

    void SetFlags(uint32_t boxflags)
    {
        m_flags = boxflags;
    }

	virtual void ReadProperties()
	{
		m_flags = m_File.ReadUInt32();
		m_version = (m_flags & 0xf0000000) >> 24;
		m_flags = m_flags & 0x0fffffff;
	}

    virtual void WriteProperties()
    {
        m_File.WriteUInt32((m_flags & 0x0fffffff) | ((uint32)m_version<<24));
    }

	virtual void DumpProperties(uint8_t indent, bool dumpImplicits)
	{
		MP4Box::DumpProperties(indent, dumpImplicits);
		osDump(indent, "Version: %8d(0x%8x)\n", m_version, m_version);
		osDump(indent, "Flags  : %d(0x%x)\n", m_flags, m_flags);
	}

private:
    MP4FullBox();
    MP4FullBox( const MP4FullBox &src );
    MP4FullBox &operator= ( const MP4FullBox &src );
};

// declare all the Box subclasses
// i.e. spare us Box_xxxx.h for all the Boxs
//
// The majority of Boxs just need their own constructor declared
// Some Boxs have a few special needs
// A small minority of Boxs need lots of special handling

class MP4RootBox : public MP4Box
{
public:
    MP4RootBox(MP4FileClass &file);
    void BeginWrite(bool use64 = false);
    void Write();
    void FinishWrite(bool use64 = false);

    void BeginOptimalWrite();
    void FinishOptimalWrite();

protected:
    uint32_t GetLastMdatIndex();
    void WriteBoxType(const char* type, bool onlyOne);

private:
    MP4RootBox();
    MP4RootBox( const MP4RootBox &src );
    MP4RootBox &operator= ( const MP4RootBox &src );

    MP4FtypBox*					m_rewrite_ftyp;
    uint64_t					m_rewrite_ftypPosition;
    MP4FreeBox*					m_rewrite_free;
    uint64_t					m_rewrite_freePosition;
};

/***********************************************************************
 * Common Box classes - standard for anything that just contains Boxs
 * and non-maleable properties, treftype and url
 ***********************************************************************/
class MP4StandardBox : public MP4Box {
public:
    MP4StandardBox(MP4FileClass &file, const char *name);
	virtual void ReadProperties();

	uint32                 m_bufferSizeDB;
	uint32                 m_avgBitrate;
	uint32                 m_maxBitrate;
	string                 m_base_url;
	uint64                 m_bytes;
	uint32                 m_milliSecs;
	uint32                 m_data_format;

	uint32                 m_granularity;
	uint32                 m_bytes32bits;
	uint64                 m_packets;
	uint32                 m_offset;
	uint32                 m_timeScale;
private:
    MP4StandardBox();
    MP4StandardBox( const MP4StandardBox &src );
    MP4StandardBox &operator= ( const MP4StandardBox &src );
};

class MP4TrefTypeBox : public MP4Box {
public:
    MP4TrefTypeBox(MP4FileClass &file, const char* type);
    void Read();
private:
    MP4TrefTypeBox();
    MP4TrefTypeBox( const MP4TrefTypeBox &src );
    MP4TrefTypeBox &operator= ( const MP4TrefTypeBox &src );
};

class MP4UrlBox : public MP4FullBox {
public:
    MP4UrlBox(MP4FileClass &file, const char *type="url ");
    void Read();
    void Write();
    string                  m_location;
private:
    MP4UrlBox();
    MP4UrlBox( const MP4UrlBox &src );
    MP4UrlBox &operator= ( const MP4UrlBox &src );
};

/***********************************************************************
 * Sound and Video Boxs - use the generic Boxs when possible
 * (MP4SoundBox and MP4VideoBox)
 ***********************************************************************/

typedef struct SampleEntry
{
	uint8					m_reserved[6];
	uint16					m_data_reference_index;
};

class MP4SoundBox : public MP4Box {
public:
    MP4SoundBox(MP4FileClass &file, const char *Boxid);
    void Generate();
    void Read();

	//SampleEntry entry;
	//const unsigned int(32)[2] reserved = 0;
	//template unsigned int(16) channelcount = 2;
	//template unsigned int(16) samplesize = 16;
	//unsigned int(16) pre_defined = 0;
	//const unsigned int(16) reserved = 0;
	//template unsigned int(32) samplerate = { default samplerate of media } << 16;
	SampleEntry						m_entry;
	uint32							m_reserved1[2];
	uint16							m_channelcount;
	uint16							m_samplesize;
	uint16							m_pre_defined;
	uint16							m_reserved2;
	uint32							m_samplerate;

	uint16 						m_soundVersion;
	uint16 						m_compressionId;
	uint16 						m_packetSize;
	uint32 						m_samplesPerPacket;
	uint32 						m_bytesPerPacket;
	uint32 						m_bytesPerFrame;
	uint32 						m_bytesPerSample;

	uint16 						m_reserved3;
	uint32 						m_reserved4;
	vector<uint8> 				m_soundVersion2Data;

protected:
    void AddProperties(uint8_t version);
private:
    MP4SoundBox();
    MP4SoundBox( const MP4SoundBox &src );
    MP4SoundBox &operator= ( const MP4SoundBox &src );
};

class MP4VideoBox : public MP4Box {
public:
    MP4VideoBox(MP4FileClass &file, const char *Boxid);
    void Generate();
private:
    MP4VideoBox();
    MP4VideoBox( const MP4VideoBox &src );
    MP4VideoBox &operator= ( const MP4VideoBox &src );
};

class MP4AmrBox : public MP4Box {
public:
    MP4AmrBox(MP4FileClass &file, const char *type);
    void Generate();
private:
    MP4AmrBox();
    MP4AmrBox( const MP4AmrBox &src );
    MP4AmrBox &operator= ( const MP4AmrBox &src );
};

// H.264 Boxs

class MP4Avc1Box : public MP4Box {
public:
    MP4Avc1Box(MP4FileClass &file);
    void Generate();
private:
    MP4Avc1Box();
    MP4Avc1Box( const MP4Avc1Box &src );
    MP4Avc1Box &operator= ( const MP4Avc1Box &src );
};

class MP4AvcCBox : public MP4Box {
public:
    MP4AvcCBox(MP4FileClass &file);
    void Generate();
    void Clone(MP4AvcCBox *dstBox);
private:
    MP4AvcCBox();
    MP4AvcCBox( const MP4AvcCBox &src );
    MP4AvcCBox &operator= ( const MP4AvcCBox &src );
};


class MP4D263Box : public MP4Box {
public:
    MP4D263Box(MP4FileClass &file);
    void Generate();
    void Write();
private:
    MP4D263Box();
    MP4D263Box( const MP4D263Box &src );
    MP4D263Box &operator= ( const MP4D263Box &src );
};

class MP4DamrBox : public MP4Box {
public:
    MP4DamrBox(MP4FileClass &file);
    void Generate();
private:
    MP4DamrBox();
    MP4DamrBox( const MP4DamrBox &src );
    MP4DamrBox &operator= ( const MP4DamrBox &src );
};

class MP4EncaBox : public MP4Box {
public:
    MP4EncaBox(MP4FileClass &file);
    void Generate();
private:
    MP4EncaBox();
    MP4EncaBox( const MP4EncaBox &src );
    MP4EncaBox &operator= ( const MP4EncaBox &src );
};

class MP4EncvBox : public MP4Box {
public:
    MP4EncvBox(MP4FileClass &file);
    void Generate();
private:
    MP4EncvBox();
    MP4EncvBox( const MP4EncvBox &src );
    MP4EncvBox &operator= ( const MP4EncvBox &src );
};

class MP4Mp4aBox : public MP4Box {
public:
    MP4Mp4aBox(MP4FileClass &file);
    void Generate();
private:
    MP4Mp4aBox();
    MP4Mp4aBox( const MP4Mp4aBox &src );
    MP4Mp4aBox &operator= ( const MP4Mp4aBox &src );
};

class MP4Ac3Box : public MP4Box {
public:
    MP4Ac3Box(MP4FileClass &file);
    void Generate();
private:
    MP4Ac3Box();
    MP4Ac3Box( const MP4Ac3Box &src );
    MP4Ac3Box &operator= ( const MP4Ac3Box &src );
};

class MP4DAc3Box : public MP4Box {
public:
    MP4DAc3Box(MP4FileClass &file);
    void Generate();
    void Dump(uint8_t indent, bool dumpImplicits);
private:
    MP4DAc3Box();
    MP4DAc3Box( const MP4DAc3Box &src );
    MP4DAc3Box &operator= ( const MP4DAc3Box &src );
};

class MP4Mp4sBox : public MP4Box {
public:
    MP4Mp4sBox(MP4FileClass &file);
    void Generate();
private:
    MP4Mp4sBox();
    MP4Mp4sBox( const MP4Mp4sBox &src );
    MP4Mp4sBox &operator= ( const MP4Mp4sBox &src );
};

class MP4Mp4vBox : public MP4Box {
public:
    MP4Mp4vBox(MP4FileClass &file);
    void Generate();
private:
    MP4Mp4vBox();
    MP4Mp4vBox( const MP4Mp4vBox &src );
    MP4Mp4vBox &operator= ( const MP4Mp4vBox &src );
};

   class MP4Tsc2Box : public MP4Box {
   public:
      MP4Tsc2Box(MP4FileClass &file);
      void Generate();
   private:
      MP4Tsc2Box();
      MP4Tsc2Box( const MP4Tsc2Box &src );
      MP4Tsc2Box &operator= ( const MP4Tsc2Box &src );
   };


class MP4S263Box : public MP4Box {
public:
    MP4S263Box(MP4FileClass &file);
    void Generate();
private:
    MP4S263Box();
    MP4S263Box( const MP4S263Box &src );
    MP4S263Box &operator= ( const MP4S263Box &src );
};



/************************************************************************
 * Specialized Boxs
 ************************************************************************/

class MP4DrefBox : public MP4FullBox {
public:
    MP4DrefBox(MP4FileClass &file);
    void Read();
    void DumpProperties(uint8_t indent, bool dumpImplicits);

    //unsigned int(32) entry_count;
    //for (i = 1; i <= entry_count; i++) {
    //    DataEntryBox(entry_version, entry_flags) data_entry;
    //}
    uint32                  m_entry_count;


private:
    MP4DrefBox();
    MP4DrefBox( const MP4DrefBox &src );
    MP4DrefBox &operator= ( const MP4DrefBox &src );
};

class MP4ElstBox : public MP4Box {
public:
    MP4ElstBox(MP4FileClass &file);
    void Generate();
    void Read();
protected:
    void AddProperties(uint8_t version);
private:
    MP4ElstBox();
    MP4ElstBox( const MP4ElstBox &src );
    MP4ElstBox &operator= ( const MP4ElstBox &src );
};

class MP4FreeBox : public MP4Box {
public:
    MP4FreeBox( MP4FileClass &file, const char* = NULL );
    void Read();
    void Write();
private:
    MP4FreeBox();
    MP4FreeBox( const MP4FreeBox &src );
    MP4FreeBox &operator= ( const MP4FreeBox &src );
};

class MP4FtypBox : public MP4Box {
public:
    MP4FtypBox(MP4FileClass &file);
    void Generate();
    void Read();

	virtual void ReadProperties();
	virtual void DumpProperties(uint8_t indent, bool dumpImplicits)
	{
		char tempStr[5] = { 0 };

		INT32TOSTR(m_majorBrand, tempStr);
		osDump(indent, "MajorBrand: %s(0x%x)\n", tempStr, m_majorBrand);
		osDump(indent, "MinorVersion: %d(0x%x)\n", m_minorVersion, m_minorVersion);

		for (uint32 i = 0; i < m_compatibleBrands.size(); i++)
		{
			INT32TOSTR(m_compatibleBrands[i], tempStr);
			osDump(indent, "MajorBrand: %s(0x%x)\n", tempStr, m_compatibleBrands[i]);
		}
	}


	// Each brand is a printable four - character code, registered with ISO, that identifies a precise specification.
	// major_brand ¨C is a brand identifier
	// minor_version ¨C is an informative integer for the minor version of the major brand
	// compatible_brands ¨C is a list, to the end of the box, of brands
	uint32					m_majorBrand;
	uint32					m_minorVersion;
	vector<uint32>			m_compatibleBrands;

private:
    MP4FtypBox();
    MP4FtypBox( const MP4FtypBox &src );
    MP4FtypBox &operator= ( const MP4FtypBox &src );
};

class MP4GminBox : public MP4Box {
public:
    MP4GminBox(MP4FileClass &file);
    void Generate();
private:
    MP4GminBox();
    MP4GminBox( const MP4GminBox &src );
    MP4GminBox &operator= ( const MP4GminBox &src );
};

typedef enum HANDLER_TYPE
{
	HANDLER_TYPE_UNKNOW  =0,
	HANDLER_TYPE_VIDEO,			//¡®vide¡¯ Video track
	HANDLER_TYPE_AUDIO,			//¡®soun¡¯ Audio track
	HANDLER_TYPE_HINT,			//¡®hint¡¯ Hint track
	HANDLER_TYPE_META,			//¡®meta¡¯ Timed Metadata track
	HANDLER_TYPE_AUXV			//¡®auxv¡¯ Auxiliary Video track
}HANDLER_TYPE;

class MP4HdlrBox : public MP4FullBox {
public:
    MP4HdlrBox(MP4FileClass &file);
    void Read();
	virtual void ReadProperties();
	virtual void DumpProperties(uint8_t indent, bool dumpImplicits);

	uint32					m_pre_defined;
	uint32					m_handler_type;
	uint32					m_reserved[3];
	string					m_name;

	//Handler type
	HANDLER_TYPE			m_Tracktype;

private:
    MP4HdlrBox();
    MP4HdlrBox( const MP4HdlrBox &src );
    MP4HdlrBox &operator= ( const MP4HdlrBox &src );
};

class MP4HinfBox : public MP4Box {
public:
    MP4HinfBox(MP4FileClass &file);
    void Generate();
private:
    MP4HinfBox();
    MP4HinfBox( const MP4HinfBox &src );
    MP4HinfBox &operator= ( const MP4HinfBox &src );
};

class MP4HntiBox : public MP4Box {
public:
    MP4HntiBox(MP4FileClass &file);
    void Read();
private:
    MP4HntiBox();
    MP4HntiBox( const MP4HntiBox &src );
    MP4HntiBox &operator= ( const MP4HntiBox &src );
};


class MP4MdatBox : public MP4Box {
public:
    MP4MdatBox(MP4FileClass &file);
    void Read();
    void Write();
private:
    MP4MdatBox();
    MP4MdatBox( const MP4MdatBox &src );
    MP4MdatBox &operator= ( const MP4MdatBox &src );
};

class MP4MdhdBox : public MP4FullBox {
public:
    MP4MdhdBox(MP4FileClass &file);
    void Generate();
    void Read();
	virtual void ReadProperties();
	virtual void DumpProperties(uint8_t indent, bool dumpImplicits);

	uint64					m_creationTime;
	uint64					m_modificationTime;
	uint32					m_timescale;
	uint64					m_duration;

	// ISO-639-2/T language code
	//bit(1) pad = 0;
	//unsigned int(5)[3] language;
	uint16					m_language;
	uint16					m_pre_defined;
	char					m_code[3];
private:
    MP4MdhdBox();
    MP4MdhdBox( const MP4MdhdBox &src );
    MP4MdhdBox &operator= ( const MP4MdhdBox &src );
};

class MP4MvhdBox : public MP4FullBox {
public:
    MP4MvhdBox(MP4FileClass &file);
    void Generate();
    void Read();

	virtual void ReadProperties();
	virtual void DumpProperties(uint8_t indent, bool dumpImplicits);

	//if (version == 1) {
	//	unsigned int(64) creation_time;
	//	unsigned int(64) modification_time;
	//	unsigned int(32) timescale;
	//	unsigned int(64) duration;
	//}
	//else { // version==0
	//	unsigned int(32) creation_time;
	//	unsigned int(32) modification_time;
	//	unsigned int(32) timescale;
	//	unsigned int(32) duration;
	//}
	uint64					m_creationTime;
	uint64					m_modificationTime;
	uint32					m_timescale;
	uint64					m_duration;

	uint32					m_rate;				// typically 1.0
	uint16					m_volume;			// typically, full volume
	uint16					m_reserved0;
	uint32					m_reserved[2];
	// { 0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000 };
	// unity matrix
	uint32                  m_matrix[9];
	uint32					m_pre_defined[6];
	uint32					m_next_track_ID;

private:
    MP4MvhdBox();
    MP4MvhdBox( const MP4MvhdBox &src );
    MP4MvhdBox &operator= ( const MP4MvhdBox &src );
};

class MP4OhdrBox : public MP4Box {
public:
    MP4OhdrBox(MP4FileClass &file);
    ~MP4OhdrBox();
    void Read();
private:
    MP4OhdrBox();
    MP4OhdrBox( const MP4OhdrBox &src );
    MP4OhdrBox &operator= ( const MP4OhdrBox &src );
};

class MP4RtpBox : public MP4Box {
public:
    MP4RtpBox(MP4FileClass &file);
    void Generate();
    void Read();
    void Write();

protected:
    void AddPropertiesStsdType();
    void AddPropertiesHntiType();

    void GenerateStsdType();
    void GenerateHntiType();

    void ReadStsdType();
    void ReadHntiType();

    void WriteHntiType();

private:
    MP4RtpBox();
    MP4RtpBox( const MP4RtpBox &src );
    MP4RtpBox &operator= ( const MP4RtpBox &src );
};

class MP4SdpBox : public MP4Box {
public:
    MP4SdpBox(MP4FileClass &file);
    void Read();
    void Write();
private:
    MP4SdpBox();
    MP4SdpBox( const MP4SdpBox &src );
    MP4SdpBox &operator= ( const MP4SdpBox &src );
};

// sdtp - Independent and Disposable Samples Box.
class MP4SdtpBox : public MP4FullBox {
public:
    MP4SdtpBox(MP4FileClass &file);
    void Read();

    // raw bytes; one byte for each sample.
    // number of bytes == stsz.sampleCount.
    int8& data;
private:
    MP4SdtpBox();
    MP4SdtpBox( const MP4SdtpBox &src );
    MP4SdtpBox &operator= ( const MP4SdtpBox &src );
};

class MP4SmiBox : public MP4Box {
public:
    MP4SmiBox(MP4FileClass &file);
    void Read();
private:
    MP4SmiBox();
    MP4SmiBox( const MP4SmiBox &src );
    MP4SmiBox &operator= ( const MP4SmiBox &src );
};

class MP4StblBox : public MP4Box {
public:
    MP4StblBox(MP4FileClass &file);
    void Generate();
private:
    MP4StblBox();
    MP4StblBox( const MP4StblBox &src );
    MP4StblBox &operator= ( const MP4StblBox &src );
};

class MP4StdpBox : public MP4Box {
public:
    MP4StdpBox(MP4FileClass &file);
    void Read();
private:
    MP4StdpBox();
    MP4StdpBox( const MP4StdpBox &src );
    MP4StdpBox &operator= ( const MP4StdpBox &src );
};

class MP4StscBox : public MP4Box {
public:
    MP4StscBox(MP4FileClass &file);
    void Read();
private:
    MP4StscBox();
    MP4StscBox( const MP4StscBox &src );
    MP4StscBox &operator= ( const MP4StscBox &src );
};

class MP4StsdBox : public MP4FullBox {
public:
    MP4StsdBox(MP4FileClass &file);
    void Read();
	virtual void ReadProperties();
	virtual void DumpProperties(uint8_t indent, bool dumpImplicits);

	uint32						m_entryCount;
private:
    MP4StsdBox();
    MP4StsdBox( const MP4StsdBox &src );
    MP4StsdBox &operator= ( const MP4StsdBox &src );
};

class MP4StszBox : public MP4Box {
public:
    MP4StszBox(MP4FileClass &file);
    void Read();
    void Write();
private:
    MP4StszBox();
    MP4StszBox( const MP4StszBox &src );
    MP4StszBox &operator= ( const MP4StszBox &src );
};

class MP4Stz2Box : public MP4Box {
public:
    MP4Stz2Box(MP4FileClass &file);
    void Read();
private:
    MP4Stz2Box();
    MP4Stz2Box( const MP4Stz2Box &src );
    MP4Stz2Box &operator= ( const MP4Stz2Box &src );
};

class MP4TextBox : public MP4Box {
public:
    MP4TextBox(MP4FileClass &file);
    void Generate();
    void Read();
protected:
    void AddPropertiesStsdType();
    void AddPropertiesGmhdType();

    void GenerateStsdType();
    void GenerateGmhdType();
private:
    MP4TextBox();
    MP4TextBox( const MP4TextBox &src );
    MP4TextBox &operator= ( const MP4TextBox &src );
};

class MP4Tx3gBox : public MP4Box {
public:
    MP4Tx3gBox(MP4FileClass &file);
    void Generate();
private:
    MP4Tx3gBox();
    MP4Tx3gBox( const MP4Tx3gBox &src );
    MP4Tx3gBox &operator= ( const MP4Tx3gBox &src );
};

class MP4FtabBox : public MP4Box {
public:
    MP4FtabBox(MP4FileClass &file);
private:
    MP4FtabBox();
    MP4FtabBox( const MP4FtabBox &src );
    MP4FtabBox &operator= ( const MP4FtabBox &src );
};

class MP4TfhdBox : public MP4Box {
public:
    MP4TfhdBox(MP4FileClass &file);
    void Read();
protected:
    void AddProperties(uint32_t flags);
private:
    MP4TfhdBox();
    MP4TfhdBox( const MP4TfhdBox &src );
    MP4TfhdBox &operator= ( const MP4TfhdBox &src );
};

class MP4TkhdBox : public MP4FullBox {
public:
    MP4TkhdBox(MP4FileClass &file);
    void Generate();
    void Read();

	virtual void ReadProperties();
	virtual void DumpProperties(uint8_t indent, bool dumpImplicits);

protected:
    //The default value of the track header flags for media tracks is 7 (track_enabled, track_in_movie,
    //track_in_preview).If in a presentation all tracks have neither track_in_movie nor track_in_preview set, then all
    //tracks shall be treated as if both flags were set on all tracks.Hint tracks should have the track header flags set
    //to 0, so that they are ignored for local playback and preview.
    //if (version == 1) {
    //    unsigned int(64) creation_time;
    //    unsigned int(64) modification_time;
    //    unsigned int(32) track_ID;
    //    const unsigned int(32) reserved = 0;
    //    unsigned int(64) duration;
    //}
    //else { // version==0
    //    unsigned int(32) creation_time;
    //    unsigned int(32) modification_time;
    //    unsigned int(32) track_ID;
    //    const unsigned int(32) reserved = 0;
    //    unsigned int(32) duration;
    //}

    uint64                  m_creationTime;
    uint64                  m_modificationTime;
    uint32                  m_trackId;
    uint32                  m_reserved0;
    uint64                  m_duration;
 
    uint32                  m_reserved1[2];
    int16                   m_layer;
    int16                   m_alternate_group;

    //{ if track_is_audio 0x0100 else 0 };
    int16                   m_volume;
    uint16                  m_reserved2;

    // { 0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000 };
    // unity matrix
    int32                   m_matrix[9];

    uint32                  m_width;
    uint32                  m_height;

private:
    MP4TkhdBox();
    MP4TkhdBox( const MP4TkhdBox &src );
    MP4TkhdBox &operator= ( const MP4TkhdBox &src );
};

class MP4TrunBox : public MP4Box {
public:
    MP4TrunBox(MP4FileClass &file);
    void Read();
protected:
    void AddProperties(uint32_t flags);
private:
    MP4TrunBox();
    MP4TrunBox( const MP4TrunBox &src );
    MP4TrunBox &operator= ( const MP4TrunBox &src );
};

class MP4UdtaBox : public MP4Box {
public:
    MP4UdtaBox(MP4FileClass &file);
    void Read();
private:
    MP4UdtaBox();
    MP4UdtaBox( const MP4UdtaBox &src );
    MP4UdtaBox &operator= ( const MP4UdtaBox &src );
};

class MP4UrnBox : public MP4FullBox {
public:
    MP4UrnBox(MP4FileClass &file);
    void Read();

    string                  m_name;
    string                  m_location;
private:
    MP4UrnBox();
    MP4UrnBox( const MP4UrnBox &src );
    MP4UrnBox &operator= ( const MP4UrnBox &src );
};

class MP4VmhdBox : public MP4FullBox {
public:
    MP4VmhdBox(MP4FileClass &file);
    void Generate();
private:
    MP4VmhdBox();
    MP4VmhdBox( const MP4VmhdBox &src );
    MP4VmhdBox &operator= ( const MP4VmhdBox &src );
};

class MP4SmhdBox : public MP4FullBox {
public:
	MP4SmhdBox(MP4FileClass &file)
		: MP4FullBox(file, "smhd")
	{
		m_balance = 0;
		m_reserved = 0;
	}


	void Read()
	{
		// read version and flags */
		MP4FullBox::ReadProperties();
		m_balance = m_File.ReadUInt16();

		Skip(); // to end of atom
	}

private:
	uint16					m_balance;
	uint16					m_reserved;
	MP4SmhdBox();
	MP4SmhdBox(const MP4SmhdBox &src);
	MP4SmhdBox &operator= (const MP4SmhdBox &src);
};

class MP4NmhdBox : public MP4FullBox {
public:
	MP4NmhdBox(MP4FileClass &file)
		: MP4FullBox(file, "nmhd")
	{

	}

private:
	MP4NmhdBox();
	MP4NmhdBox(const MP4NmhdBox &src);
	MP4NmhdBox &operator= (const MP4NmhdBox &src);
};

class MP4HmhdBox : public MP4FullBox {
public:
	MP4HmhdBox(MP4FileClass &file)
		: MP4FullBox(file, "hmhd")
	{
		m_maxPDUsize = 0;
		m_avgPDUsize = 0;
		m_maxbitrate = 0;
		m_avgbitrate = 0;
	}


	void Read()
	{
		// read version and flags */
		MP4FullBox::ReadProperties();
		m_maxPDUsize = m_File.ReadUInt16();
		m_avgPDUsize = m_File.ReadUInt16();
		m_maxbitrate = m_File.ReadUInt32();
		m_avgbitrate = m_File.ReadUInt32();
		Skip(); // to end of atom
	}

	uint16 					m_maxPDUsize;
	uint16 					m_avgPDUsize;
	uint32 					m_maxbitrate;
	uint32 					m_avgbitrate;

private:
	uint16					m_reserved;
	MP4HmhdBox();
	MP4HmhdBox(const MP4HmhdBox &src);
	MP4HmhdBox &operator= (const MP4HmhdBox &src);
};

class MP4HrefBox : public MP4Box {
public:
    MP4HrefBox(MP4FileClass &file);
    void Generate(void);
private:
    MP4HrefBox();
    MP4HrefBox( const MP4HrefBox &src );
    MP4HrefBox &operator= ( const MP4HrefBox &src );
};

class MP4PaspBox : public MP4Box {
public:
    MP4PaspBox(MP4FileClass &file);
    void Generate();
private:
    MP4PaspBox();
    MP4PaspBox( const MP4PaspBox &src );
    MP4PaspBox &operator= ( const MP4PaspBox &src );
};

class MP4ColrBox : public MP4Box {
public:
    MP4ColrBox(MP4FileClass &file);
    void Generate();
private:
    MP4ColrBox();
    MP4ColrBox( const MP4ColrBox &src );
    MP4ColrBox &operator= ( const MP4ColrBox &src );
};

class IPodUUIDBox : public MP4Box {
public:
    IPodUUIDBox(MP4FileClass &file);
    uint32_t        value;
private:
    IPodUUIDBox();
    IPodUUIDBox( const IPodUUIDBox &src );
    IPodUUIDBox &operator= ( const IPodUUIDBox &src );
};

/*! Nero Chapter List.
 * This Box defines the structure of a Nero chapter list.
 * Although it is not completely clear if this structure is
 * correct it is complete enough to successfully read and write
 * the chapter list so that even Nero's software accepts it.
 *
 * The assumed format is as follows:
 * - int8("version")
 * - int32("flags")
 * - int8("reserved", 1)
 * - int32("chaptercount")\n
 * - MP4TableProperty("chapters", "ref to chaptercount");
 *     - MP4Integer64Property("starttime")\n
 *       The start time of the chapter expressed in 100 nanosecond units
 *     - MP4StringProperty("title", true)\n
 *       The title of the chapter encoded in UTF-8
 *
 * The chapter title only accepts strings of 255 bytes so if a string
 * only contains simple (two-byte) UTF-8 chars the maximum length is
 * 127 chars.
 */
class MP4ChplBox : public MP4Box {
public:
    MP4ChplBox(MP4FileClass &file);
    void Generate();
private:
    MP4ChplBox();
    MP4ChplBox( const MP4ChplBox &src );
    MP4ChplBox &operator= ( const MP4ChplBox &src );
};

///////////////////////////////////////////////////////////////////////////////

/// iTMF hdlr-Box.
class MP4ItmfHdlrBox : public MP4FullBox
{
public:
    MP4ItmfHdlrBox(MP4FileClass &file);
    void Read();

    int32& reserved1;
    int8&     handlerType;
    int8&     reserved2;
    int8&     name;
private:
    MP4ItmfHdlrBox();
    MP4ItmfHdlrBox( const MP4ItmfHdlrBox &src );
    MP4ItmfHdlrBox &operator= ( const MP4ItmfHdlrBox &src );
};

/// iTMF item-Box.
class MP4ItemBox : public MP4Box
{
public:
    MP4ItemBox( MP4FileClass &file, const char* type );
private:
    MP4ItemBox();
    MP4ItemBox( const MP4ItemBox &src );
    MP4ItemBox &operator= ( const MP4ItemBox &src );
};

/// iTMF meaning-Box.
class MP4MeanBox : public MP4FullBox
{
public:
    MP4MeanBox(MP4FileClass &file);
    void Read();

    int8& value;
private:
    MP4MeanBox();
    MP4MeanBox( const MP4MeanBox &src );
    MP4MeanBox &operator= ( const MP4MeanBox &src );
};

/// iTMF name-Box.
class MP4NameBox : public MP4FullBox
{
public:
    MP4NameBox(MP4FileClass &file);
    void Read();

    int8& value;
private:
    MP4NameBox();
    MP4NameBox( const MP4NameBox &src );
    MP4NameBox &operator= ( const MP4NameBox &src );
};

/// iTMF data-Box.
class MP4DataBox : public MP4Box
{
public:
    MP4DataBox(MP4FileClass &file);
    void Read();

    int16& typeReserved;
    int8&  typeSetIdentifier;
    int32& typeCode;
    int32& locale;
    int8&     metadata;
private:
    MP4DataBox();
    MP4DataBox( const MP4DataBox &src );
    MP4DataBox &operator= ( const MP4DataBox &src );
};

///////////////////////////////////////////////////////////////////////////////

/// QTFF udta data element-Box.
class MP4UdtaElementBox : public MP4Box
{
public:
    MP4UdtaElementBox( MP4FileClass &file, const char* type );
    void Read();

    int8& value;
private:
    MP4UdtaElementBox();
    MP4UdtaElementBox( const MP4UdtaElementBox &src );
    MP4UdtaElementBox &operator= ( const MP4UdtaElementBox &src );
};


#endif

