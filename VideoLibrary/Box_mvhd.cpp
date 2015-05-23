

#include "AllMP4Box.h"





MP4MvhdBox::MP4MvhdBox(MP4FileClass &file)
        : MP4Box(file, "mvhd")
{
    AddVersionAndFlags();
}

void MP4MvhdBox::AddProperties(uint8_t version)
{
    if (version == 1) {
        AddProperty( /* 2 */
            new MP4Integer64Property(*this, "creationTime"));
        AddProperty( /* 3 */
            new MP4Integer64Property(*this, "modificationTime"));
    } else {
        AddProperty( /* 2 */
            new MP4Integer32Property(*this, "creationTime"));
        AddProperty( /* 3 */
            new MP4Integer32Property(*this, "modificationTime"));
    }

    AddProperty( /* 4 */
        new MP4Integer32Property(*this, "timeScale"));

    if (version == 1) {
        AddProperty( /* 5 */
            new MP4Integer64Property(*this, "duration"));
    } else {
        AddProperty( /* 5 */
            new MP4Integer32Property(*this, "duration"));
    }

    MP4Float32Property* pProp;

    pProp = new MP4Float32Property(*this, "rate");
    pProp->SetFixed32Format();
    AddProperty(pProp); /* 6 */

    pProp = new MP4Float32Property(*this, "volume");
    pProp->SetFixed16Format();
    AddProperty(pProp); /* 7 */

    AddReserved(*this, "reserved1", 70); /* 8 */

    AddProperty( /* 9 */
        new MP4Integer32Property(*this, "nextTrackId"));
}

void MP4MvhdBox::Generate()
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

    ((MP4Integer32Property*)m_pProperties[4])->SetValue(1000);

    ((MP4Float32Property*)m_pProperties[6])->SetValue(1.0);
    ((MP4Float32Property*)m_pProperties[7])->SetValue(1.0);

    // property reserved has non-zero fixed values
    static uint8_t reserved[70] = {
        0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x40, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
    };
    m_pProperties[8]->SetReadOnly(false);
    ((MP4BytesProperty*)m_pProperties[8])->
    SetValue(reserved, sizeof(reserved));
    m_pProperties[8]->SetReadOnly(true);

    // set next track id
    ((MP4Integer32Property*)m_pProperties[9])->SetValue(1);
}

void MP4MvhdBox::Read()
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
