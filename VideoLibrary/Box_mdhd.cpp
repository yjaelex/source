

#include "AllMP4Box.h"

namespace mp4v2 { namespace impl {



MP4MdhdBox::MP4MdhdBox(MP4FileClass &file)
        : MP4Box(file, "mdhd")
{
    AddVersionAndFlags();
}

void MP4MdhdBox::AddProperties(uint8_t version)
{
    if (version == 1) {
        AddProperty(
            new MP4Integer64Property(*this, "creationTime"));
        AddProperty(
            new MP4Integer64Property(*this, "modificationTime"));
    } else {
        AddProperty(
            new MP4Integer32Property(*this, "creationTime"));
        AddProperty(
            new MP4Integer32Property(*this, "modificationTime"));
    }

    AddProperty(
        new MP4Integer32Property(*this, "timeScale"));

    if (version == 1) {
        AddProperty(
            new MP4Integer64Property(*this, "duration"));
    } else {
        AddProperty(
            new MP4Integer32Property(*this, "duration"));
    }

    AddProperty( new MP4LanguageCodeProperty(*this,  "language" ));
    AddReserved(*this, "reserved", 2);
}

void MP4MdhdBox::Generate()
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
}

void MP4MdhdBox::Read()
{
    /* read atom version */
    ReadProperties(0, 1);

    /* need to create the properties based on the atom version */
    AddProperties(GetVersion());

    /* now we can read the remaining properties */
    ReadProperties(1);

    Skip(); // to end of atom
}



}} // namespace mp4v2::impl
