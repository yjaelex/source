

#include "AllMP4Box.h"





MP4TfhdBox::MP4TfhdBox(MP4FileClass &file)
        : MP4Box(file, "tfhd")
{
    AddVersionAndFlags();   /* 0, 1 */
    AddProperty( /* 2 */
        new MP4Integer32Property(*this, "trackId"));
}

void MP4TfhdBox::AddProperties(uint32_t flags)
{
    if (flags & 0x01) {
        // note this property is signed 64!
        AddProperty(
            new MP4Integer64Property(*this, "baseDataOffset"));
    }
    if (flags & 0x02) {
        AddProperty(
            new MP4Integer32Property(*this, "sampleDescriptionIndex"));
    }
    if (flags & 0x08) {
        AddProperty(
            new MP4Integer32Property(*this, "defaultSampleDuration"));
    }
    if (flags & 0x10) {
        AddProperty(
            new MP4Integer32Property(*this, "defaultSampleSize"));
    }
    if (flags & 0x20) {
        AddProperty(
            new MP4Integer32Property(*this, "defaultSampleFlags"));
    }
}

void MP4TfhdBox::Read()
{
    /* read atom version, flags, and trackId */
    ReadProperties(0, 3);

    /* need to create the properties based on the atom flags */
    AddProperties(GetFlags());

    /* now we can read the remaining properties */
    ReadProperties(3);

    Skip(); // to end of atom
}



}
} // namespace mp4v2::impl
