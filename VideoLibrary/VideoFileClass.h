#ifndef _VIDEO_FILE_CLASS_H
#define _VIDEO_FILE_CLASS_H
#include "osFile.h"
#include "os.h"
#include "vptypes.h"
#include "vpUtil.h"
#include "osException.h"

#define WORD_SIZE sizeof(long)
#if WORDS_BIGENDIAN
#define endian_fix(x) (x)
#define endian_fix64(x) (x)
#define endian_fix32(x) (x)
#define endian_fix16(x) (x)
#else
static INLINE uint32_t endian_fix32(uint32_t x)
{
    return (x << 24) + ((x << 8) & 0xff0000) + ((x >> 8) & 0xff00) + (x >> 24);
}
static INLINE uint64_t endian_fix64(uint64_t x)
{
    return endian_fix32(x >> 32) + ((uint64_t)endian_fix32(x) << 32);
}
static INLINE intptr_t endian_fix(intptr_t x)
{
    return WORD_SIZE == 8 ? endian_fix64(x) : endian_fix32(x);
}
static INLINE uint16_t endian_fix16(uint16_t x)
{
    return (x << 8) | (x >> 8);
}
#endif

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
    VP_STREAM_VIDEO,
    VP_STREAM_AUDIO,
    VP_STREAM_HINT,			//¡®hint¡¯ Hint track
    VP_STREAM_META,			//¡®meta¡¯ Timed Metadata track
    VP_STREAM_AUXV,			//¡®auxv¡¯ Auxiliary Video track
    VP_STREAM_SUBTITLE,
    VP_STREAM_DATA
};

enum AVCodecID {
    AV_CODEC_ID_NONE,

