#include "MpegTS.h"
#include "os.h"

/* The section length is 12 bits. The first 2 are set to 0, the remaining
* 10 bits should not exceed 1021. */
#define SECTION_LENGTH 1020

static const uint32_t crc_table[256] =
{
    0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9,
    0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
    0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
    0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9,
    0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
    0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011,
    0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
    0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039,
    0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
    0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81,
    0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
    0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49,
    0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
    0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1,
    0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
    0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE,
    0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
    0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16,
    0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA,
    0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE,
    0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
    0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066,
    0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
    0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E,
    0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
    0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6,
    0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
    0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E,
    0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
    0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686,
    0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
    0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637,
    0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
    0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F,
    0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
    0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47,
    0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
    0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
    0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
    0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7,
    0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
    0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F,
    0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
    0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7,
    0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
    0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F,
    0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
    0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640,
    0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
    0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8,
    0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
    0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30,
    0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
    0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088,
    0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
    0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0,
    0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
    0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18,
    0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
    0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0,
    0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
    0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668,
    0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};

uint32_t crc_32(uint8_t *bytes, int length)
{
    uint32_t crc = 0xffffffff;

    for (int i = 0; i < length; i++)
    {
        int idx = ((crc >> 24) ^ bytes[i]) & 0xff;
        crc = crc_table[idx] ^ (crc << 8);
    }
    return crc;
}

static INLINE void put16(uint8_t **q_ptr, int val)
{
    uint8_t *q;
    q = *q_ptr;
    *q++ = val >> 8;
    *q++ = val;
    *q_ptr = q;
}

// 32bytes header
uint32_t tsWritePacketHeader(uint8 * pBuf, uint8 payload_unit_start, uint8 pid, uint8 adapt_field, uint8 * cc)
{
    ts_header * pHeader = (ts_header *)pBuf;
    pHeader->sync_byte =  0x47;                 // sync byte
    pHeader->transport_error_indicator = 0;
    pHeader->payload_unit_start_indicator = payload_unit_start;
    pHeader->transport_priority = 0;
    pHeader->PID = pid;
    pHeader->transport_scrambling_control = 0;
    pHeader->adaptation_field_control = adapt_field & 0x03;
     
    if (adapt_field == ADAPT_FIELD_ONLY)
    {
        pHeader->continuity_counter = (*cc - 1) & 0xf;     // continuity counter
    }
    else
    {
        pHeader->continuity_counter = (*cc)++ & 0xf;        // continuity counter
    }

    return sizeof(ts_header);
}

// TS_packet = 32bytes_header + optional_adaptation_field + payload + "0xff"
// buf = optional_adaptation_field + payload
void tsWriteSection(MpegTSSection *s, uint8_t *buf, uint32 len)
{
    unsigned int crc;
    unsigned char packet[TSPKT_LENGTH];
    const unsigned char *buf_ptr;
    unsigned char *q;
    int first, b, len1, left;

    crc = endian_fix32(crc_32(buf, len - 4));

    buf[len - 4] = (crc >> 24) & 0xff;
    buf[len - 3] = (crc >> 16) & 0xff;
    buf[len - 2] = (crc >> 8) & 0xff;
    buf[len - 1] = crc & 0xff;

    /* send each packet */
    buf_ptr = buf;
    while (len > 0)
    {
        first = buf == buf_ptr;
        q = packet;
        *q++ = 0x47;
        b = s->pid >> 8;
        if (first)  b |= 0x40;
        *q++ = b;
        *q++ = s->pid;
        s->cc = s->cc + 1 & 0xf;
        *q++ = 0x10 | s->cc;
        if (first)  *q++ = 0; /* 0 offset */
        len1 = TSPKT_LENGTH - (q - packet);
        if (len1 > len)
        {
            len1 = len;
        }
        memcpy(q, buf_ptr, len1);
        q += len1;
        /* add known padding data */
        left = TSPKT_LENGTH - (q - packet);
        if (left > 0)
        {
            memset(q, 0xff, left);
        }

        buf_ptr += len1;
        len -= len1;
    }
}


