

#include "AllMP4Box.h"

namespace mp4v2 { namespace impl {



MP4HinfBox::MP4HinfBox(MP4FileClass &file)
        : MP4Box(file, "hinf")
{
    ExpectChildBox("trpy", Optional, OnlyOne);
    ExpectChildBox("nump", Optional, OnlyOne);
    ExpectChildBox("tpyl", Optional, OnlyOne);
    ExpectChildBox("maxr", Optional, Many);
    ExpectChildBox("dmed", Optional, OnlyOne);
    ExpectChildBox("dimm", Optional, OnlyOne);
    ExpectChildBox("drep", Optional, OnlyOne);
    ExpectChildBox("tmin", Optional, OnlyOne);
    ExpectChildBox("tmax", Optional, OnlyOne);
    ExpectChildBox("pmax", Optional, OnlyOne);
    ExpectChildBox("dmax", Optional, OnlyOne);
    ExpectChildBox("payt", Optional, OnlyOne);
}

void MP4HinfBox::Generate()
{
    // hinf is special in that although all it's child atoms
    // are optional (on read), if we generate it for writing
    // we really want all the children

    for (uint32_t i = 0; i < m_pChildBoxInfos.Size(); i++) {
        MP4Box* pChildBox =
            CreateBox(m_File, this, m_pChildBoxInfos[i]->m_name);

        AddChildBox(pChildBox);

        // and ask it to self generate
        pChildBox->Generate();
    }
}



}} // namespace mp4v2::impl