    /* video codecs */
    AV_CODEC_ID_MPEG1VIDEO,
    AV_CODEC_ID_MPEG2VIDEO, ///< preferred ID for MPEG-1/2 video decoding
    AV_CODEC_ID_H261,
    AV_CODEC_ID_H263,
    AV_CODEC_ID_RV10,
    AV_CODEC_ID_RV20,
    AV_CODEC_ID_MJPEG,
    AV_CODEC_ID_MJPEGB,
    AV_CODEC_ID_LJPEG,
    AV_CODEC_ID_SP5X,
    AV_CODEC_ID_JPEGLS,
    AV_CODEC_ID_MPEG4,
    AV_CODEC_ID_RAWVIDEO,
    AV_CODEC_ID_MSMPEG4V1,
    AV_CODEC_ID_MSMPEG4V2,
    AV_CODEC_ID_MSMPEG4V3,
    AV_CODEC_ID_WMV1,
    AV_CODEC_ID_WMV2,
    AV_CODEC_ID_H263P,
    AV_CODEC_ID_H263I,
    AV_CODEC_ID_FLV1,
    AV_CODEC_ID_SVQ1,
    AV_CODEC_ID_SVQ3,
    AV_CODEC_ID_DVVIDEO,
    AV_CODEC_ID_HUFFYUV,
    AV_CODEC_ID_CYUV,
    AV_CODEC_ID_H264,
    AV_CODEC_ID_INDEO3,
    AV_CODEC_ID_VP3,
    AV_CODEC_ID_THEORA,
    AV_CODEC_ID_ASV1,
    AV_CODEC_ID_ASV2,
    AV_CODEC_ID_FFV1,
    AV_CODEC_ID_4XM,
    AV_CODEC_ID_VCR1,
    AV_CODEC_ID_CLJR,
    AV_CODEC_ID_MDEC,
    AV_CODEC_ID_ROQ,
    AV_CODEC_ID_INTERPLAY_VIDEO,
    AV_CODEC_ID_XAN_WC3,
    AV_CODEC_ID_XAN_WC4,
    AV_CODEC_ID_RPZA,
    AV_CODEC_ID_CINEPAK,
    AV_CODEC_ID_WS_VQA,
    AV_CODEC_ID_MSRLE,
    AV_CODEC_ID_MSVIDEO1,
    AV_CODEC_ID_IDCIN,
    AV_CODEC_ID_8BPS,
    AV_CODEC_ID_SMC,
    AV_CODEC_ID_FLIC,
    AV_CODEC_ID_TRUEMOTION1,
    AV_CODEC_ID_VMDVIDEO,
    AV_CODEC_ID_MSZH,
    AV_CODEC_ID_ZLIB,
    AV_CODEC_ID_QTRLE,
    AV_CODEC_ID_TSCC,
    AV_CODEC_ID_ULTI,
    AV_CODEC_ID_QDRAW,
    AV_CODEC_ID_VIXL,
    AV_CODEC_ID_QPEG,
    AV_CODEC_ID_PNG,
    AV_CODEC_ID_PPM,
    AV_CODEC_ID_PBM,
    AV_CODEC_ID_PGM,
    AV_CODEC_ID_PGMYUV,
    AV_CODEC_ID_PAM,
    AV_CODEC_ID_FFVHUFF,
    AV_CODEC_ID_RV30,
    AV_CODEC_ID_RV40,
    AV_CODEC_ID_VC1,
    AV_CODEC_ID_WMV3,
    AV_CODEC_ID_LOCO,
    AV_CODEC_ID_WNV1,
    AV_CODEC_ID_AASC,
    AV_CODEC_ID_INDEO2,
    AV_CODEC_ID_FRAPS,
    AV_CODEC_ID_TRUEMOTION2,
    AV_CODEC_ID_BMP,
    AV_CODEC_ID_CSCD,
    AV_CODEC_ID_MMVIDEO,
    AV_CODEC_ID_ZMBV,
    AV_CODEC_ID_AVS,
    AV_CODEC_ID_SMACKVIDEO,
    AV_CODEC_ID_NUV,
    AV_CODEC_ID_KMVC,
    AV_CODEC_ID_FLASHSV,
    AV_CODEC_ID_CAVS,
    AV_CODEC_ID_JPEG2000,
    AV_CODEC_ID_VMNC,
    AV_CODEC_ID_VP5,
    AV_CODEC_ID_VP6,
    AV_CODEC_ID_VP6F,
    AV_CODEC_ID_TARGA,
    AV_CODEC_ID_DSICINVIDEO,
    AV_CODEC_ID_TIERTEXSEQVIDEO,
    AV_CODEC_ID_TIFF,
    AV_CODEC_ID_GIF,
    AV_CODEC_ID_DXA,
    AV_CODEC_ID_DNXHD,
    AV_CODEC_ID_THP,
    AV_CODEC_ID_SGI,
    AV_CODEC_ID_C93,
    AV_CODEC_ID_BETHSOFTVID,
    AV_CODEC_ID_PTX,
    AV_CODEC_ID_TXD,
    AV_CODEC_ID_VP6A,
    AV_CODEC_ID_AMV,
    AV_CODEC_ID_VB,
    AV_CODEC_ID_PCX,
    AV_CODEC_ID_SUNRAST,
    AV_CODEC_ID_INDEO4,
    AV_CODEC_ID_INDEO5,
    AV_CODEC_ID_MIMIC,
    AV_CODEC_ID_RL2,
    AV_CODEC_ID_ESCAPE124,
    AV_CODEC_ID_DIRAC,
    AV_CODEC_ID_BFI,
    AV_CODEC_ID_CMV,
    AV_CODEC_ID_MOTIONPIXELS,
    AV_CODEC_ID_TGV,
    AV_CODEC_ID_TGQ,
    AV_CODEC_ID_TQI,
    AV_CODEC_ID_AURA,
    AV_CODEC_ID_AURA2,
    AV_CODEC_ID_V210X,
    AV_CODEC_ID_TMV,
    AV_CODEC_ID_V210,
    AV_CODEC_ID_DPX,
    AV_CODEC_ID_MAD,
    AV_CODEC_ID_FRWU,
    AV_CODEC_ID_FLASHSV2,
    AV_CODEC_ID_CDGRAPHICS,
    AV_CODEC_ID_R210,
    AV_CODEC_ID_ANM,
    AV_CODEC_ID_BINKVIDEO,
    AV_CODEC_ID_IFF_ILBM,
    AV_CODEC_ID_IFF_BYTERUN1,
    AV_CODEC_ID_KGV1,
    AV_CODEC_ID_YOP,
    AV_CODEC_ID_VP8,
    AV_CODEC_ID_PICTOR,
    AV_CODEC_ID_ANSI,
    AV_CODEC_ID_A64_MULTI,
    AV_CODEC_ID_A64_MULTI5,
    AV_CODEC_ID_R10K,
    AV_CODEC_ID_MXPEG,
    AV_CODEC_ID_LAGARITH,
    AV_CODEC_ID_PRORES,
    AV_CODEC_ID_JV,
    AV_CODEC_ID_DFA,
    AV_CODEC_ID_WMV3IMAGE,
    AV_CODEC_ID_VC1IMAGE,
    AV_CODEC_ID_UTVIDEO,
    AV_CODEC_ID_BMV_VIDEO,
    AV_CODEC_ID_VBLE,
    AV_CODEC_ID_DXTORY,
    AV_CODEC_ID_V410,
    AV_CODEC_ID_XWD,
    AV_CODEC_ID_CDXL,
    AV_CODEC_ID_XBM,
    AV_CODEC_ID_ZEROCODEC,
    AV_CODEC_ID_MSS1,
    AV_CODEC_ID_MSA1,
    AV_CODEC_ID_TSCC2,
    AV_CODEC_ID_MTS2,
    AV_CODEC_ID_CLLC,
    AV_CODEC_ID_MSS2,
    AV_CODEC_ID_VP9,
    AV_CODEC_ID_AIC,
    AV_CODEC_ID_ESCAPE130_DEPRECATED,
    AV_CODEC_ID_G2M_DEPRECATED,
    AV_CODEC_ID_WEBP_DEPRECATED,
    AV_CODEC_ID_HNM4_VIDEO,
    AV_CODEC_ID_HEVC_DEPRECATED,
    AV_CODEC_ID_FIC,
    AV_CODEC_ID_ALIAS_PIX,
    AV_CODEC_ID_BRENDER_PIX_DEPRECATED,
    AV_CODEC_ID_PAF_VIDEO_DEPRECATED,
    AV_CODEC_ID_EXR_DEPRECATED,
    AV_CODEC_ID_VP7_DEPRECATED,
    AV_CODEC_ID_SANM_DEPRECATED,
    AV_CODEC_ID_SGIRLE_DEPRECATED,
    AV_CODEC_ID_MVC1_DEPRECATED,
    AV_CODEC_ID_MVC2_DEPRECATED,
    AV_CODEC_ID_HQX,
    AV_CODEC_ID_TDSC,
    AV_CODEC_ID_HQ_HQA,
    AV_CODEC_ID_HAP,
    AV_CODEC_ID_DDS,