// For sdt/pat/pmt. Max length 1024 bytes.
// Table_section = 3_bytes_table_header + 5_bytes_Syntax_section + N_bytes_table_data + crc32
//Table ID	                    8	Table Identifier, that defines the structure of the syntax section and other contained data.
//Section syntax indicator	    1	A flag that indicates if the syntax section follows the section length.The PAT, PMT, and CAT all set this to 1.
//Private bit	                1	The PAT, PMT, and CAT all set this to 0. Other tables set this to 1.
//Reserved bits	                2	Set to 0x03 (all bits on)
//Section length unused bits	2	Set to 0 (all bits off)
//Section length	            10	The number of bytes that follow for the syntax section(with CRC value) and / or table data.These bytes must not exceed a value of 1021.
//// above 3 bytes table header /////
//Syntax section / Table data	N * 8	When the section length is non - zero, this is the section length number of syntax and data bytes.
//
// for PAT/PMT/SDT, synctax+data. 5_bytes_syntax.
//
//Table ID extension	        16	Informational only identifier.The PAT uses this for the transport stream identifier and the PMT uses this for the Program number.
//Reserved bits	                2	Set to 0x03 (all bits on)
//Version number	            5	Syntax version number.Incremented when data is changed and wrapped around on overflow for values greater than 32.
//Current / next indicator	    1	Indicates if data is current in effect or is for future use.If the bit is flagged on, then the data is to be used at the present moment.
//Section number	            8	This is an index indicating which table this is in a related sequence of tables.The first table starts from 0.
//Last section number	        8	This indicates which table is the last table in the sequence of tables.
//Table data	                N * 8	Data as defined by the Table Identifier.
//CRC32	                        32	A checksum of the entire table excluding the pointer field, pointer filler bytes and the trailing CRC32.
// buf = Table_data
bool tsWriteTableSection(MpegTSSection *s, uint8 tid, uint32 id, uint32 version,
    uint32 sec_num, uint32 last_sec_num, uint8_t *buf, uint32 len)
{
    uint8_t section[1024], *q;
    unsigned int tot_len;
    /* reserved_future_use field must be set to 1 for SDT */
    unsigned int flags = tid == SDT_TID ? 0xf000 : 0xb000;

    tot_len = 3 + 5 + len + 4;
    /* check if not too big */
    if (tot_len > 1024) return false;

    q = section;
    *q++ = tid;
    put16(&q, flags | (len + 5 + 4));       /* 5 byte header + 4 byte CRC */
    put16(&q, id);                          // transport_stream_id
    *q++ = 0xc1 | (version << 1);           /* current_next_indicator = 1 */
    *q++ = sec_num;
    *q++ = last_sec_num;
    memcpy(q, buf, len);

    tsWriteSection(s, section, tot_len);
    return 0;
}

// PAT Table_data:
//Program num	    16	Relates to the Table ID extension in the associated PMT.A value of 0 is reserved for a NIT packet identifier.
//Reserved bits	    3	Set to 0x07 (all bits on)
//Program map PID	13	The packet identifier that contains the associated PMT
void tsWritePAT(uint16 * programId, uint16 * pmtPID, uint32 numOfProg)
{
    uint8_t data[SECTION_LENGTH], *q;
    int i;

    q = data;
    for (i = 0; i < numOfProg; i++)
    {
        put16(&q, programId[i]);
        put16(&q, 0xe000 | pmtPID[i]);
    }

    MpegTSSection pat;
    pat.pid = PAT_PID;
    tsWriteTableSection(&pat, PAT_TID, 1, 0, 0, 0, data, q - data);
}

#if 0
void tsWriteSDT()
{
    uint8_t data[SECTION_LENGTH], *q, *desc_list_len_ptr, *desc_len_ptr;
    int i, running_status, free_ca_mode, val;

    q = data;
    put16(&q, ts->onid);
    *q++ = 0xff;
        put16(&q, service->sid);
        *q++ = 0xfc | 0x00; /* currently no EIT info */
        desc_list_len_ptr = q;
        q += 2;
        running_status = 4; /* running */
        free_ca_mode = 0;

        /* write only one descriptor for the service name and provider */
        *q++ = 0x48;
        desc_len_ptr = q;
        q++;
        *q++ = ts->service_type;
        putstr8(&q, service->provider_name);
        putstr8(&q, service->name);
        desc_len_ptr[0] = q - desc_len_ptr - 1;

        /* fill descriptor length */
        val = (running_status << 13) | (free_ca_mode << 12) |
            (q - desc_list_len_ptr - 2);
        desc_list_len_ptr[0] = val >> 8;
        desc_list_len_ptr[1] = val;
    }
    mpegts_write_section1(&ts->sdt, SDT_TID, ts->tsid, ts->tables_version, 0, 0,
        data, q - data);
}
#endif

