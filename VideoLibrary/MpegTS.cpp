#include "MpegTS.h"
#include "os.h"

/* The section length is 12 bits. The first 2 are set to 0, the remaining
* 10 bits should not exceed 1021. */
#define SECTION_LENGTH 1020
#define AV_TIME_BASE            1000000
#define PCR_TIME_BASE 27000000



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

        s->write_packet(packet);

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

/* NOTE: !str is accepted for an empty string */
static void putstr8(uint8_t **q_ptr, const char *str)
{
    uint8_t *q;
    int len;

    q = *q_ptr;
    if (!str)
        len = 0;
    else
        len = strlen(str);
    *q++ = len;
    memcpy(q, str, len);
    q += len;
    *q_ptr = q;
}


void tsWriteSDT(uint16 program_number)
{
    uint8_t data[SECTION_LENGTH], *q, *desc_list_len_ptr, *desc_len_ptr;
    int i, running_status, free_ca_mode, val;

    q = data;
    // original_netword_id
    put16(&q, 1);
    // reserved 8bits
    *q++ = 0xff;

    ///////////////////////////////////////////////////
    // one service
    // service_id:16 bits, PMT:program_number
    put16(&q, program_number);  // 0x01
    *q++ = 0xfc | 0x00; /* currently no EIT info */
    desc_list_len_ptr = q;
    q += 2;
    running_status = 4; /* running */
    free_ca_mode = 0;

    /* write only one descriptor for the service name and provider */
    *q++ = 0x48;
    desc_len_ptr = q;
    q++;
    *q++ = 1;   //service_type
    putstr8(&q, "Alex-VideoLibrary");
    putstr8(&q, "AlexVideoService");
    desc_len_ptr[0] = q - desc_len_ptr - 1;

    /* fill descriptor length */
    val = (running_status << 13) | (free_ca_mode << 12) |
        (q - desc_list_len_ptr - 2);
    desc_list_len_ptr[0] = val >> 8;
    desc_list_len_ptr[1] = val;

    MpegTSSection sdt;
    sdt.pid = PAT_PID;
    tsWriteTableSection(&sdt, SDT_TID, 1, 0, 0, 0, data, q - data);
}


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

enum AVRounding {
    AV_ROUND_ZERO = 0, ///< Round toward zero.
    AV_ROUND_INF = 1, ///< Round away from zero.
    AV_ROUND_DOWN = 2, ///< Round toward -infinity.
    AV_ROUND_UP = 3, ///< Round toward +infinity.
    AV_ROUND_NEAR_INF = 5, ///< Round to nearest and halfway cases away from zero.
    AV_ROUND_PASS_MINMAX = 8192, ///< Flag to pass INT64_MIN/MAX through instead of rescaling, this avoids special cases for AV_NOPTS_VALUE
};
/**
* Rescale a 64-bit integer with rounding to nearest.
* A simple a*b/c isn't possible as it can overflow.
*/
int64_t av_rescale_rnd(int64_t a, int64_t b, int64_t c, enum AVRounding rnd)
{
    int64_t r = 0;
    osAssert(c > 0);
    osAssert(b >= 0);
    osAssert((unsigned)(rnd&~AV_ROUND_PASS_MINMAX) <= 5 && (rnd&~AV_ROUND_PASS_MINMAX) != 4);

    if (c <= 0 || b < 0 || !((unsigned)(rnd&~AV_ROUND_PASS_MINMAX) <= 5 && (rnd&~AV_ROUND_PASS_MINMAX) != 4))
        return INT64_MIN;

    if (rnd & AV_ROUND_PASS_MINMAX) {
        if (a == INT64_MIN || a == INT64_MAX)
            return a;
        rnd = (AVRounding)((uint32)rnd - (uint32)AV_ROUND_PASS_MINMAX);
    }

    if (a < 0 && a != INT64_MIN)
        return -av_rescale_rnd(-a, b, c, (AVRounding)((uint32)rnd ^ ((rnd >> 1) & 1)));

    if (rnd == AV_ROUND_NEAR_INF)
        r = c / 2;
    else if (rnd & 1)
        r = c - 1;

    if (b <= INT_MAX && c <= INT_MAX) {
        if (a <= INT_MAX)
            return (a * b + r) / c;
        else
            return a / c * b + (a % c * b + r) / c;
    }
    else {
#if 1
        uint64_t a0 = a & 0xFFFFFFFF;
        uint64_t a1 = a >> 32;
        uint64_t b0 = b & 0xFFFFFFFF;
        uint64_t b1 = b >> 32;
        uint64_t t1 = a0 * b1 + a1 * b0;
        uint64_t t1a = t1 << 32;
        int i;

        a0 = a0 * b0 + t1a;
        a1 = a1 * b1 + (t1 >> 32) + (a0 < t1a);
        a0 += r;
        a1 += a0 < r;

        for (i = 63; i >= 0; i--) {
            a1 += a1 + ((a0 >> i) & 1);
            t1 += t1;
            if (c <= a1) {
                a1 -= c;
                t1++;
            }
        }
        return t1;
    }
#else
        AVInteger ai;
        ai = av_mul_i(av_int2i(a), av_int2i(b));
        ai = av_add_i(ai, av_int2i(r));
        return av_i2int(av_div_i(ai, av_int2i(c)));
}
#endif
}
/**
* Rescale a 64-bit integer with rounding to nearest.
* A simple a*b/c isn't possible as it can overflow.
*/
int64_t av_rescale(int64_t a, int64_t b, int64_t c)
{
    return av_rescale_rnd(a, b, c, AV_ROUND_NEAR_INF);
}