    AV_CODEC_ID_BRENDER_PIX = MKBETAG('B', 'P', 'I', 'X'),
    AV_CODEC_ID_Y41P = MKBETAG('Y', '4', '1', 'P'),
    AV_CODEC_ID_ESCAPE130 = MKBETAG('E', '1', '3', '0'),
    AV_CODEC_ID_EXR = MKBETAG('0', 'E', 'X', 'R'),
    AV_CODEC_ID_AVRP = MKBETAG('A', 'V', 'R', 'P'),

    AV_CODEC_ID_012V = MKBETAG('0', '1', '2', 'V'),
    AV_CODEC_ID_G2M = MKBETAG(0, 'G', '2', 'M'),
    AV_CODEC_ID_AVUI = MKBETAG('A', 'V', 'U', 'I'),
    AV_CODEC_ID_AYUV = MKBETAG('A', 'Y', 'U', 'V'),
    AV_CODEC_ID_TARGA_Y216 = MKBETAG('T', '2', '1', '6'),
    AV_CODEC_ID_V308 = MKBETAG('V', '3', '0', '8'),
    AV_CODEC_ID_V408 = MKBETAG('V', '4', '0', '8'),
    AV_CODEC_ID_YUV4 = MKBETAG('Y', 'U', 'V', '4'),
    AV_CODEC_ID_SANM = MKBETAG('S', 'A', 'N', 'M'),
    AV_CODEC_ID_PAF_VIDEO = MKBETAG('P', 'A', 'F', 'V'),
    AV_CODEC_ID_AVRN = MKBETAG('A', 'V', 'R', 'n'),
    AV_CODEC_ID_CPIA = MKBETAG('C', 'P', 'I', 'A'),
    AV_CODEC_ID_XFACE = MKBETAG('X', 'F', 'A', 'C'),
    AV_CODEC_ID_SGIRLE = MKBETAG('S', 'G', 'I', 'R'),
    AV_CODEC_ID_MVC1 = MKBETAG('M', 'V', 'C', '1'),
    AV_CODEC_ID_MVC2 = MKBETAG('M', 'V', 'C', '2'),
    AV_CODEC_ID_SNOW = MKBETAG('S', 'N', 'O', 'W'),
    AV_CODEC_ID_WEBP = MKBETAG('W', 'E', 'B', 'P'),
    AV_CODEC_ID_SMVJPEG = MKBETAG('S', 'M', 'V', 'J'),
    AV_CODEC_ID_HEVC = MKBETAG('H', '2', '6', '5'),
#define AV_CODEC_ID_H265 AV_CODEC_ID_HEVC
    AV_CODEC_ID_VP7 = MKBETAG('V', 'P', '7', '0'),
    AV_CODEC_ID_APNG = MKBETAG('A', 'P', 'N', 'G'),