static int tsWritePMT(MpegTSProgramInfo * prog)
{
    uint8_t data[SECTION_LENGTH], *q, *desc_length_ptr, *program_info_length_ptr;
    int val, stream_type, i, err = 0;

    q = data;
    put16(&q, 0xe000 | prog->pcr_pid);
    program_info_length_ptr = q;
    q += 2; /* patched after */
    /* put program info here */
    val = 0xf000 | (q - program_info_length_ptr - 2);
    program_info_length_ptr[0] = val >> 8;
    program_info_length_ptr[1] = val;

    uint32 nb_streams = prog->avStream.size();
    for (i = 0; i < nb_streams; i++)
    {
        AVStream *st = prog->avStream[i];
        uint32 codec_id = st->GetCodecID();
        uint32 codecType = st->GetType();

        if (q - data > SECTION_LENGTH - 32)
        {
            err = 1;
            break;
        }
        switch (codec_id)
        {
        case AV_CODEC_ID_MPEG1VIDEO:
        case AV_CODEC_ID_MPEG2VIDEO:
            stream_type = STREAM_TYPE_VIDEO_MPEG2;
            break;
        case AV_CODEC_ID_MPEG4:
            stream_type = STREAM_TYPE_VIDEO_MPEG4;
            break;
        case AV_CODEC_ID_H264:
            stream_type = STREAM_TYPE_VIDEO_H264;
            break;
        case AV_CODEC_ID_HEVC:
            stream_type = STREAM_TYPE_VIDEO_HEVC;
            break;
        case AV_CODEC_ID_CAVS:
            stream_type = STREAM_TYPE_VIDEO_CAVS;
            break;
        case AV_CODEC_ID_DIRAC:
            stream_type = STREAM_TYPE_VIDEO_DIRAC;
            break;
        case AV_CODEC_ID_MP2:
        case AV_CODEC_ID_MP3:
            stream_type = STREAM_TYPE_AUDIO_MPEG1;
            break;
        case AV_CODEC_ID_AAC:
            stream_type = (prog->flags & MPEGTS_FLAG_AAC_LATM)
                ? STREAM_TYPE_AUDIO_AAC_LATM
                : STREAM_TYPE_AUDIO_AAC;
            break;
        case AV_CODEC_ID_AAC_LATM:
            stream_type = STREAM_TYPE_AUDIO_AAC_LATM;
            break;
        case AV_CODEC_ID_AC3:
            stream_type = STREAM_TYPE_AUDIO_AC3;
            break;
        case AV_CODEC_ID_DTS:
            stream_type = STREAM_TYPE_AUDIO_DTS;
            break;
        case AV_CODEC_ID_TRUEHD:
            stream_type = STREAM_TYPE_AUDIO_TRUEHD;
            break;
        default:
            stream_type = STREAM_TYPE_PRIVATE_DATA;
            break;
        }

        *q++ = stream_type;
        put16(&q, 0xe000 | (AVSTREAM_START_PID+1));
        desc_length_ptr = q;
        q += 2; /* patched after */

        /* write optional descriptors here */
        switch (codecType)
        {
        case VP_STREAM_AUDIO:
            if (codec_id == AV_CODEC_ID_EAC3) {
                *q++ = 0x7a; // EAC3 descriptor see A038 DVB SI
                *q++ = 1; // 1 byte, all flags sets to 0
                *q++ = 0; // omit all fields...
            }
            if (codec_id == AV_CODEC_ID_S302M) {
                *q++ = 0x05; /* MPEG-2 registration descriptor*/
                *q++ = 4;
                *q++ = 'B';
                *q++ = 'S';
                *q++ = 'S';
                *q++ = 'D';
            }
            break;
        case VP_STREAM_SUBTITLE:
        {
            const char default_language[] = "und";
            /// TODO:
            const char *language = default_language;
#if 0
            if (codec_id == AV_CODEC_ID_DVB_SUBTITLE)
            {
                uint8_t *len_ptr;
                int extradata_copied = 0;

                *q++ = 0x59; /* subtitling_descriptor */
                len_ptr = q++;

                while (strlen(language) >= 3) {
                    if (sizeof(data) - (q - data) < 8) { /* 8 bytes per DVB subtitle substream data */
                        err = 1;
                        break;
                    }
                    *q++ = *language++;
                    *q++ = *language++;
                    *q++ = *language++;
                    /* Skip comma */
                    if (*language != '\0')
                        language++;

                    if (st->codec->extradata_size - extradata_copied >= 5) {
                        *q++ = st->codec->extradata[extradata_copied + 4]; /* subtitling_type */
                        memcpy(q, st->codec->extradata + extradata_copied, 4); /* composition_page_id and ancillary_page_id */
                        extradata_copied += 5;
                        q += 4;
                    }
                    else {
                        /* subtitling_type:
                        * 0x10 - normal with no monitor aspect ratio criticality
                        * 0x20 - for the hard of hearing with no monitor aspect ratio criticality */
                        *q++ = (st->disposition & AV_DISPOSITION_HEARING_IMPAIRED) ? 0x20 : 0x10;
                        if ((st->codec->extradata_size == 4) && (extradata_copied == 0)) {
                            /* support of old 4-byte extradata format */
                            memcpy(q, st->codec->extradata, 4); /* composition_page_id and ancillary_page_id */
                            extradata_copied += 4;
                            q += 4;
                        }
                        else {
                            put16(&q, 1); /* composition_page_id */
                            put16(&q, 1); /* ancillary_page_id */
                        }
                    }
                }

                *len_ptr = q - len_ptr - 1;
            }
            else if (st->codec->codec_id == AV_CODEC_ID_DVB_TELETEXT) {
                uint8_t *len_ptr = NULL;
                int extradata_copied = 0;

                /* The descriptor tag. teletext_descriptor */
                *q++ = 0x56;
                len_ptr = q++;

                while (strlen(language) >= 3 && q - data < sizeof(data) - 6) {
                    *q++ = *language++;
                    *q++ = *language++;
                    *q++ = *language++;
                    /* Skip comma */
                    if (*language != '\0')
                        language++;

                    if (st->codec->extradata_size - 1 > extradata_copied) {
                        memcpy(q, st->codec->extradata + extradata_copied, 2);
                        extradata_copied += 2;
                        q += 2;
                    }
                    else {
                        /* The Teletext descriptor:
                        * teletext_type: This 5-bit field indicates the type of Teletext page indicated. (0x01 Initial Teletext page)
                        * teletext_magazine_number: This is a 3-bit field which identifies the magazine number.
                        * teletext_page_number: This is an 8-bit field giving two 4-bit hex digits identifying the page number. */
                        *q++ = 0x08;
                        *q++ = 0x00;
                    }
                }

                *len_ptr = q - len_ptr - 1;
            }
#endif
        }
            break;
        case VP_STREAM_VIDEO:
            if (stream_type == STREAM_TYPE_VIDEO_DIRAC) {
                *q++ = 0x05; /*MPEG-2 registration descriptor*/
                *q++ = 4;
                *q++ = 'd';
                *q++ = 'r';
                *q++ = 'a';
                *q++ = 'c';
            }
            break;
        case VP_STREAM_DATA:
            if (codec_id == AV_CODEC_ID_SMPTE_KLV) {
                *q++ = 0x05; /* MPEG-2 registration descriptor */
                *q++ = 4;
                *q++ = 'K';
                *q++ = 'L';
                *q++ = 'V';
                *q++ = 'A';
            }
            break;
        }

        val = 0xf000 | (q - desc_length_ptr - 2);
        desc_length_ptr[0] = val >> 8;
        desc_length_ptr[1] = val;
    }

    if (err)
    {
        osLog(LOG_ERROR,
            "The PMT section cannot fit stream %d and all following streams.\n"
            "Try reducing the number of languages in the audio streams "
            "or the total number of streams.\n", i);
    }

    MpegTSSection pmt;
    pmt.pid = PMT_PID;
    tsWriteTableSection(&pmt, PMT_TID, 1, 0, 0, 0, data, q - data);
    return 0;
}


