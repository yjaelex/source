#include "AllMP4Box.h"

MP4TkhdBox::MP4TkhdBox(MP4FileClass &file)
        : MP4FullBox(file, "tkhd")
{
    AddVersionAndFlags();
    m_creationTime = 0;
    m_modificationTime = 0;
    m_trackId = 0;
    m_reserved0 = 0;
    m_duration = 0;

    m_reserved1[1] = m_reserved1[0] = 0;
    m_layer = 0;
    m_alternate_group = 0;

    //{ if track_is_audio 0x0100 else 0 };
    m_volume = 0;
    m_reserved2 = 0;

    // { 0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000 };
    // unity matrix
    memset(m_matrix,0,sizeof(m_matrix));
    m_width = 0;
    m_height = 0;
}

void MP4TkhdBox::Generate()
{
    uint8_t version = m_File.Use64Bits(GetType()) ? 1 : 0;
    SetVersion(version);

    MP4Box::Generate();

    // set creation and modification times
    MP4Timestamp now = MP4GetAbsTimestamp();
    m_creationTime = now;
    m_modificationTime = now;

    // property "matrix" has non-zero fixed values
    // this default identity matrix indicates no transformation, i.e.
    // 1, 0, 0
    // 0, 1, 0
    // 0, 0, 1
    // see http://developer.apple.com/documentation/QuickTime/QTFF/QTFFChap4/chapter_5_section_4.html

    static uint8_t matrix[36] = {
        0x00, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x40, 0x00, 0x00, 0x00,
    };

    memcpy(m_matrix, matrix, sizeof(matrix));
}

void MP4TkhdBox::ReadProperties()
{
    MP4FullBox::ReadProperties();
}

void MP4TkhdBox::Read()
{
    /* read atom version */
    ReadProperties(0, 1);

    /* now we can read the remaining properties */
    ReadProperties(1);

    Skip(); // to end of atom
}