    /* various PCM "codecs" */
    AV_CODEC_ID_FIRST_AUDIO = 0x10000,     ///< A dummy id pointing at the start of audio codecs
    AV_CODEC_ID_PCM_S16LE = 0x10000,
    AV_CODEC_ID_PCM_S16BE,
    AV_CODEC_ID_PCM_U16LE,
    AV_CODEC_ID_PCM_U16BE,
    AV_CODEC_ID_PCM_S8,
    AV_CODEC_ID_PCM_U8,
    AV_CODEC_ID_PCM_MULAW,
    AV_CODEC_ID_PCM_ALAW,
    AV_CODEC_ID_PCM_S32LE,
    AV_CODEC_ID_PCM_S32BE,
    AV_CODEC_ID_PCM_U32LE,
    AV_CODEC_ID_PCM_U32BE,
    AV_CODEC_ID_PCM_S24LE,
    AV_CODEC_ID_PCM_S24BE,
    AV_CODEC_ID_PCM_U24LE,
    AV_CODEC_ID_PCM_U24BE,
    AV_CODEC_ID_PCM_S24DAUD,
    AV_CODEC_ID_PCM_ZORK,
    AV_CODEC_ID_PCM_S16LE_PLANAR,
    AV_CODEC_ID_PCM_DVD,
    AV_CODEC_ID_PCM_F32BE,
    AV_CODEC_ID_PCM_F32LE,
    AV_CODEC_ID_PCM_F64BE,
    AV_CODEC_ID_PCM_F64LE,
    AV_CODEC_ID_PCM_BLURAY,
    AV_CODEC_ID_PCM_LXF,
    AV_CODEC_ID_S302M,
    AV_CODEC_ID_PCM_S8_PLANAR,
    AV_CODEC_ID_PCM_S24LE_PLANAR_DEPRECATED,
    AV_CODEC_ID_PCM_S32LE_PLANAR_DEPRECATED,
    AV_CODEC_ID_PCM_S16BE_PLANAR_DEPRECATED,
    AV_CODEC_ID_PCM_S24LE_PLANAR = MKBETAG(24, 'P', 'S', 'P'),
    AV_CODEC_ID_PCM_S32LE_PLANAR = MKBETAG(32, 'P', 'S', 'P'),
    AV_CODEC_ID_PCM_S16BE_PLANAR = MKBETAG('P', 'S', 'P', 16),

