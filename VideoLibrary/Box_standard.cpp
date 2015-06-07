#include "AllMP4Box.h"

MP4StandardBox::MP4StandardBox (MP4FileClass &file, const char *type) : MP4Box(file, type)
{
	// The following if-else structure is a big one;
	// This is just a temp solution, to define various box structures; ported ftom libmp4v2.
	// It intends to cover all other types of boxes.
    if (BoxID(type) == BoxID("bitr")) {
        /* 0 */
            uint32 m_avgBitrate;

        /* 1 */
            uint32 m_maxBitrate;

    } else if (BoxID(type) == BoxID("btrt")) {
        uint32 m_bufferSizeDB; /* 0 */
        uint32 m_avgBitrate;   /* 1 */
        uint32 m_maxBitrate;   /* 2 */
    } else if (BoxID(type) == BoxID("burl")) {
        string m_base_url;

    }
    else if (BoxID(type) == BoxID("dinf")) {
        ExpectChildBox("dref", Required, OnlyOne);

    } else if (BoxID(type) == BoxID("dimm")) {
        // bytes of immediate data
            uint64 m_bytes;

    } else if (BoxID(type) == BoxID("dmax")) {
        // max packet duration
            uint32 m_milliSecs;

    } else if (BoxID(type) == BoxID("dmed")) {
        // bytes sent from media data
            uint64 m_bytes;

    } else if (BoxID(type) == BoxID("drep")) {
        // bytes of repeated data
            uint64 m_bytes;
        /*
         * e???
         */
    } else if (BoxID(type) == BoxID("edts")) {
        ExpectChildBox("elst", Required, OnlyOne);

    }
    else if (BoxID(type) == BoxID("esds"))
    {
        osAssert(0);
        /// TODO
        //AddVersionAndFlags();
        //AddProperty(
        //    new MP4DescriptorProperty(*this, NULL, MP4ESDescrTag, 0,
        //                              Required, OnlyOne));

    } else if (BoxID(type) == BoxID("frma")) {
 
            uint32 m_data_format;
    } else if (BoxID(type) == BoxID("gmhd")) {
        ExpectChildBox("gmin", Required, OnlyOne);
        ExpectChildBox("tmcd", Optional, OnlyOne);
        ExpectChildBox("text", Optional, OnlyOne);
    }
    else if (BoxID(type) == BoxID("hmhd"))
    {
        osAssert(0);
        /// TODO
        //AddVersionAndFlags();

        //AddProperty(new MP4Integer16Property(*this, "maxPduSize;
        //AddProperty(new MP4Integer16Property(*this, "avgPduSize;
        //AddProperty(uint32 m_maxBitRate;
        //AddProperty(uint32 m_avgBitRate;
        //AddProperty(uint32 m_slidingAvgBitRate;
        /*
         * i???
         */
    } else if (BoxID(type) == BoxID("iKMS")) {
        osAssert(0);
        /// TODO
        //AddVersionAndFlags(); /* 0, 1 */
        //MP4StringProperty* pProp = new MP4StringProperty(*this, "kms_URI");
        //AddProperty(pProp); /* 2 */

    } else if (BoxID(type) == BoxID("iSFM")) {
        osAssert(0);
        /// TODO
        //AddVersionAndFlags(); /* 0, 1 */
        ///* 2 */
        //    new MP4BitfieldProperty(*this, "selective-encryption", 1));
        ///* 3 */
        //    new MP4BitfieldProperty(*this, "reserved", 7));
        ///* 4 */
        //    new MP4Integer8Property(*this, "key-indicator-length;
        ///* 5 */
        //    new MP4Integer8Property(*this, "IV-length;

    } else if (BoxID(type) == BoxID("ilst")) {
        ExpectChildBox("\251nam", Optional, OnlyOne); /* name */
        ExpectChildBox("\251ART", Optional, OnlyOne); /* artist */
        ExpectChildBox("\251wrt", Optional, OnlyOne); /* writer */
        ExpectChildBox("\251alb", Optional, OnlyOne); /* album */
        ExpectChildBox("\251day", Optional, OnlyOne); /* date */
        ExpectChildBox("\251too", Optional, OnlyOne); /* tool */
        ExpectChildBox("\251cmt", Optional, OnlyOne); /* comment */
        ExpectChildBox("\251gen", Optional, OnlyOne); /* custom genre */
        ExpectChildBox("trkn", Optional, OnlyOne); /* tracknumber */
        ExpectChildBox("disk", Optional, OnlyOne); /* disknumber */
        ExpectChildBox("gnre", Optional, OnlyOne); /* genre (ID3v1 index + 1) */
        ExpectChildBox("cpil", Optional, OnlyOne); /* compilation */
        ExpectChildBox("tmpo", Optional, OnlyOne); /* BPM */
        ExpectChildBox("covr", Optional, OnlyOne); /* cover art */
        ExpectChildBox("aART", Optional, OnlyOne); /* album artist */
        ExpectChildBox("----", Optional, Many); /* ---- free form */
        ExpectChildBox("pgap", Optional, OnlyOne); /* part of gapless album */
        ExpectChildBox("tvsh", Optional, OnlyOne); /* TV show */
        ExpectChildBox("tvsn", Optional, OnlyOne); /* TV season */
        ExpectChildBox("tven", Optional, OnlyOne); /* TV episode number */
        ExpectChildBox("tvnn", Optional, OnlyOne); /* TV network name */
        ExpectChildBox("tves", Optional, OnlyOne); /* TV epsidoe */
        ExpectChildBox("desc", Optional, OnlyOne); /* description */
        ExpectChildBox("ldes", Optional, OnlyOne); /* long description */
        ExpectChildBox("soal", Optional, OnlyOne); /* sort album */
        ExpectChildBox("soar", Optional, OnlyOne); /* sort artist */
        ExpectChildBox("soaa", Optional, OnlyOne); /* sort album artist */
        ExpectChildBox("sonm", Optional, OnlyOne); /* sort name */
        ExpectChildBox("soco", Optional, OnlyOne); /* sort composer */
        ExpectChildBox("sosn", Optional, OnlyOne); /* sort show */
        ExpectChildBox("hdvd", Optional, OnlyOne); /* HD video */
        ExpectChildBox("©enc", Optional, OnlyOne); /* Encoded by */
        ExpectChildBox("pcst", Optional, OnlyOne); /* Podcast flag */
        ExpectChildBox("keyw", Optional, OnlyOne); /* Keywords (for podcasts?) */
        ExpectChildBox("catg", Optional, OnlyOne); /* Category (for podcasts?) */
        ExpectChildBox("purl", Optional, OnlyOne); /* Podcast URL */
        ExpectChildBox("egid", Optional, OnlyOne); /* Podcast episode global unique ID */
        ExpectChildBox("rtng", Optional, OnlyOne); /* Content Rating */
        ExpectChildBox("stik", Optional, OnlyOne); /* MediaType */
        ExpectChildBox("\251grp", Optional, OnlyOne); /* Grouping */
        ExpectChildBox("\251lyr", Optional, OnlyOne); /* Lyrics */
        ExpectChildBox("cprt", Optional, OnlyOne); /* Copyright */
        ExpectChildBox("apID", Optional, OnlyOne); /* iTunes Account */
        ExpectChildBox("akID", Optional, OnlyOne); /* iTunes Account Type */
        ExpectChildBox("sfID", Optional, OnlyOne); /* iTunes Country */
        ExpectChildBox("cnID", Optional, OnlyOne); /* Content ID */
        ExpectChildBox("atID", Optional, OnlyOne); /* Artist ID */
        ExpectChildBox("plID", Optional, OnlyOne); /* Playlist ID */
        ExpectChildBox("geID", Optional, OnlyOne); /* Genre ID */
        ExpectChildBox("cmID", Optional, OnlyOne); /* Composer ID */
        ExpectChildBox("xid ", Optional, OnlyOne); /* XID */

    }  else if (BoxID(type) == BoxID("imif")) {
        osAssert(0);
        /// TODO
        //AddVersionAndFlags();
        //AddProperty(new MP4DescriptorProperty(*this, "ipmp_desc", MP4IPMPDescrTag,
        //                                      MP4IPMPDescrTag, Required, Many));
    } else if (BoxID(type) == BoxID("iods")) {
        osAssert(0);
        /// TODO
        //AddVersionAndFlags();
        //AddProperty(
        //    new MP4DescriptorProperty(*this, NULL, MP4FileIODescrTag,
        //                              MP4FileODescrTag,
        //                              Required, OnlyOne));
        /*
         * m???
         */
    } else if (BoxID(type) == BoxID("maxr")) {
        uint32 m_granularity;
        uint32 m_bytes;

    } else if (BoxID(type) == BoxID("mdia")) {
        ExpectChildBox("mdhd", Required, OnlyOne);
        ExpectChildBox("hdlr", Required, OnlyOne);
        ExpectChildBox("minf", Required, OnlyOne);

    } else if (BoxID(type) == BoxID("meta")) { // iTunes
		osAssert(0);
        //AddVersionAndFlags(); /* 0, 1 */
        //ExpectChildBox("hdlr", Required, OnlyOne);
        //ExpectChildBox("ilst", Required, OnlyOne);

    } else if (BoxID(type) == BoxID("mfhd")) {
        osAssert(0);
        /// TODO
        //AddVersionAndFlags();   /* 0, 1 */
        ///* 2 */
        //    uint32 m_sequenceNumber;

    } else if (BoxID(type) == BoxID("minf")) {
        ExpectChildBox("vmhd", Optional, OnlyOne);
        ExpectChildBox("smhd", Optional, OnlyOne);
        ExpectChildBox("hmhd", Optional, OnlyOne);
        ExpectChildBox("nmhd", Optional, OnlyOne);
        ExpectChildBox("gmhd", Optional, OnlyOne);
        ExpectChildBox("dinf", Required, OnlyOne);
        ExpectChildBox("stbl", Required, OnlyOne);

    } else if (BoxID(type) == BoxID("moof")) {
        ExpectChildBox("mfhd", Required, OnlyOne);
        ExpectChildBox("traf", Optional, Many);

    } else if (BoxID(type) == BoxID("moov")) {
        ExpectChildBox("mvhd", Required, OnlyOne);
        ExpectChildBox("iods", Optional, OnlyOne);
        ExpectChildBox("trak", Required, Many);
        ExpectChildBox("udta", Optional, Many);
        ExpectChildBox("mvex", Optional, OnlyOne);

    } else if (BoxID(type) == BoxID("mvex")) {
        ExpectChildBox("trex", Required, Many);

        /*
         * n???
         */
    } else if (BoxID(type) == BoxID("nmhd")) {
        osAssert(0);
        /// TODO
        // AddVersionAndFlags();

    } else if (BoxID(type) == BoxID("nump")) {
        // packets sent
            uint64 m_packets;

    } else if (BoxID(type) == BoxID("odkm")) {
        osAssert(0);
        /// TODO
        //AddVersionAndFlags();
        //ExpectChildBox("ohdr", Required, OnlyOne);
        /*
         * p???
         */
    } else if (BoxID(type) == BoxID("payt")) {
        osAssert(0);
        /// TODO
        //AddProperty(uint32 m_payloadNumber;
        //AddProperty(new MP4StringProperty(*this, "rtpMap", Counted));

    } else if (BoxID(type) == BoxID("pinf")) {
        ExpectChildBox("frma", Required, OnlyOne);
    } else if (BoxID(type) == BoxID("pmax")) {
        // max packet size
            uint32 m_bytes;
    } else if (BoxID(type) == BoxID("schi")) {
        // not sure if this is child boxs or table of boxes
        // get clarification on spec 9.1.2.5
        ExpectChildBox("odkm", Optional, OnlyOne);
        ExpectChildBox("iKMS", Optional, OnlyOne);
        ExpectChildBox("iSFM", Optional, OnlyOne);

    } else if (BoxID(type) == BoxID("schm")) {
        osAssert(0);
        /// TODO
        //AddVersionAndFlags(); /* 0, 1 */
        ///* 2 */
        //    uint32 m_scheme_type;
        ///* 3 */
        //    uint32 m_scheme_version;
        //// browser URI if flags set, TODO

    } else if (BoxID(type) == BoxID("sinf")) {
        ExpectChildBox("frma", Required, OnlyOne);
        ExpectChildBox("imif", Optional, OnlyOne);
        ExpectChildBox("schm", Optional, OnlyOne);
        ExpectChildBox("schi", Optional, OnlyOne);

    } else if (BoxID(type) == BoxID("smhd")) {
        osAssert(0);
        /// TODO
        //AddVersionAndFlags();
        //AddReserved(*this, "reserved", 4);

    } else if (BoxID(type) == BoxID("snro")) {
        uint32 m_offset;
    } else if (BoxID(type) == BoxID("stsh")) {
        osAssert(0);
        /// TODO
        //AddVersionAndFlags();

        //MP4Integer32Property* pCount = uint32 m_entryCount");
        //AddProperty(pCount);

        //MP4TableProperty* pTable = new MP4TableProperty(*this, "entries", pCount);
        //AddProperty(pTable);

        //pTable->AddProperty(new MP4Integer32Property(pTable->GetParentBox(), "shadowedSampleNumber;
        //pTable->AddProperty(new MP4Integer32Property(pTable->GetParentBox(), "syncSampleNumber;

    } else if (BoxID(type) == BoxID("tims")) {

            uint32 m_timeScale;

    } else if (BoxID(type) == BoxID("tmin")) {
        // min relative xmit time
            uint32 m_milliSecs;

    } else if (BoxID(type) == BoxID("tmax")) {
        // max relative xmit time
            uint32 m_milliSecs;

    } else if (BoxID(type) == BoxID("traf")) {
        ExpectChildBox("tfhd", Required, OnlyOne);
        ExpectChildBox("trun", Optional, Many);

    } else if (BoxID(type) == BoxID("trak")) {
        ExpectChildBox("tkhd", Required, OnlyOne);
        ExpectChildBox("tref", Optional, OnlyOne);
        ExpectChildBox("edts", Optional, OnlyOne);
        ExpectChildBox("mdia", Required, OnlyOne);
        ExpectChildBox("udta", Optional, Many);

    } else if (BoxID(type) == BoxID("tref")) {
        ExpectChildBox("chap", Optional, OnlyOne);
        ExpectChildBox("dpnd", Optional, OnlyOne);
        ExpectChildBox("hint", Optional, OnlyOne);
        ExpectChildBox("ipir", Optional, OnlyOne);
        ExpectChildBox("mpod", Optional, OnlyOne);
        ExpectChildBox("sync", Optional, OnlyOne);

    } else if (BoxID(type) == BoxID("trex")) {
        osAssert(0);
        /// TODO
        //AddVersionAndFlags();   /* 0, 1 */
        ///* 2 */
        //    uint32 m_trackId;
        ///* 3 */
        //    uint32 m_defaultSampleDesriptionIndex;
        ///* 4 */
        //    uint32 m_defaultSampleDuration;
        ///* 5 */
        //    uint32 m_defaultSampleSize;
        ///* 6 */
        //    uint32 m_defaultSampleFlags;

    } else if (BoxID(type) == BoxID("trpy") ||
               BoxID(type) == BoxID("tpyl")) {
        // bytes sent including RTP headers
            uint64 m_bytes;

    } else if (BoxID(type) == BoxID("tsro")) {
        uint32 m_offset;
    } else if (BoxID(type) == BoxID("wave")) {
        ExpectChildBox("esds", Required, OnlyOne);
    } else {
        /*
         * default - unknown type
         */
        osAssert(!"Unsupported Box Type. TODO::");
        osLog(LOG_ERROR, "Unsupported Box Type: %s", type);
        SetUnknownType(true);
    }

	m_bufferSizeDB = 0;
	m_avgBitrate = 0;
	m_maxBitrate = 0;
	m_base_url.clear();
	m_bytes = 0;
	m_milliSecs = 0;
	m_data_format = 0;
	m_granularity = 0;
	m_bytes32bits = 0;
	m_packets = 0;
	m_offset = 0;
	m_timeScale = 0;
}