/* send SDT, PAT and PMT tables regulary */
static void retransmit_si_info(MpegTSProgramInfo * prog, int force_pat)
{
    if (++prog->sdt_packet_count == prog->sdt_packet_period) {
        prog->sdt_packet_count = 0;
        tsWriteSDT(prog->progNum);
    }
    if (++prog->pat_packet_count == prog->pat_packet_period || force_pat) {
        prog->pat_packet_count = 0;
        tsWritePAT(&prog->progNum, &prog->pmtPID, 1);
        tsWritePMT(prog);
    }
}

static int64_t get_pcr(MpegTSProgramInfo * prog)
{
    return av_rescale(prog->tsFile->GetPosition() + 11, 8 * PCR_TIME_BASE, prog->mux_rate);
}

static int write_pcr_bits(uint8_t *buf, int64_t pcr)
{
    int64_t pcr_low = pcr % 300, pcr_high = pcr / 300;

    *buf++ = pcr_high >> 25;
    *buf++ = pcr_high >> 17;
    *buf++ = pcr_high >> 9;
    *buf++ = pcr_high >> 1;
    *buf++ = pcr_high << 7 | pcr_low >> 8 | 0x7e;
    *buf++ = pcr_low;

    return 6;
}

/* Write a single null transport stream packet */
static void mpegts_insert_null_packet(MpegTSPacket * pkt)
{
    uint8_t *q;
    uint8_t buf[TSPKT_LENGTH];

    q = buf;
    *q++ = 0x47;
    *q++ = 0x00 | 0x1f;
    *q++ = 0xff;
    *q++ = 0x10;
    memset(q, 0x0FF, TSPKT_LENGTH - (q - buf));
    pkt->write(buf, TSPKT_LENGTH);
}

/* Write a single transport stream packet with a PCR and no payload */
static void mpegts_insert_pcr_only(MpegTSProgramInfo * prog, MpegTSPacket * pkt)
{
    uint8_t *q;
    uint8_t buf[TSPKT_LENGTH];

    q = buf;
    *q++ = 0x47;
    *q++ = pkt->pid >> 8;
    *q++ = pkt->pid;
    *q++ = 0x20 | pkt->cc;   /* Adaptation only */
    /* Continuity Count field does not increment (see 13818-1 section 2.4.3.3) */
    *q++ = TSPKT_LENGTH - 5; /* Adaptation Field Length */
    *q++ = 0x10;               /* Adaptation flags: PCR present */

    /* PCR coded into 6 bytes */
    q += write_pcr_bits(q, get_pcr(prog));

    /* stuffing bytes */
    memset(q, 0xFF, TSPKT_LENGTH - (q - buf));
    pkt->write(buf, TSPKT_LENGTH);
}

static void write_pts(uint8_t *q, int fourbits, int64_t pts)
{
    int val;

    val = fourbits << 4 | (((pts >> 30) & 0x07) << 1) | 1;
    *q++ = val;
    val = (((pts >> 15) & 0x7fff) << 1) | 1;
    *q++ = val >> 8;
    *q++ = val;
    val = (((pts)& 0x7fff) << 1) | 1;
    *q++ = val >> 8;
    *q++ = val;
}

/* Set an adaptation field flag in an MPEG-TS packet*/
static void set_af_flag(uint8_t *pkt, int flag)
{
    // expect at least one flag to set
    osAssert(flag);

    if ((pkt[3] & 0x20) == 0) {
        // no AF yet, set adaptation field flag
        pkt[3] |= 0x20;
        // 1 byte length, no flags
        pkt[4] = 1;
        pkt[5] = 0;
    }
    pkt[5] |= flag;
}

/* Extend the adaptation field by size bytes */
static void extend_af(uint8_t *pkt, int size)
{
    // expect already existing adaptation field
    osAssert(pkt[3] & 0x20);
    pkt[4] += size;
}