    /* various ADPCM codecs */
    AV_CODEC_ID_ADPCM_IMA_QT = 0x11000,
    AV_CODEC_ID_ADPCM_IMA_WAV,
    AV_CODEC_ID_ADPCM_IMA_DK3,
    AV_CODEC_ID_ADPCM_IMA_DK4,
    AV_CODEC_ID_ADPCM_IMA_WS,
    AV_CODEC_ID_ADPCM_IMA_SMJPEG,
    AV_CODEC_ID_ADPCM_MS,
    AV_CODEC_ID_ADPCM_4XM,
    AV_CODEC_ID_ADPCM_XA,
    AV_CODEC_ID_ADPCM_ADX,
    AV_CODEC_ID_ADPCM_EA,
    AV_CODEC_ID_ADPCM_G726,
    AV_CODEC_ID_ADPCM_CT,
    AV_CODEC_ID_ADPCM_SWF,
    AV_CODEC_ID_ADPCM_YAMAHA,
    AV_CODEC_ID_ADPCM_SBPRO_4,
    AV_CODEC_ID_ADPCM_SBPRO_3,
    AV_CODEC_ID_ADPCM_SBPRO_2,
    AV_CODEC_ID_ADPCM_THP,
    AV_CODEC_ID_ADPCM_IMA_AMV,
    AV_CODEC_ID_ADPCM_EA_R1,
    AV_CODEC_ID_ADPCM_EA_R3,
    AV_CODEC_ID_ADPCM_EA_R2,
    AV_CODEC_ID_ADPCM_IMA_EA_SEAD,
    AV_CODEC_ID_ADPCM_IMA_EA_EACS,
    AV_CODEC_ID_ADPCM_EA_XAS,
    AV_CODEC_ID_ADPCM_EA_MAXIS_XA,
    AV_CODEC_ID_ADPCM_IMA_ISS,
    AV_CODEC_ID_ADPCM_G722,
    AV_CODEC_ID_ADPCM_IMA_APC,
    AV_CODEC_ID_ADPCM_VIMA_DEPRECATED,
    AV_CODEC_ID_ADPCM_VIMA = MKBETAG('V', 'I', 'M', 'A'),
#if FF_API_VIMA_DECODER
    AV_CODEC_ID_VIMA = MKBETAG('V', 'I', 'M', 'A'),
#endif
    AV_CODEC_ID_ADPCM_AFC = MKBETAG('A', 'F', 'C', ' '),
    AV_CODEC_ID_ADPCM_IMA_OKI = MKBETAG('O', 'K', 'I', ' '),
    AV_CODEC_ID_ADPCM_DTK = MKBETAG('D', 'T', 'K', ' '),
    AV_CODEC_ID_ADPCM_IMA_RAD = MKBETAG('R', 'A', 'D', ' '),
    AV_CODEC_ID_ADPCM_G726LE = MKBETAG('6', '2', '7', 'G'),
    AV_CODEC_ID_ADPCM_THP_LE = MKBETAG('T', 'H', 'P', 'L'),

    /* AMR */
    AV_CODEC_ID_AMR_NB = 0x12000,
    AV_CODEC_ID_AMR_WB,

    /* RealAudio codecs*/
    AV_CODEC_ID_RA_144 = 0x13000,
    AV_CODEC_ID_RA_288,

    /* various DPCM codecs */
    AV_CODEC_ID_ROQ_DPCM = 0x14000,
    AV_CODEC_ID_INTERPLAY_DPCM,
    AV_CODEC_ID_XAN_DPCM,
    AV_CODEC_ID_SOL_DPCM,