void MP4StandardBox::ReadProperties()
{
	if (BoxID(m_type) == BoxID("bitr")) {
		/* 0 */
		m_avgBitrate = m_File.ReadUInt32();

		/* 1 */
		m_maxBitrate = m_File.ReadUInt32();

	}
	else if (BoxID(m_type) == BoxID("btrt")) {
		m_bufferSizeDB = m_File.ReadUInt32(); /* 0 */
		m_avgBitrate = m_File.ReadUInt32();   /* 1 */
		m_maxBitrate = m_File.ReadUInt32();   /* 2 */
	}
	else if (BoxID(m_type) == BoxID("burl")) {
		// read a null terminated string
		char * pStr = m_File.ReadString();
		osAssert(pStr);
		if (pStr)
		{
			m_base_url.append(pStr);
		}
		m_File.FreeString(pStr);
	}

	else if (BoxID(m_type) == BoxID("dimm")) {
		// bytes of immediate data
		m_bytes = m_File.ReadUInt64();

	}
	else if (BoxID(m_type) == BoxID("dmax")) {
		// max packet duration
		m_milliSecs = m_File.ReadUInt32();

	}
	else if (BoxID(m_type) == BoxID("dmed")) {
		// bytes sent from media data
		m_bytes = m_File.ReadUInt64();

	}
	else if (BoxID(m_type) == BoxID("drep")) {
		// bytes of repeated data
		m_bytes = m_File.ReadUInt64();
		/*
		* e???
		*/
	}
	else if (BoxID(m_type) == BoxID("frma")) {

		m_data_format = m_File.ReadUInt32();
	}

	else if (BoxID(m_type) == BoxID("maxr")) {
		m_granularity = m_File.ReadUInt32();
		m_bytes32bits = m_File.ReadUInt32();

	}

	else if (BoxID(m_type) == BoxID("nump")) {
		// packets sent
		m_packets = m_File.ReadUInt64();

	}

	else if (BoxID(m_type) == BoxID("pmax")) {
		// max packet size
		m_bytes32bits = m_File.ReadUInt32();
	}

	else if (BoxID(m_type) == BoxID("snro")) {
		m_offset = m_File.ReadUInt32();

	}

	else if (BoxID(m_type) == BoxID("tims")) {

		m_timeScale = m_File.ReadUInt32();

	}
	else if (BoxID(m_type) == BoxID("tmin")) {
		// min relative xmit time
		m_milliSecs = m_File.ReadUInt32();

	}
	else if (BoxID(m_type) == BoxID("tmax")) {
		// max relative xmit time
		m_milliSecs = m_File.ReadUInt32();

	}

	else if (BoxID(m_type) == BoxID("trpy") ||
		BoxID(m_type) == BoxID("tpyl")) {
		// bytes sent including RTP headers
		m_bytes = m_File.ReadUInt32();

	}
	else if (BoxID(m_type) == BoxID("tsro")) {
		m_offset = m_File.ReadUInt32();
	}
	else {
		/*
		* default - unknown type
		*/
		// osAssert(0);
	}
}