/* Get a pointer to MPEG-TS payload (right after TS packet header) */
static uint8_t *get_ts_payload_start(uint8_t *pkt)
{
    if (pkt[3] & 0x20)
        return pkt + 5 + pkt[4];
    else
        return pkt + 4;
}

/* Add a PES header to the front of the payload, and segment into an integer
* number of TS packets. The final TS packet is padded using an oversized
* adaptation header to exactly fill the last TS packet.
* NOTE: 'payload' contains a complete PES payload. */
static void tsWritePES(MpegTSProgramInfo * prog, AVStream * av, MpegTSPacket * pkt)
{
    uint16 pid = pkt->pid;
    const uint8_t *payload = pkt->payload;
    int payload_size = pkt->payload_size;
    int64_t pts = pkt->pts;
    int64_t dts = pkt->dts;
    int keyFrame = pkt->keyFrame;
    
    uint8_t buf[TSPKT_LENGTH];
    uint8_t *q;
    int val, is_start, len, header_len, write_pcr, is_dvb_subtitle, is_dvb_teletext, flags;
    int afc_len, stuffing_len;
    int64_t pcr = -1; /* avoid warning */
    int64_t delay = av_rescale(prog->max_delay, 90000, AV_TIME_BASE);
    int force_pat = av->GetType() == VP_STREAM_VIDEO && keyFrame && !av->prev_payload_key;

    is_start = 1;
    while (payload_size > 0) {
        retransmit_si_info(prog, force_pat);
        force_pat = 0;

        write_pcr = 0;
        if (pid == prog->pcr_pid) {
            if (prog->mux_rate > 1 || is_start) // VBR pcr period is based on frames
                prog->pcr_packet_count++;
            if (prog->pcr_packet_count >=
                prog->pcr_packet_period) {
                prog->pcr_packet_count = 0;
                write_pcr = 1;
            }
        }

        if (prog->mux_rate > 1 && dts != AV_NOPTS_VALUE &&
            (dts - get_pcr(prog) / 300) > delay) {
            /* pcr insert gets priority over null packet insert */
            if (write_pcr)
                mpegts_insert_pcr_only(prog, pkt);
            else
                mpegts_insert_null_packet(pkt);
            /* recalculate write_pcr and possibly retransmit si_info */
            continue;
        }

        /* prepare packet header */
        q = buf;
        *q++ = 0x47;
        val = pkt->pid >> 8;
        if (is_start)
            val |= 0x40;
        *q++ = val;
        *q++ = pkt->pid;
        pkt->cc = pkt->cc + 1 & 0xf;
        *q++ = 0x10 | pkt->cc; // payload indicator + CC
        if (keyFrame && is_start && pts != AV_NOPTS_VALUE) {
            // set Random Access for key frames
            if (pid == prog->pcr_pid)
                write_pcr = 1;
            set_af_flag(buf, 0x40);
            q = get_ts_payload_start(buf);
        }
        if (write_pcr) {
            set_af_flag(buf, 0x10);
            q = get_ts_payload_start(buf);
            // add 11, pcr references the last byte of program clock reference base
            if (prog->mux_rate > 1)
                pcr = get_pcr(prog);
            else
                pcr = (dts - delay) * 300;
            if (dts != AV_NOPTS_VALUE && dts < pcr / 300)
                osLog(LOG_WARN, "dts < pcr, TS is invalid\n");
            extend_af(buf, write_pcr_bits(q, pcr));
            q = get_ts_payload_start(buf);
        }
        if (is_start) {
            int pes_extension = 0;
            int pes_header_stuffing_bytes = 0;
            /* write PES header */
            *q++ = 0x00;
            *q++ = 0x00;
            *q++ = 0x01;
            is_dvb_subtitle = 0;
            is_dvb_teletext = 0;
            if (av->GetType() == VP_STREAM_VIDEO) {
                if (av->GetCodecID() == AV_CODEC_ID_DIRAC)
                    *q++ = 0xfd;
                else
                    *q++ = 0xe0;
            }
            else if (av->GetType() == VP_STREAM_AUDIO &&
                (av->GetCodecID() == AV_CODEC_ID_MP2 ||
                av->GetCodecID() == AV_CODEC_ID_MP3 ||
                av->GetCodecID() == AV_CODEC_ID_AAC)) {
                *q++ = 0xc0;
            }
            else {
                *q++ = 0xbd;
                if (av->GetType() == VP_STREAM_SUBTITLE) {
                    if (av->GetCodecID() == AV_CODEC_ID_DVB_SUBTITLE) {
                        is_dvb_subtitle = 1;
                    }
                    else if (av->GetCodecID() == AV_CODEC_ID_DVB_TELETEXT) {
                        is_dvb_teletext = 1;
                    }
                }
            }
            header_len = 0;
            flags = 0;
            if (pts != AV_NOPTS_VALUE) {
                header_len += 5;
                flags |= 0x80;
            }
            if (dts != AV_NOPTS_VALUE && pts != AV_NOPTS_VALUE && dts != pts) {
                header_len += 5;
                flags |= 0x40;
            }
            if (av->GetType() == VP_STREAM_VIDEO &&
                av->GetCodecID() == AV_CODEC_ID_DIRAC) {
                /* set PES_extension_flag */
                pes_extension = 1;
                flags |= 0x01;

                /* One byte for PES2 extension flag +
                * one byte for extension length +
                * one byte for extension id */
                header_len += 3;
            }
            
            len = payload_size + header_len + 3;
            /* 3 extra bytes should be added to DVB subtitle payload: 0x20 0x00 at the beginning and trailing 0xff */
            if (is_dvb_subtitle) {
                len += 3;
                payload_size++;
            }
            if (len > 0xffff)
                len = 0;
            //if (ts->omit_video_pes_length && st->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            //    len = 0;
            //}
            *q++ = len >> 8;
            *q++ = len;
            val = 0x80;
            /* data alignment indicator is required for subtitle and data streams */
            if (av->GetType() == VP_STREAM_SUBTITLE || av->GetType() == VP_STREAM_DATA)
                val |= 0x04;
            *q++ = val;
            *q++ = flags;
            *q++ = header_len;
            if (pts != AV_NOPTS_VALUE) {
                write_pts(q, flags >> 6, pts);
                q += 5;
            }
            if (dts != AV_NOPTS_VALUE && pts != AV_NOPTS_VALUE && dts != pts) {
                write_pts(q, 1, dts);
                q += 5;
            }
            if (pes_extension && av->GetCodecID() == AV_CODEC_ID_DIRAC) {
                flags = 0x01;  /* set PES_extension_flag_2 */
                *q++ = flags;
                *q++ = 0x80 | 0x01; /* marker bit + extension length */
                /* Set the stream ID extension flag bit to 0 and
                * write the extended stream ID. */
                *q++ = 0x00 | 0x60;
            }
            /* For Blu-ray AC3 Audio Setting extended flags */
            //if (ts->m2ts_mode &&
            //    pes_extension &&
            //    st->codec->codec_id == AV_CODEC_ID_AC3) {
            //    flags = 0x01; /* set PES_extension_flag_2 */
            //    *q++ = flags;
            //    *q++ = 0x80 | 0x01; /* marker bit + extension length */
            //    *q++ = 0x00 | 0x71; /* for AC3 Audio (specifically on blue-rays) */
            //}

            is_start = 0;
        }
        /* header size */
        header_len = q - buf;
        /* data len */
        len = TSPKT_LENGTH - header_len;
        if (len > payload_size)
            len = payload_size;
        stuffing_len = TSPKT_LENGTH - header_len - len;
        if (stuffing_len > 0) {
            /* add stuffing with AFC */
            if (buf[3] & 0x20) {
                /* stuffing already present: increase its size */
                afc_len = buf[4] + 1;
                memmove(buf + 4 + afc_len + stuffing_len,
                    buf + 4 + afc_len,
                    header_len - (4 + afc_len));
                buf[4] += stuffing_len;
                memset(buf + 4 + afc_len, 0xff, stuffing_len);
            }
            else {
                /* add stuffing */
                memmove(buf + 4 + stuffing_len, buf + 4, header_len - 4);
                buf[3] |= 0x20;
                buf[4] = stuffing_len - 1;
                if (stuffing_len >= 2) {
                    buf[5] = 0x00;
                    memset(buf + 6, 0xff, stuffing_len - 2);
                }
            }
        }

        if (is_dvb_subtitle && payload_size == len) {
            memcpy(buf + TSPKT_LENGTH - len, payload, len - 1);
            buf[TSPKT_LENGTH - 1] = 0xff; /* end_of_PES_data_field_marker: an 8-bit field with fixed contents 0xff for DVB subtitle */
        }
        else {
            memcpy(buf + TSPKT_LENGTH - len, payload, len);
        }

        payload += len;
        payload_size -= len;
        pkt->write(buf, TSPKT_LENGTH);
    }
    av->prev_payload_key = keyFrame;
}

bool MpegTSClass::writePacket(uint32 index, uint8 * buf, uint32 size, bool isSync,
    int64 pts = AV_NOPTS_VALUE, int64 dts = AV_NOPTS_VALUE)
{
    m_pkt.init(getStreamPID(index), size, pts, dts, isSync);
    m_pkt.alloc(size);

    try {
        tsWritePES(&m_prog, m_prog.avStream[index], &m_pkt);
    }
    catch (osException * except)
    {
        osLog(LOG_WARN, "Execptions: %s", except->msg().c_str());
        return false;
    }

    return true;
}