    /* audio codecs */
    AV_CODEC_ID_MP2 = 0x15000,
    AV_CODEC_ID_MP3, ///< preferred ID for decoding MPEG audio layer 1, 2 or 3
    AV_CODEC_ID_AAC,
    AV_CODEC_ID_AC3,
    AV_CODEC_ID_DTS,
    AV_CODEC_ID_VORBIS,
    AV_CODEC_ID_DVAUDIO,
    AV_CODEC_ID_WMAV1,
    AV_CODEC_ID_WMAV2,
    AV_CODEC_ID_MACE3,
    AV_CODEC_ID_MACE6,
    AV_CODEC_ID_VMDAUDIO,
    AV_CODEC_ID_FLAC,
    AV_CODEC_ID_MP3ADU,
    AV_CODEC_ID_MP3ON4,
    AV_CODEC_ID_SHORTEN,
    AV_CODEC_ID_ALAC,
    AV_CODEC_ID_WESTWOOD_SND1,
    AV_CODEC_ID_GSM, ///< as in Berlin toast format
    AV_CODEC_ID_QDM2,
    AV_CODEC_ID_COOK,
    AV_CODEC_ID_TRUESPEECH,
    AV_CODEC_ID_TTA,
    AV_CODEC_ID_SMACKAUDIO,
    AV_CODEC_ID_QCELP,
    AV_CODEC_ID_WAVPACK,
    AV_CODEC_ID_DSICINAUDIO,
    AV_CODEC_ID_IMC,
    AV_CODEC_ID_MUSEPACK7,
    AV_CODEC_ID_MLP,
    AV_CODEC_ID_GSM_MS, /* as found in WAV */
    AV_CODEC_ID_ATRAC3,
#if FF_API_VOXWARE
    AV_CODEC_ID_VOXWARE,
#endif
    AV_CODEC_ID_APE,
    AV_CODEC_ID_NELLYMOSER,
    AV_CODEC_ID_MUSEPACK8,
    AV_CODEC_ID_SPEEX,
    AV_CODEC_ID_WMAVOICE,
    AV_CODEC_ID_WMAPRO,
    AV_CODEC_ID_WMALOSSLESS,
    AV_CODEC_ID_ATRAC3P,
    AV_CODEC_ID_EAC3,
    AV_CODEC_ID_SIPR,
    AV_CODEC_ID_MP1,
    AV_CODEC_ID_TWINVQ,
    AV_CODEC_ID_TRUEHD,
    AV_CODEC_ID_MP4ALS,
    AV_CODEC_ID_ATRAC1,
    AV_CODEC_ID_BINKAUDIO_RDFT,
    AV_CODEC_ID_BINKAUDIO_DCT,
    AV_CODEC_ID_AAC_LATM,
    AV_CODEC_ID_QDMC,
    AV_CODEC_ID_CELT,
    AV_CODEC_ID_G723_1,
    AV_CODEC_ID_G729,
    AV_CODEC_ID_8SVX_EXP,
    AV_CODEC_ID_8SVX_FIB,
    AV_CODEC_ID_BMV_AUDIO,
    AV_CODEC_ID_RALF,
    AV_CODEC_ID_IAC,
    AV_CODEC_ID_ILBC,
    AV_CODEC_ID_OPUS_DEPRECATED,
    AV_CODEC_ID_COMFORT_NOISE,
    AV_CODEC_ID_TAK_DEPRECATED,
    AV_CODEC_ID_METASOUND,
    AV_CODEC_ID_PAF_AUDIO_DEPRECATED,
    AV_CODEC_ID_ON2AVC,
    AV_CODEC_ID_DSS_SP,
    AV_CODEC_ID_FFWAVESYNTH = MKBETAG('F', 'F', 'W', 'S'),
    AV_CODEC_ID_SONIC = MKBETAG('S', 'O', 'N', 'C'),
    AV_CODEC_ID_SONIC_LS = MKBETAG('S', 'O', 'N', 'L'),
    AV_CODEC_ID_PAF_AUDIO = MKBETAG('P', 'A', 'F', 'A'),
    AV_CODEC_ID_OPUS = MKBETAG('O', 'P', 'U', 'S'),
    AV_CODEC_ID_TAK = MKBETAG('t', 'B', 'a', 'K'),
    AV_CODEC_ID_EVRC = MKBETAG('s', 'e', 'v', 'c'),
    AV_CODEC_ID_SMV = MKBETAG('s', 's', 'm', 'v'),
    AV_CODEC_ID_DSD_LSBF = MKBETAG('D', 'S', 'D', 'L'),
    AV_CODEC_ID_DSD_MSBF = MKBETAG('D', 'S', 'D', 'M'),
    AV_CODEC_ID_DSD_LSBF_PLANAR = MKBETAG('D', 'S', 'D', '1'),
    AV_CODEC_ID_DSD_MSBF_PLANAR = MKBETAG('D', 'S', 'D', '8'),
    AV_CODEC_ID_4GV = MKBETAG('s', '4', 'g', 'v'),

