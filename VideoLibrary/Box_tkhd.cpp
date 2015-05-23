

#include "AllMP4Box.h"





MP4TkhdBox::MP4TkhdBox(MP4FileClass &file)
        : MP4Box(file, "tkhd")
{
    AddVersionAndFlags();
}

void MP4TkhdBox::AddProperties(uint8_t version)
{
    if (version == 1) {
        AddProperty( /* 2 */
            new MP4Integer64Property(*this, "creationTime"));
        AddProperty( /* 3 */
            new MP4Integer64Property(*this, "modificationTime"));
    } else { // version == 0
        AddProperty( /* 2 */
            new MP4Integer32Property(*this, "creationTime"));
        AddProperty( /* 3 */
            new MP4Integer32Property(*this, "modificationTime"));
    }

    AddProperty( /* 4 */
        new MP4Integer32Property(*this, "trackId"));
    AddReserved(*this, "reserved1", 4); /* 5 */

    if (version == 1) {
        AddProperty( /* 6 */
            new MP4Integer64Property(*this, "duration"));
    } else {
        AddProperty( /* 6 */
            new MP4Integer32Property(*this, "duration"));
    }

    AddReserved(*this, "reserved2", 8); /* 7 */

    AddProperty( /* 8 */
        new MP4Integer16Property(*this, "layer"));
    AddProperty( /* 9 */
        new MP4Integer16Property(*this, "alternate_group"));

    MP4Float32Property* pProp;

    pProp = new MP4Float32Property(*this, "volume");
    pProp->SetFixed16Format();
    AddProperty(pProp); /* 10 */

    AddReserved(*this, "reserved3", 2); /* 11 */

    AddProperty(new MP4BytesProperty(*this, "matrix", 36)); /* 12 */

    pProp = new MP4Float32Property(*this, "width");
    pProp->SetFixed32Format();
    AddProperty(pProp); /* 13 */

    pProp = new MP4Float32Property(*this, "height");
    pProp->SetFixed32Format();
    AddProperty(pProp); /* 14 */
}

void MP4TkhdBox::Generate()
{
    uint8_t version = m_File.Use64Bits(GetType()) ? 1 : 0;
    SetVersion(version);
    AddProperties(version);

    MP4Box::Generate();

    // set creation and modification times
    MP4Timestamp now = MP4GetAbsTimestamp();
    if (version == 1) {
        ((MP4Integer64Property*)m_pProperties[2])->SetValue(now);
        ((MP4Integer64Property*)m_pProperties[3])->SetValue(now);
    } else {
        ((MP4Integer32Property*)m_pProperties[2])->SetValue(now);
        ((MP4Integer32Property*)m_pProperties[3])->SetValue(now);
    }

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

    ((MP4BytesProperty*)m_pProperties[12])->
    SetValue(matrix, sizeof(matrix));
}

void MP4TkhdBox::Read()
{
    /* read atom version */
    ReadProperties(0, 1);

    /* need to create the properties based on the atom version */
    AddProperties(GetVersion());

    /* now we can read the remaining properties */
    ReadProperties(1);

    Skip(); // to end of atom
}



}
} // namespace mp4v2::impl
