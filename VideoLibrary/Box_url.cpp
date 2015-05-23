

#include "AllMP4Box.h"


MP4UrlBox::MP4UrlBox(MP4FileClass &file, const char *type)
        : MP4Box(file, type)
{
    AddVersionAndFlags();
}

void MP4UrlBox::Read()
{
    // read the version and flags properties
    ReadProperties(0, 2);

    // check if self-contained flag is set
    if (!(GetFlags() & 1)) {
        // if not then read url location
        ReadProperties(2);
    }

    Skip(); // to end of atom
}

void MP4UrlBox::Write()
{
    MP4StringProperty* pLocationProp =
        (MP4StringProperty*)m_pProperties[2];

    // if no url location has been set
    // then set self-contained flag
    // and don't attempt to write anything
    if (pLocationProp->GetValue() == NULL) {
        SetFlags(GetFlags() | 1);
        pLocationProp->SetImplicit(true);
    } else {
        SetFlags(GetFlags() & 0xFFFFFE);
        pLocationProp->SetImplicit(false);
    }

    // write atom as usual
    MP4Box::Write();
}



}
} // namespace mp4v2::impl