    /* subtitle codecs */
    AV_CODEC_ID_FIRST_SUBTITLE = 0x17000,          ///< A dummy ID pointing at the start of subtitle codecs.
    AV_CODEC_ID_DVD_SUBTITLE = 0x17000,
    AV_CODEC_ID_DVB_SUBTITLE,
    AV_CODEC_ID_TEXT,  ///< raw UTF-8 text
    AV_CODEC_ID_XSUB,
    AV_CODEC_ID_SSA,
    AV_CODEC_ID_MOV_TEXT,
    AV_CODEC_ID_HDMV_PGS_SUBTITLE,
    AV_CODEC_ID_DVB_TELETEXT,
    AV_CODEC_ID_SRT,
    AV_CODEC_ID_MICRODVD = MKBETAG('m', 'D', 'V', 'D'),
    AV_CODEC_ID_EIA_608 = MKBETAG('c', '6', '0', '8'),
    AV_CODEC_ID_JACOSUB = MKBETAG('J', 'S', 'U', 'B'),
    AV_CODEC_ID_SAMI = MKBETAG('S', 'A', 'M', 'I'),
    AV_CODEC_ID_REALTEXT = MKBETAG('R', 'T', 'X', 'T'),
    AV_CODEC_ID_STL = MKBETAG('S', 'p', 'T', 'L'),
    AV_CODEC_ID_SUBVIEWER1 = MKBETAG('S', 'b', 'V', '1'),
    AV_CODEC_ID_SUBVIEWER = MKBETAG('S', 'u', 'b', 'V'),
    AV_CODEC_ID_SUBRIP = MKBETAG('S', 'R', 'i', 'p'),
    AV_CODEC_ID_WEBVTT = MKBETAG('W', 'V', 'T', 'T'),
    AV_CODEC_ID_MPL2 = MKBETAG('M', 'P', 'L', '2'),
    AV_CODEC_ID_VPLAYER = MKBETAG('V', 'P', 'l', 'r'),
    AV_CODEC_ID_PJS = MKBETAG('P', 'h', 'J', 'S'),
    AV_CODEC_ID_ASS = MKBETAG('A', 'S', 'S', ' '),  ///< ASS as defined in Matroska

    /* other specific kind of codecs (generally used for attachments) */
    AV_CODEC_ID_FIRST_UNKNOWN = 0x18000,           ///< A dummy ID pointing at the start of various fake codecs.
    AV_CODEC_ID_TTF = 0x18000,
    AV_CODEC_ID_BINTEXT = MKBETAG('B', 'T', 'X', 'T'),
    AV_CODEC_ID_XBIN = MKBETAG('X', 'B', 'I', 'N'),
    AV_CODEC_ID_IDF = MKBETAG(0, 'I', 'D', 'F'),
    AV_CODEC_ID_OTF = MKBETAG(0, 'O', 'T', 'F'),
    AV_CODEC_ID_SMPTE_KLV = MKBETAG('K', 'L', 'V', 'A'),
    AV_CODEC_ID_DVD_NAV = MKBETAG('D', 'N', 'A', 'V'),
    AV_CODEC_ID_TIMED_ID3 = MKBETAG('T', 'I', 'D', '3'),
    AV_CODEC_ID_BIN_DATA = MKBETAG('D', 'A', 'T', 'A'),


    AV_CODEC_ID_PROBE = 0x19000, ///< codec_id is not known (like AV_CODEC_ID_NONE) but lavf should attempt to identify it

    AV_CODEC_ID_MPEG2TS = 0x20000, /**< _FAKE_ codec to indicate a raw MPEG-2 TS
                                   * stream (only used by libavformat) */
    AV_CODEC_ID_MPEG4SYSTEMS = 0x20001, /**< _FAKE_ codec to indicate a MPEG-4 Systems
                                        * stream (only used by libavformat) */
    AV_CODEC_ID_FFMETADATA = 0x21000,   ///< Dummy codec for streams containing only metadata information.
};

// stream class, similar with track concept in MP4 file.
class AVStream
{
public:
    AVStream()
    {
        m_StreamFileFormat = VP_FILE_RAW;
        m_pMediaInfo = NULL;
        m_pStreamBuffer = NULL;
        m_Type = VP_STREAM_UNKOWN;
        m_colorFormat = VP_UNKNOWN_COLOR;
        m_Codec_Id = AV_CODEC_ID_NONE;
        m_NumOfSamples = m_nDurationTime = m_nTimeScale = 0;
    }

    ~AVStream()
    {

    }

    uint32 GetNumOfSamples()
    {
        return m_NumOfSamples;
    }

    VP_STREAMTYPE GetType()
    {
        return m_Type;
    }

    uint32 GetCodecID()
    {
        return m_Codec_Id;
    }

    uint64 GetDuration()
    {
        return m_nDurationTime;
    }

    uint32 GetTimeScale()
    {
        return m_nTimeScale;
    }

    int                 prev_payload_key;

private:
    pvoid               m_pMediaInfo;
    pvoid               m_pStreamBuffer;
    uint64              m_nByteSize;

protected:
    // sample maybe different with different file type
    // RAW:         Frame
    // H.264 RAW:   NALU
    // MP4:         mp4 Samples
    uint32              m_NumOfSamples;
    uint32              m_Codec_Id;
    VP_FILETYPE         m_StreamFileFormat;
    VP_STREAMTYPE       m_Type;
    uint64              m_nDurationTime;
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
        m_file = NULL;
        m_nNumStreams = m_nNumFrames = 0;
        m_nFileDurationTime = m_nFileTimeScale = 0;
        m_nWidth = m_nHeight = 0;
        m_vFileStreams.clear();
        m_memoryBuffer = NULL;
	}

    ~VideoFileClass()
    {
        if (m_memoryBuffer)
        {
            osFree(m_memoryBuffer);         // just in case
        }
        delete m_file;
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

    bool IsWriteMode()
    {
        if (!m_file)
            return false;

        switch (m_file->mode) {
        case File::MODE_READ:
            return false;

        case File::MODE_MODIFY:
        case File::MODE_CREATE:
        default:
            break;
        }

        return true;
    }

    virtual bool Open(const char* fileName, File::Mode mode, FileProvider* provider);

    // Read video file metadata; construct file structure.
    virtual void ReadFromFile();

    virtual void Dump(bool dumpImplicits);

    virtual void Close();

    virtual uint32 GetSampleByteSize(uint32 sampleIndex);

    virtual bool DumpSampleData(pvoid pBuffer, uint32 bufferSize);

private:
    File*				m_file;
    string              m_FileName;
    uint32              m_fileOriginalSize;
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
